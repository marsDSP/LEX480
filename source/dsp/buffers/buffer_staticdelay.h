#pragma once

#ifndef LEX480_BUFFER_STATICDELAY_H
#define LEX480_BUFFER_STATICDELAY_H

#include <algorithm>
#include <cmath>

#include "math/interpolator.h"
#include "utils/type_utils.h"

#ifndef jassert
#define jassert(x) assert(x)
#endif

namespace MarsDSP::Buffers
{
    template<typename SampleType, typename InterpolationType = Math::DelayLineInterpolationTypes::None,
        int maxDelaySamples = 1 << 18, typename StorageType = SampleType>
    class StaticDelayBuffer
    {
        using NumericType = Utils::SampleTypeHelpers::NumericType<SampleType>;

    public:
        StaticDelayBuffer() = default;

        void reset() { std::fill(std::begin(buffer), std::end(buffer), StorageType{}); }

        template<typename IT = InterpolationType>
        std::enable_if_t<!std::is_same_v<IT, Math::DelayLineInterpolationTypes::None>, void>
        pushSample(SampleType inputSample, int writeIndex) noexcept
        {
            jassert(writeIndex >= 0 && writeIndex < maxDelaySamples);
            buffer[writeIndex] = static_cast<StorageType>(inputSample);
            buffer[writeIndex + maxDelaySamples] = static_cast<StorageType>(inputSample);
        }

        template<typename IT = InterpolationType>
        std::enable_if_t<std::is_same_v<IT, Math::DelayLineInterpolationTypes::None>, void>
        pushSample(SampleType inputSample, int writeIndex) noexcept
        {
            jassert(writeIndex >= 0 && writeIndex < maxDelaySamples);
            buffer[writeIndex] = static_cast<StorageType>(inputSample);
        }

        template<typename IT = InterpolationType>
        std::enable_if_t<!(std::is_same_v<IT, Math::DelayLineInterpolationTypes::Thiran> ||
                                  std::is_same_v<IT, Math::DelayLineInterpolationTypes::None>), SampleType>
        popSample(NumericType readIndex) noexcept
        {
            jassert(readIndex >= 0 && readIndex < maxDelaySamples);
            return interpolator.call(buffer, static_cast<int>(readIndex),
                                     readIndex - static_cast<NumericType>(static_cast<int>(readIndex)));
        }

        template<typename IT = InterpolationType>
        std::enable_if_t<std::is_same_v<IT, Math::DelayLineInterpolationTypes::None>,
            SampleType> popSample(NumericType readIndex) noexcept
        {
            jassert(readIndex >= 0 && readIndex < maxDelaySamples);
            return interpolator.call(buffer, static_cast<int>(readIndex));
        }

        template<typename IT = InterpolationType>
        std::enable_if_t<std::is_same_v<IT, Math::DelayLineInterpolationTypes::Thiran>, SampleType>
        popSample(NumericType readIndex, SampleType &filterState) noexcept
        {
            jassert(readIndex >= 0 && readIndex < maxDelaySamples);
            return interpolator.call(buffer, static_cast<int>(readIndex),
                                     readIndex - static_cast<NumericType>(static_cast<int>(readIndex)), filterState);
        }

        template<typename T>
        static void decrementPointer(T &index) noexcept
        {
            index += T(maxDelaySamples - 1);
            index = index >= static_cast<T>(maxDelaySamples) ? index - static_cast<T>(maxDelaySamples) : index;
        }

        static NumericType getReadPointer(int writeIndex, NumericType delayInSamples) noexcept
        {
            delayInSamples = std::max(static_cast<NumericType>(1), delayInSamples);
            const auto readIndex = std::fmod(NumericType(writeIndex) + delayInSamples,
                                             static_cast<NumericType>(maxDelaySamples));

            jassert(readIndex >= 0 && readIndex < static_cast<NumericType>(maxDelaySamples));
            return readIndex;
        }

        static int getReadPointer(int writeIndex, int delayInSamples) noexcept
        {
            delayInSamples = std::max(1, delayInSamples);
            const auto readIndex = (writeIndex + delayInSamples) % maxDelaySamples;
            jassert(readIndex >= 0 && readIndex < maxDelaySamples);
            return readIndex;
        }

    private:
        InterpolationType interpolator;

        static constexpr auto bufferSize = static_cast<size_t>(std::is_same_v<InterpolationType,
                                                                   Math::DelayLineInterpolationTypes::None>
                                                                   ? maxDelaySamples
                                                                   : (2 * maxDelaySamples));

        StorageType buffer[bufferSize]{};

#if MARSDSP_USING_JUCE
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticDelayBuffer);
#endif
    };
}
#endif
