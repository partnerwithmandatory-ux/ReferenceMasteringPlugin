#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Analysis.h"
#include "EditHistory.h"

class ReferenceMasterProcessor : public juce::AudioProcessor
{
public:
    ReferenceMasterProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Reference Master"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    // Called only on the message thread, never from processBlock.
    void recordEdit();
    void undoEdit();
    void redoEdit();
    bool canUndo() const { return !resetHistory.load() && edits.canUndo(); }
    bool canRedo() const { return !resetHistory.load() && edits.canRedo(); }
    juce::AudioProcessorValueTreeState state;
    std::atomic<float> inputRms {-120}, inputPeak {-120}, inputSide {0};
private:
    EditHistory<std::vector<float>> edits;
    std::atomic<bool> resetHistory {false};
    void restoreEdit(const std::vector<float>*);
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Limiter<float> limiter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> low, high;
    juce::SmoothedValue<float> width, gain;
    double rate = 44100;
    float lastLow = 1000, lastHigh = 1000;
};
