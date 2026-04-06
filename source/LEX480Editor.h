#pragma once

#include "LEX480Processor.h"

//==============================================================================
class LEX480Editor final : public juce::AudioProcessorEditor
{
public:
    explicit LEX480Editor (LEX480Processor&);
    ~LEX480Editor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LEX480Processor& processorRef;

    struct ParamControl
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    std::array<ParamControl, 10> controls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEX480Editor)
};
