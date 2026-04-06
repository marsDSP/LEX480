#pragma once

#include <JuceHeader.h>
#include "dsp/datt_core.h"
#include "utils/parameters/param_utils.h"

namespace Params
{
    static inline const juce::ParameterID preDelay  { "preDelay",  1 };
    static inline const juce::ParameterID bandwidth { "bandwidth", 1 };
    static inline const juce::ParameterID inputDiff1 { "inputDiff1", 1 };
    static inline const juce::ParameterID inputDiff2 { "inputDiff2", 1 };
    static inline const juce::ParameterID decay { "decay", 1 };
    static inline const juce::ParameterID damping { "damping", 1 };
    static inline const juce::ParameterID decayDiff1 { "decayDiff1", 1 };
    static inline const juce::ParameterID decayDiff2 { "decayDiff2", 1 };
    static inline const juce::ParameterID wet { "wet", 1 };
    static inline const juce::ParameterID dry { "dry", 1 };
}

//==============================================================================
class LEX480Processor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    LEX480Processor();
    ~LEX480Processor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    juce::AudioProcessorValueTreeState apvts;

    // Parameter pointers
    MarsDSP::Params::FloatParameter* preDelayParam = nullptr;
    MarsDSP::Params::FloatParameter* bandwidthParam = nullptr;
    MarsDSP::Params::FloatParameter* inputDiff1Param = nullptr;
    MarsDSP::Params::FloatParameter* inputDiff2Param = nullptr;
    MarsDSP::Params::FloatParameter* decayParam = nullptr;
    MarsDSP::Params::FloatParameter* dampingParam = nullptr;
    MarsDSP::Params::FloatParameter* decayDiff1Param = nullptr;
    MarsDSP::Params::FloatParameter* decayDiff2Param = nullptr;
    MarsDSP::Params::FloatParameter* wetParam = nullptr;
    MarsDSP::Params::FloatParameter* dryParam = nullptr;

    MarsDSP::Reverb::DattoroReverb<float> reverb;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEX480Processor)
};
