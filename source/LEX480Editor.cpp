#include "LEX480Processor.h"
#include "LEX480Editor.h"

//==============================================================================
LEX480Editor::LEX480Editor (LEX480Processor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (400, 300);
}

LEX480Editor::~LEX480Editor()
{
}

//==============================================================================
void LEX480Editor::paint (juce::Graphics& g)
{
    juce::ignoreUnused (g);
}

void LEX480Editor::resized()
{
}
