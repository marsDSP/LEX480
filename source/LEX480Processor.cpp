#include "LEX480Processor.h"
#include "LEX480Editor.h"

//==============================================================================
LEX480Processor::LEX480Processor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    using namespace MarsDSP::Params::ParamUtils;
    loadParameterPointer(preDelayParam, apvts, Params::preDelay);
    loadParameterPointer(bandwidthParam, apvts, Params::bandwidth);
    loadParameterPointer(inputDiff1Param, apvts, Params::inputDiff1);
    loadParameterPointer(inputDiff2Param, apvts, Params::inputDiff2);
    loadParameterPointer(decayParam, apvts, Params::decay);
    loadParameterPointer(dampingParam, apvts, Params::damping);
    loadParameterPointer(decayDiff1Param, apvts, Params::decayDiff1);
    loadParameterPointer(decayDiff2Param, apvts, Params::decayDiff2);
    loadParameterPointer(wetParam, apvts, Params::wet);
    loadParameterPointer(dryParam, apvts, Params::dry);
}

LEX480Processor::~LEX480Processor()
{
}

//==============================================================================
const juce::String LEX480Processor::getName() const
{
    return JucePlugin_Name;
}

bool LEX480Processor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LEX480Processor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LEX480Processor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LEX480Processor::getTailLengthSeconds() const
{
    return 0.0;
}

int LEX480Processor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LEX480Processor::getCurrentProgram()
{
    return 0;
}

void LEX480Processor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String LEX480Processor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void LEX480Processor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void LEX480Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    reverb.prepare(sampleRate);
}

void LEX480Processor::releaseResources()
{
}

void LEX480Processor::reset()
{
    reverb.reset();
}

bool LEX480Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void LEX480Processor::processBlock (juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    reverb.setPreDelayMs(*preDelayParam);
    reverb.setBandwidth(*bandwidthParam);
    reverb.setInputDiffusion1(*inputDiff1Param);
    reverb.setInputDiffusion2(*inputDiff2Param);
    reverb.setDecay(*decayParam);
    reverb.setDamping(*dampingParam);
    reverb.setDecayDiffusion1(*decayDiff1Param);
    reverb.setDecayDiffusion2(*decayDiff2Param);

    reverb.process(buffer, *wetParam, *dryParam);
}

//==============================================================================
bool LEX480Processor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LEX480Processor::createEditor()
{
    return new LEX480Editor (*this);
}

//==============================================================================
void LEX480Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void LEX480Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout LEX480Processor::createParameterLayout()
{
    MarsDSP::Params::Parameters params;

    using namespace MarsDSP::Params::ParamUtils;

    createTimeMsParameter(params, Params::preDelay, "Pre-delay", {0.0f, 100.0f, 0.0f, 1.0f}, 0.0f);
    createPercentParameter(params, Params::bandwidth, "Bandwidth", 0.99f);
    createPercentParameter(params, Params::inputDiff1, "Input Diffusion 1", 0.75f);
    createPercentParameter(params, Params::inputDiff2, "Input Diffusion 2", 0.625f);
    createPercentParameter(params, Params::decay, "Decay", 0.5f);
    createPercentParameter(params, Params::damping, "Damping", 0.5f);
    createPercentParameter(params, Params::decayDiff1, "Decay Diffusion 1", 0.7f);
    createPercentParameter(params, Params::decayDiff2, "Decay Diffusion 2", 0.5f);
    createPercentParameter(params, Params::wet, "Wet", 1.0f);
    createPercentParameter(params, Params::dry, "Dry", 1.0f);

    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LEX480Processor();
}
