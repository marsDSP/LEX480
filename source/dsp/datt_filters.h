#pragma once

#ifndef LEX480_DATT_FILTERS_H
#define LEX480_DATT_FILTERS_H

#include "buffers/buffer_core.h"
#include "math/coeffcalc.h"

namespace MarsDSP::Reverb::inline Filters
{
    template <int Order, typename T, size_t maxChannelCount = Utils::Buffers::defaultChannelCount>
    class IIRFilter
    {
    public:
        IIRFilter() { reset(); }
        virtual ~IIRFilter() = default;

        void reset()
        {
            for (auto& s : state)
                std::fill(s.begin(), s.end(), T{});
        }

        T processSample (T x, size_t channel = 0) noexcept
        {
            jassert (channel < maxChannelCount);
            auto& s = state[channel];

            T out = b[0] * x + s[0];

            if constexpr (Order > 0)
            {
                for (int i = 0; i < Order - 1; ++i)
                    s[static_cast<size_t>(i)] = b[i + 1] * x - a[i + 1] * out + s[static_cast<size_t>(i + 1)];

                s[Order - 1] = b[Order] * x - a[Order] * out;
            }

            return out;
        }

    protected:
        T b[Order + 1] {};
        T a[Order + 1] {};
        std::array<std::array<T, (Order > 0 ? Order : 1)>, maxChannelCount> state;
    };

    /**
 * First-order lowpass filter (-6 dB / octave).
 * This filter can be used to model an ideal RC lowpwass filter.
 */
template <typename T, size_t maxChannelCount = Utils::Buffers::defaultChannelCount>
class FirstOrderLPF final : public IIRFilter<1, T, maxChannelCount>
{
public:
    using NumericType = T;

    FirstOrderLPF() = default;
    FirstOrderLPF (FirstOrderLPF&&) noexcept = default;
    FirstOrderLPF& operator= (FirstOrderLPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s + 1} \f$
     */
    void calcCoefs (T fc, NumericType fs)
    {
        Math::CoeffCalc::calcFirstOrderLPF (this->b, this->a, fc, fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderLPF)
};

/**
 * First-order highpass filter (-6 dB / octave).
 * This filter can be used to model an ideal RC highpass filter.
 */
template <typename T, size_t maxChannelCount = Utils::Buffers::defaultChannelCount>
class FirstOrderHPF final : public IIRFilter<1, T, maxChannelCount>
{
public:
    using NumericType = T;

    FirstOrderHPF() = default;
    FirstOrderHPF (FirstOrderHPF&&) noexcept = default;
    FirstOrderHPF& operator= (FirstOrderHPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s}{s + 1} \f$
     */
    void calcCoefs (T fc, NumericType fs)
    {
        Math::CoeffCalc::calcFirstOrderHPF (this->b, this->a, fc, fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderHPF)
};

/**
 * A first order shelving filter, with a set gain at DC,
 * a set gain at high frequencies, and a transition frequency.
 */
template <typename T = float, size_t maxChannelCount = Utils::Buffers::defaultChannelCount>
class ShelfFilter final : public IIRFilter<1, T, maxChannelCount>
{
public:
    using NumericType = T;

    ShelfFilter() = default;
    ShelfFilter (ShelfFilter&&) noexcept = default;
    ShelfFilter& operator= (ShelfFilter&&) noexcept = default;

    /** Calculates the coefficients for the filter.
     * @param lowGain: the gain of the filter at low frequencies
     * @param highGain: the gain of the filter at high frequencies
     * @param fc: the transition frequency of the filter
     * @param fs: the sample rate for the filter
     *
     * For information on the filter coefficient derivation,
     * see Abel and Berners dsp4dae, pg. 249
     */
    void calcCoefs (T lowGain, T highGain, T fc, NumericType fs)
    {
        Math::CoeffCalc::calcFirstOrderShelf (this->b, this->a, lowGain, highGain, fc, fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShelfFilter)
};
}
#endif