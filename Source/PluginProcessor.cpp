#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout parameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&](const char* id, const char* label, float min, float max, float initial) {
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1}, label, juce::NormalisableRange<float>{min,max}, initial));
    };
    add("width","Stereo width",0,2,1);
    add("low","Low shelf dB",-12,12,0); add("high","High shelf dB",-12,12,0);
    add("threshold","Threshold dB",-48,0,0); add("ratio","Ratio",1,10,1);
    add("attack","Attack ms",1,100,20); add("release","Release ms",10,500,150);
    add("gain","Drive dB",-12,18,0); add("ceiling","Ceiling dB",-12,0,-1);
    return p;
}
ReferenceMasterProcessor::ReferenceMasterProcessor()
    : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), state(*this,nullptr,"ReferenceMaster",parameters()) {}
bool ReferenceMasterProcessor::isBusesLayoutSupported(const BusesLayout& b) const
{ return b.getMainInputChannelSet()==juce::AudioChannelSet::stereo() && b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo(); }
void ReferenceMasterProcessor::prepareToPlay(double sr,int block)
{
    rate=sr; juce::dsp::ProcessSpec spec{sr,static_cast<juce::uint32>(block),2};
    for (auto* f : {&low,&high}) { f->prepare(spec); f->reset(); }
    compressor.prepare(spec); compressor.reset(); limiter.prepare(spec); limiter.reset();
    width.reset(sr,.03); gain.reset(sr,.03);
    width.setCurrentAndTargetValue(state.getRawParameterValue("width")->load());
    gain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("gain")->load()));
    lastLow=lastHigh=1000;
}
void ReferenceMasterProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto value=[&](const char* id){return state.getRawParameterValue(id)->load();};
    Analysis a;
    for(int i=0;i<b.getNumSamples();++i) a.push(b.getSample(0,i),b.getSample(1,i));
    inputRms.store(a.rmsDb()); inputPeak.store(a.peakDb()); inputSide.store(a.sideRatio());
    // Coefficient objects allocate only on parameter changes, not on every block.
    if(value("low")!=lastLow) { lastLow=value("low"); *low.state=*juce::dsp::IIR::Coefficients<float>::makeLowShelf(rate,120,.707f,juce::Decibels::decibelsToGain(lastLow)); }
    if(value("high")!=lastHigh) { lastHigh=value("high"); *high.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(rate,8000,.707f,juce::Decibels::decibelsToGain(lastHigh)); }
    width.setTargetValue(value("width")); gain.setTargetValue(juce::Decibels::decibelsToGain(value("gain")));
    for(int i=0;i<b.getNumSamples();++i) {
        float l=b.getSample(0,i),r=b.getSample(1,i),m=(l+r)*.5f,s=(l-r)*.5f*width.getNextValue();
        b.setSample(0,i,m+s); b.setSample(1,i,m-s);
    }
    juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block);
    low.process(ctx); high.process(ctx);
    compressor.setThreshold(value("threshold")); compressor.setRatio(value("ratio"));
    compressor.setAttack(value("attack")); compressor.setRelease(value("release")); compressor.process(ctx);
    for(int i=0;i<b.getNumSamples();++i) { auto g=gain.getNextValue(); for(int c=0;c<2;++c) b.setSample(c,i,b.getSample(c,i)*g); }
    limiter.setThreshold(value("ceiling")); limiter.setRelease(100); limiter.process(ctx);
}
void ReferenceMasterProcessor::getStateInformation(juce::MemoryBlock& data) { auto xml=state.copyState().createXml(); copyXmlToBinary(*xml,data); }
void ReferenceMasterProcessor::setStateInformation(const void* data,int size) { if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName("ReferenceMaster")) state.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessorEditor* ReferenceMasterProcessor::createEditor() { return new ReferenceMasterEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ReferenceMasterProcessor(); }
