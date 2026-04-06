#include "buffer_smoothed.h"

namespace MarsDSP::Buffers
{
    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle(std::atomic<float> *handle)
    {
#if ! MARSDSP_SMOOTHED_BUFFER_SMALL
#if JUCE_MODULE_AVAILABLE_MarsDSP_parameters
        modulatableParameterHandle = nullptr;
#endif

        parameterHandle = handle;
        reset(parameterHandle->load());
#endif
    }

#if JUCE_MODULE_AVAILABLE_MarsDSP_parameters
    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle(
        [[maybe_unused]] const FloatParameter *handle)
    {
        parameterHandle = nullptr;

        modulatableParameterHandle = handle;
        reset(modulatableParameterHandle->getCurrentValue());
    }
#endif

    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::prepare(double fs, int samplesPerBlock,
                                                                      bool useInternalVector)
    {
        sampleRate = fs;
        if (useInternalVector)
        {
#if ! MARSDSP_SMOOTHED_BUFFER_SMALL
            buffer.resize((size_t) samplesPerBlock, {});
            bufferData = buffer.data();
#else
            jassertfalse;
#endif
        }
        smoother.reset(sampleRate, rampLengthInSeconds);

#if ! MARSDSP_SMOOTHED_BUFFER_SMALL
        if (parameterHandle != nullptr)
            reset(parameterHandle->load());
#if JUCE_MODULE_AVAILABLE_MarsDSP_parameters
    else if (modulatableParameterHandle != nullptr)
            reset(modulatableParameterHandle->getCurrentValue());
#endif
        else
#endif
            reset();
    }

    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset(FloatType resetValue)
    {
        smoother.setCurrentAndTargetValue(mappingFunction(resetValue));
        isCurrentlySmoothing = false;
    }

    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset()
    {
#if ! MARSDSP_SMOOTHED_BUFFER_SMALL
        if (parameterHandle != nullptr)
        {
            reset((FloatType) parameterHandle->load());
        }
#if JUCE_MODULE_AVAILABLE_MarsDSP_parameters
    else if (modulatableParameterHandle != nullptr)
        {
            reset((FloatType) modulatableParameterHandle->getCurrentValue());
        }
#endif
        else
#endif
        {
            reset(getCurrentValue());
        }
    }

    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setRampLength(double rampLengthSeconds)
    {
        rampLengthInSeconds = rampLengthSeconds;
        reset();
        smoother.reset(sampleRate, rampLengthInSeconds);
        isCurrentlySmoothing = false;
    }

    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process(int numSamples)
    {
#if ! MARSDSP_SMOOTHED_BUFFER_SMALL
        if (parameterHandle != nullptr)
        {
            process((FloatType) parameterHandle->load(), numSamples);
        }
#if JUCE_MODULE_AVAILABLE_MarsDSP_parameters
    else if (modulatableParameterHandle != nullptr)
        {
            process((FloatType) modulatableParameterHandle->getCurrentValue(), numSamples);
        }
#endif
        else
#endif
        {
            // you must set a parameter handle that is not nullptr using setParameterHandle
            // before calling the method!
            jassertfalse;
        }
    }

#if JUCE_MODULE_AVAILABLE_DataStructures
    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process(int numSamples, ArenaAllocatorView alloc)
    {
        bufferData = alloc.allocate<FloatType>(numSamples, bufferAlignment);
        jassert(bufferData != nullptr); // arena allocator is out of memory!
        process(numSamples);
    }
#endif

    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process(FloatType value, int numSamples)
    {
        const auto mappedValue = mappingFunction(value);
        smoother.setTargetValue(mappedValue);

        if (!smoother.isSmoothing())
        {
            isCurrentlySmoothing = false;
            juce::FloatVectorOperations::fill(bufferData, mappedValue, numSamples);
            return;
        }

        isCurrentlySmoothing = true;
        for (int n = 0; n < numSamples; ++n)
            bufferData[n] = smoother.getNextValue();
    }

#if JUCE_MODULE_AVAILABLE_DataStructures
    template<typename FloatType, typename ValueSmoothingTypes>
    void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process(FloatType value, int numSamples,
                                                                      ArenaAllocatorView alloc)
    {
        bufferData = alloc.allocate<FloatType>(numSamples, bufferAlignment);
        jassert(bufferData != nullptr); // arena allocator is out of memory!
        process(value, numSamples);
    }
#endif

#if MARSDSP_ALLOW_TEMPLATE_INSTANTIATIONS
    template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Linear>;
    template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Linear>;
    template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative>;
    template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Multiplicative>;
#endif
}
