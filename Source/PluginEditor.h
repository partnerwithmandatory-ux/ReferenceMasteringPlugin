#pragma once
#include "PluginProcessor.h"
class ReferenceMasterEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ReferenceMasterEditor(ReferenceMasterProcessor&);
    ~ReferenceMasterEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void chooseReference();
    void preset(bool);
    ReferenceMasterProcessor& p;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
    juce::TextButton load{"Load reference"}, apply{"Apply level"}, save{"Save preset"}, restore{"Load preset"};
    juce::TextButton undo{"Undo"}, redo{"Redo"};
    juce::Label title, meters, report;
    std::vector<std::unique_ptr<juce::Component>> pages;
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Label>> labels;
    std::vector<std::unique_ptr<juce::ParameterAttachment>> attachments;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::ThreadPool workers{1};
    juce::CriticalSection lock;
    juce::String result;
    std::atomic<bool> busy{false}, valid{false};
    std::atomic<float> target{-120};
};
