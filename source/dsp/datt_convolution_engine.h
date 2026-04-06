#pragma once

#ifndef LEX480_DATT_CONVOLUTION_ENGINE_H
#define LEX480_DATT_CONVOLUTION_ENGINE_H

#include <JuceHeader.h>

namespace MarsDSP::Reverb
{
    template<typename FFTEngineType = dsp::FFT>
    struct ConvolutionEngine
    {
        /** Creates a new convolution engine for a given IR, note that while future IRs
            may be loaded into this engine, the IR size MUST stay the same.
         */
        ConvolutionEngine(size_t numSamples, size_t maxBlockSize, const float *initialIR = nullptr);

        ~ConvolutionEngine() = default;

        /** Move constructor */
        ConvolutionEngine(ConvolutionEngine &&) noexcept;

        /** Move assignment operator */
        ConvolutionEngine &operator=(ConvolutionEngine &&other) noexcept;

        // resets the state of this convolution
        void reset();

        // sets these samples as the new IR
        void setNewIR(const float *newIR);

        // process samples with zero latency
        void processSamples(const float *input, float *output, size_t numSamples);

        // processes samples with (around) a block size of latency
        void processSamplesWithAddedLatency(const float *input, float *output, size_t numSamples);

        // updates the segments of this convolution
        static void updateSegmentsIfNecessary(size_t numSegmentsToUpdate,
                                              std::vector<juce::AudioBuffer<float> > &segments, size_t fftSize);

        // After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls.
        static void prepareForConvolution(float *samples, size_t fftSize) noexcept;

        // Does the convolution operation itself only on half of the frequency domain samples.
        void convolutionProcessingAndAccumulate(const float *input, const float *impulse, float *output) const;

        // Undoes the re-organization of samples from the function prepareForConvolution.
        // Then takes the conjugate of the frequency domain first half of samples to fill the
        // second half, so that the inverse transform will return real samples in the time domain.
        void updateSymmetricFrequencyDomainData(float *samples) const noexcept;

        //==============================================================================
        const size_t irNumSamples;
        const size_t blockSize;
        const size_t fftSize;
        const std::unique_ptr<FFTEngineType> fftObject;
        const size_t numSegments;
        const size_t numInputSegments;
        size_t currentSegment = 0, inputDataPos = 0;

        juce::AudioBuffer<float> bufferInput, bufferOutput, bufferTempOutput, bufferOverlap;
        std::vector<juce::AudioBuffer<float> > buffersInputSegments, buffersImpulseSegments;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvolutionEngine)
    };
}
#endif
