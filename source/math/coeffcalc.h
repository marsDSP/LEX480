#pragma once

#ifndef LEX480_COEFFCALC_H
#define LEX480_COEFFCALC_H

#include <xsimd/xsimd.hpp>
#include <JuceHeader.h>

#include "simd/simd_complex.h"

namespace MarsDSP::Math::inline CoeffCalc
{
    template<typename T>
    constexpr auto butterworthQ = static_cast<T>(1) / MathConstants<T>::sqrt2;

    static constexpr int combination(int n, int k)
    {
        if (k < 0 || k > n)
        {
            return 0;
        }
        if (k == 0 || k == n)
        {
            return 1;
        }
        if (k > n / 2)
        {
            k = n - k;
        }
        long res = 1;
        for (int i = 1; i <= k; ++i)
        {
            res = res * (n - i + 1) / i;
        }
        return static_cast<int>(res);
    }

    template<typename T, int M>
    struct Transform
    {
        static constexpr int N = M + 1;

        static void bilinear(T (&b)[static_cast<size_t>(N)], T (&a)[static_cast<size_t>(N)],
                             const T (&bs)[static_cast<size_t>(N)], const T (&as)[static_cast<size_t>(N)], T K)
        {
            for (int j = 0; j < N; ++j)
            {
                auto val_b = T{};
                auto val_a = T{};
                auto k_val = static_cast<T>(1);

                for (int i = 0; i < N; ++i)
                {
                    int n1_pow = 1;
                    for (int k = 0; k < i + 1; ++k)
                    {
                        const auto comb_i_k = combination(i, k);
                        const auto k_pow = k_val * n1_pow;

                        for (int l = 0; l < N - i; ++l)
                        {
                            if (k + l != j)
                                continue;

                            const auto coeff_mult = comb_i_k * combination(M - i, l) * k_pow;
                            val_b += coeff_mult * bs[M - i];
                            val_a += coeff_mult * as[M - i];
                        }
                        n1_pow *= -1;
                    }
                    k_val *= K;
                }
                b[j] = val_b;
                a[j] = val_a;
            }

            for (int j = N - 1; j >= 0; --j)
            {
                b[j] /= a[0];
                a[j] /= a[0];
            }
        }
    };

    template<typename T, typename NumericType>
    void calcFirstOrderShelf(T (&b)[2], T (&a)[2], T lowGain, T highGain, T fc, NumericType fs)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wfloat-equal")
        if (xsimd::all(lowGain == highGain))
        {
            b[0] = lowGain;
            b[1] = static_cast<T>(0);
            a[0] = static_cast<T>(1);
            a[1] = static_cast<T>(0);
            return;
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        MARSDSP_USING_XSIMD_STD(sqrt);
        MARSDSP_USING_XSIMD_STD(tan);

        const auto rho_recip = static_cast<T>(1) / sqrt(highGain / lowGain);
        const auto K = static_cast<T>(1) / tan(MathConstants<NumericType>::pi * fc / fs);

        const T bs[] = {highGain * rho_recip, lowGain};
        const T as[] = {rho_recip, static_cast<T>(1)};
        Transform<T, 1>::bilinear(b, a, bs, as, K);
    }

    template<typename T, typename NumericType>
    T computeKValueAngular(T wc, NumericType fs)
    {
        MARSDSP_USING_XSIMD_STD(tan);
        return wc / tan(wc / (static_cast<NumericType>(2) * fs));
    }

    template<typename T, typename NumericType>
    void calcFirstOrderHPF(T (&b)[2], T (&a)[2], T fc, NumericType fs)
    {
        const auto wc = MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValueAngular(wc, fs);
        const auto wc_recip = static_cast<T>(1) / wc;
        const T bs[] = {wc_recip, static_cast<T>(0)};
        const T as[] = {wc_recip, static_cast<T>(1)};
        Transform<T, 1>::bilinear(b, a, bs, as, K);
    }


    template<typename T, typename NumericType>
    void calcFirstOrderLPF(T (&b)[2], T (&a)[2], T fc, NumericType fs)
    {
        const auto wc = MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValueAngular(wc, fs);
        const T bs[] = {static_cast<T>(0), static_cast<T>(1)};
        const T as[] = {static_cast<T>(1) / wc, static_cast<T>(1)};
        Transform<T, 1>::bilinear(b, a, bs, as, K);
    }
}
#endif
