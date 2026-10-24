#include "PluginEditor.h"
ReferenceMasterEditor::ReferenceMasterEditor(ReferenceMasterProcessor& processor):AudioProcessorEditor(processor),p(processor)
{
    setSize(920,620);
    title.setText("REFERENCE MASTER / Prototype",juce::dontSendNotification);
    title.setFont(juce::Font(24.f));
    juce::Component* controls[]{&title,&meters,&report,&load,&apply,&save,&restore,&tabs};
    for(auto* c:controls) addAndMakeVisible(c);
    report.setText("Load a WAV, AIFF or FLAC reference. No AI source separation is included yet.",juce::dontSendNotification);
    juce::StringArray names{"Width & Depth","EQ","Compression","Maximizer"};
    std::vector<std::vector<juce::String>> ids{{"width"},{"low","high"},{"threshold","ratio","attack","release"},{"gain","ceiling"}};
    for(size_t page=0;page<ids.size();++page) {
        auto panel=std::make_unique<juce::Component>(); int row=0;
        for(auto& id:ids[page]) {
            auto s=std::make_unique<juce::Slider>(); auto l=std::make_unique<juce::Label>();
            l->setText(p.state.getParameter(id)->getName(64),juce::dontSendNotification);
            l->setBounds(20,20+row*66,180,26); s->setBounds(200,20+row*66,630,40);
            s->setSliderStyle(juce::Slider::LinearHorizontal); s->setTextBoxStyle(juce::Slider::TextBoxRight,false,90,24);
            panel->addAndMakeVisible(*s); panel->addAndMakeVisible(*l);
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,*s));
            sliders.push_back(std::move(s)); labels.push_back(std::move(l)); ++row;
        }
        tabs.addTab(names[int(page)],juce::Colour(0xff192731),panel.get(),false); pages.push_back(std::move(panel));
    }
    load.onClick=[this]{chooseReference();}; save.onClick=[this]{preset(true);}; restore.onClick=[this]{preset(false);};
    apply.onClick=[this]{if(valid.load() && p.inputRms.load()>-70) {auto* param=p.state.getParameter("gain");param->beginChangeGesture();param->setValueNotifyingHost(param->convertTo0to1(juce::jlimit(-12.f,12.f,target.load()-p.inputRms.load())));param->endChangeGesture();}};
    startTimerHz(10);
}
ReferenceMasterEditor::~ReferenceMasterEditor(){stopTimer();workers.removeAllJobs(true,-1);}
void ReferenceMasterEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour(0xff0d171f));}
void ReferenceMasterEditor::resized(){title.setBounds(24,15,870,40);meters.setBounds(24,60,870,30);load.setBounds(24,105,150,34);apply.setBounds(190,105,150,34);save.setBounds(590,105,140,34);restore.setBounds(745,105,150,34);report.setBounds(24,150,870,100);tabs.setBounds(24,260,getWidth()-48,getHeight()-280);}
void ReferenceMasterEditor::timerCallback(){meters.setText("Input RMS "+juce::String(p.inputRms.load(),1)+" dBFS | Sample peak "+juce::String(p.inputPeak.load(),1)+" dBFS | Side energy "+juce::String(p.inputSide.load()*100,1)+"%",juce::dontSendNotification);{const juce::ScopedLock guard(lock);if(result.isNotEmpty())report.setText(result,juce::dontSendNotification);}load.setEnabled(!busy.load());apply.setEnabled(valid.load() && p.inputRms.load()>-70);}
void ReferenceMasterEditor::chooseReference()
{
    chooser=std::make_unique<juce::FileChooser>("Reference audio",juce::File{},"*.wav;*.aiff;*.aif;*.flac");
    chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& fc){
        auto file=fc.getResult();if(!file.existsAsFile())return;busy.store(true);valid.store(false);
        {const juce::ScopedLock guard(lock);result="Analyzing "+file.getFileName();}
        workers.addJob([this,file]{
            juce::AudioFormatManager formats;formats.registerBasicFormats();std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));Analysis a;bool ok=reader!=nullptr;
            if(reader){juce::AudioBuffer<float> b(2,32768);for(juce::int64 pos=0;pos<reader->lengthInSamples;pos+=b.getNumSamples()){int n=int(std::min<juce::int64>(b.getNumSamples(),reader->lengthInSamples-pos));if(!reader->read(&b,0,n,pos,true,true)){ok=false;break;}for(int i=0;i<n;++i)a.push(b.getSample(0,i),reader->numChannels==1?b.getSample(0,i):b.getSample(1,i));}}
            ok=ok && a.samples>0 && a.rmsDb()>-100;
            {const juce::ScopedLock guard(lock);result=ok?file.getFileName()+"\nRMS "+juce::String(a.rmsDb(),1)+" dBFS | Peak "+juce::String(a.peakDb(),1)+" dBFS | Crest "+juce::String(a.peakDb()-a.rmsDb(),1)+" dB\nSide energy "+juce::String(a.sideRatio()*100,1)+"% | Correlation "+juce::String(a.correlation(),2)+". Apply suggests drive only.":"Cannot analyze unsupported, unreadable, empty or silent reference.";}
            target.store(a.rmsDb());valid.store(ok);busy.store(false);
        });
    });
}
void ReferenceMasterEditor::preset(bool saving)
{
    chooser=std::make_unique<juce::FileChooser>(saving?"Save preset":"Load preset",juce::File{},"*.rmpreset");
    chooser->launchAsync((saving?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting:juce::FileBrowserComponent::openMode)|juce::FileBrowserComponent::canSelectFiles,[this,saving](const juce::FileChooser& fc){auto f=fc.getResult();if(f==juce::File{})return;bool ok=false;if(saving){auto xml=p.state.copyState().createXml();ok=xml->writeTo(f.withFileExtension("rmpreset"));}else if(auto xml=juce::XmlDocument::parse(f))if(xml->hasTagName("ReferenceMaster")){p.state.replaceState(juce::ValueTree::fromXml(*xml));ok=true;}const juce::ScopedLock guard(lock);result=ok?"Preset operation completed.":"Preset operation failed.";});
}
