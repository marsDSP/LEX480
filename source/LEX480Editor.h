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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEX480Editor)
};
