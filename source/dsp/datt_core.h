#pragma once

#ifndef LEX480_DATT_CORE_H
#define LEX480_DATT_CORE_H

#include <JuceHeader.h>
#include "datt_input_network.h"
#include "datt_tank_network.h"
#include "datt_filters.h"
#include "buffers/buffer_staticdelay.h"

namespace MarsDSP::Reverb::inline Core::inline Dattoro
{
    /** Reverb engine implementing the Dattorro architecture */
    template <typename FloatType = float>
    class DattoroReverb
    {
    public:
        using DelayType = Buffers::StaticDelayBuffer<FloatType, Math::DelayLineInterpolationTypes::None, 48000>;

        DattoroReverb() = default;

        /** Prepares the reverb for a given sample rate */
        void prepare(double sampleRate)
        {
            fs = sampleRate;
            preDelay.reset();
            
            inputNetwork.prepare(static_cast<FloatType>(sampleRate));
            tankNetwork.prepare(static_cast<FloatType>(sampleRate));
            
            bandwidthFilter.reset();
        }

        /** Resets the reverb state */
        void reset()
        {
            preDelay.reset();
            inputNetwork.reset();
            tankNetwork.reset();
            bandwidthFilter.reset();
            wp = 0;
        }

        /** Sets the pre-delay time in milliseconds */
        void setPreDelayMs(FloatType ms)
        {
            preDelaySamples = juce::roundToInt(ms * static_cast<FloatType>(0.001) * static_cast<FloatType>(fs));
        }

        /** Sets the input bandwidth (cutoff frequency) as a normalized value [0, 1] */
        void setBandwidth(FloatType bandwidth)
        {
            auto freq = juce::jmap(bandwidth, FloatType(20.0), FloatType(20000.0));
            bandwidthFilter.calcCoefs(freq, static_cast<FloatType>(fs));
        }

        /** Sets the input diffusion 1 amount [0, 1] */
        void setInputDiffusion1(FloatType diff)
        {
            inputNetwork.getStage(DefaultInputNetworkConfig<FloatType>::InputDiffusionStage::Stage1_Part1).g = diff;
            inputNetwork.getStage(DefaultInputNetworkConfig<FloatType>::InputDiffusionStage::Stage1_Part2).g = diff;
        }

        /** Sets the input diffusion 2 amount [0, 1] */
        void setInputDiffusion2(FloatType diff)
        {
            inputNetwork.getStage(DefaultInputNetworkConfig<FloatType>::InputDiffusionStage::Stage2_Part1).g = diff;
            inputNetwork.getStage(DefaultInputNetworkConfig<FloatType>::InputDiffusionStage::Stage2_Part2).g = diff;
        }

        /** Sets the decay amount [0, 1] */
        void setDecay(FloatType decay)
        {
            tankNetwork.setDecayAmount(decay);
        }

        /** Sets the feedback damping (cutoff frequency) as a normalized value [0, 1] */
        void setDamping(FloatType damping)
        {
            auto freq = juce::jmap(damping, FloatType(20.0), FloatType(20000.0));
            tankNetwork.setDampingFrequency(freq);
        }

        /** Sets the decay diffusion 1 amount [0, 1] */
        void setDecayDiffusion1(FloatType diff)
        {
            tankNetwork.getDecayDiffusion1Stage(0).g = diff;
            tankNetwork.getDecayDiffusion1Stage(1).g = diff;
        }

        /** Sets the decay diffusion 2 amount [0, 1] */
        void setDecayDiffusion2(FloatType diff)
        {
            tankNetwork.getDecayDiffusion2Stage(0).g = diff;
            tankNetwork.getDecayDiffusion2Stage(1).g = diff;
        }

        /** Processes a block of audio */
        void process(juce::AudioBuffer<FloatType>& buffer, FloatType wet, FloatType dry)
        {
            auto* left = buffer.getWritePointer(0);
            auto* right = buffer.getWritePointer(1);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                const FloatType dryL = left[i];
                const FloatType dryR = right[i];
                FloatType monoIn = (dryL + dryR) * FloatType(0.5);
                
                // Pre-delay
                preDelay.pushSample(monoIn, wp);
                auto rp = DelayType::getReadPointer(wp, preDelaySamples);
                monoIn = preDelay.popSample(rp);
                DelayType::decrementPointer(wp);

                // Bandwidth
                monoIn = bandwidthFilter.processSample(monoIn);

                // Input Diffusion
                monoIn = inputNetwork.processSample(monoIn);

                // Tank
                auto [outL, outR] = tankNetwork.processSample(monoIn, monoIn);

                left[i] = dryL * dry + outL * wet;
                right[i] = dryR * dry + outR * wet;
            }
        }

    private:
        double fs = 48000.0;
        int wp = 0;
        int preDelaySamples = 0;
        DelayType preDelay;
        InputNetwork<DefaultInputNetworkConfig<FloatType>> inputNetwork;
        TankNetwork<DefaultTankNetworkConfig<FloatType>> tankNetwork;
        FirstOrderLPF<FloatType> bandwidthFilter;
    };
}

#endif