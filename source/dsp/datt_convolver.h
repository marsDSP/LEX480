#pragma once

#ifndef LEX480_DATT_CONVOLVER_H
#define LEX480_DATT_CONVOLVER_H

#include <JuceHeader.h>

#include "datt_convolution_engine.h"
#include "buffers/buffer_math.h"
#include "buffers/buffer_smoothed.h"

namespace MarsDSP::Reverb
{
    class Convolver
    {
    public:
        explicit Convolver(double maxDiffusionTimeS = 0.1) : maxDiffusionSeconds(maxDiffusionTimeS)
        {
        }

        void prepare(const dsp::ProcessSpec &spec)
        {
            fs = static_cast<float>(spec.sampleRate);

            const auto kernelSize = nextPowerOfTwo(roundToInt(spec.sampleRate * maxDiffusionSeconds));
            generateConvolutionKernel(spec.sampleRate, static_cast<int>(spec.numChannels), kernelSize);

            convolutionEngines.clear();
            convolutionEngines.reserve(spec.numChannels);
            for (size_t ch = 0; ch < spec.numChannels; ++ch)
                convolutionEngines.emplace_back(static_cast<size_t>(kernelSize), spec.maximumBlockSize);

            scratchBuffer.setMaxSize(static_cast<int>(spec.numChannels), kernelSize);
            setDiffusionTime(diffusionTimeSeconds);
        }

        /** Resets the state of the processor */
        void reset()
        {
            for (auto &eng: convolutionEngines)
                eng.reset();
        }

        /** Sets the diffusion time in seconds */
        void setDiffusionTime(float newDiffusionTimeS)
        {
            jassert(newDiffusionTimeS <= (float) maxDiffusionSeconds);
            diffusionTimeSeconds = jmin(newDiffusionTimeS, static_cast<float>(maxDiffusionSeconds));

            const auto diffusionTimeSamples = roundToInt(fs * diffusionTimeSeconds);
            scratchBuffer.clear();
            Math::BufferMath::copyBufferData(convolutionKernelBuffer,
                                             scratchBuffer,
                                             0,
                                             0,
                                             diffusionTimeSamples);

            const auto makeupGain = 32.0f / std::sqrt(static_cast<float>(diffusionTimeSamples));
            Math::BufferMath::applyGain(scratchBuffer, makeupGain);

            for (int ch = 0; ch < convolutionKernelBuffer.getNumChannels(); ++ch)
                convolutionEngines[static_cast<size_t>(ch)].setNewIR(scratchBuffer.getReadPointer(ch));
        }

        /** Processes a buffer */
        void processBlock(const Buffers::BufferView<float> &buffer) noexcept
        {
            const auto numChannels = buffer.getNumChannels();
            const auto numSamples = buffer.getNumSamples();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                convolutionEngines[static_cast<size_t>(ch)].processSamples(buffer.getReadPointer(ch),
                                                                           buffer.getWritePointer(ch),
                                                                           static_cast<size_t>(numSamples));
            }
        }

    private:
        void generateConvolutionKernel(double sampleRate, int numChannels, int kernelSize)
        {
            convolutionKernelBuffer.setMaxSize(numChannels, kernelSize);
            for (auto [_, data]: channels(convolutionKernelBuffer))
            {
                Random rand;
                for (auto &sample: data)
                    sample = rand.nextFloat() * 2.0f - 1.0f;
            }

            Buffers::SmoothedBufferValue<float> kernelRamp;
            kernelRamp.prepare(sampleRate, kernelSize);
            kernelRamp.setRampLength(0.05); // Should the ramp length be a parameter in some way?
            kernelRamp.reset(0.0f);
            kernelRamp.process(1.0f, kernelSize);
            Math::BufferMath::applyGainSmoothedBuffer(convolutionKernelBuffer, kernelRamp);
        }

        std::vector<ConvolutionEngine<> > convolutionEngines{};

        Buffers::Buffer<float> convolutionKernelBuffer;
        Buffers::Buffer<float> scratchBuffer;

        const double maxDiffusionSeconds;
        float fs = 48000.0f;
        float diffusionTimeSeconds = 0.1f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Convolver)
    };
}
#endif
