#pragma once

#ifndef LEX480_DATT_LATTICE_H
#define LEX480_DATT_LATTICE_H
#include "buffers/buffer_staticdelay.h"

namespace MarsDSP::Reverb
{
    /** Lattice filter used in Dattorro reverb architecture */
    template <typename FloatType, int maxSize, bool invert = false>
    struct Lattice
    {
        using DelayType = Buffers::StaticDelayBuffer<FloatType, Math::DelayLineInterpolationTypes::None, maxSize>;

        void reset()
        {
            delay.reset();
            setDelayLength (delayLengthSamples, true);
        }

        void setDelayLength (int lengthSamples, bool forceUpdate = false)
        {
            if (lengthSamples == delayLengthSamples && ! forceUpdate)
                return;

            writePointer = 0;
            delayLengthSamples = lengthSamples;
            readPointer = DelayType::getReadPointer (writePointer, delayLengthSamples);
        }

        inline FloatType process (FloatType x) noexcept
        {
            // read from delay line
            const auto delayOut = delay.popSample (static_cast<FloatType>(readPointer));
            DelayType::decrementPointer (readPointer);

            // write to delay line
            if constexpr (invert)
                delay.pushSample (x + g * delayOut, writePointer);
            else
                delay.pushSample (x - g * delayOut, writePointer);
            DelayType::decrementPointer (writePointer);

            if constexpr (invert)
                return delayOut - g * x;
            else
                return delayOut + g * x;
        }

        inline FloatType getTap (int tapPointSamples) noexcept
        {
            jassert (tapPointSamples <= delayLengthSamples);
            const auto rp = DelayType::getReadPointer (writePointer, tapPointSamples);
            return delay.popSample (static_cast<FloatType>(rp));
        }

        DelayType delay;
        int writePointer = 0;
        int readPointer = 0;
        int delayLengthSamples = 1;
        FloatType g;
    };
}
#endif