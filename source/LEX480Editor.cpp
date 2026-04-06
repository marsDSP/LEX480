#include "LEX480Processor.h"
#include "LEX480Editor.h"

//==============================================================================
LEX480Editor::LEX480Editor (LEX480Processor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (600, 400);

    auto& apvts = p.getAPVTS();

    struct ParamInfo { juce::ParameterID id; juce::String name; };
    std::array<ParamInfo, 10> params {{
        { Params::preDelay, "Pre-delay" },
        { Params::bandwidth, "Bandwidth" },
        { Params::inputDiff1, "Input Diff 1" },
        { Params::inputDiff2, "Input Diff 2" },
        { Params::decay, "Decay" },
        { Params::damping, "Damping" },
        { Params::decayDiff1, "Decay Diff 1" },
        { Params::decayDiff2, "Decay Diff 2" },
        { Params::wet, "Wet" },
        { Params::dry, "Dry" }
    }};

    for (int i = 0; i < 10; ++i)
    {
        auto& ctrl = controls[i];
        
        ctrl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        ctrl.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(ctrl.slider);

        ctrl.label.setText(params[i].name, juce::dontSendNotification);
        ctrl.label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(ctrl.label);

        ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, params[i].id.getParamID(), ctrl.slider);
    }
}

LEX480Editor::~LEX480Editor()
{
}

//==============================================================================
void LEX480Editor::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void LEX480Editor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    int cols = 5;
    int rows = 2;
    int width = area.getWidth() / cols;
    int height = area.getHeight() / rows;

    for (int i = 0; i < 10; ++i)
    {
        int col = i % cols;
        int row = i / cols;

        auto r = area.removeFromTop(height);
        if (col == cols - 1 && i < 10) {
            // we will use another way for layouting to be simpler
        }
    }

    // Better layout
    area = getLocalBounds().reduced(20);
    for (int i = 0; i < 10; ++i)
    {
        int col = i % cols;
        int row = i / cols;
        
        auto cell = juce::Rectangle<int>(area.getX() + col * width, area.getY() + row * height, width, height).reduced(5);
        
        controls[i].label.setBounds(cell.removeFromTop(20));
        controls[i].slider.setBounds(cell);
    }
}
