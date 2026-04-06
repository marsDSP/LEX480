#pragma once

#ifndef LEX480_PARAM_UTILS_H
#define LEX480_PARAM_UTILS_H

#include <memory>
#include <vector>
#include <JuceHeader.h>

namespace MarsDSP::Params
{
    using ParameterID = juce::ParameterID;

    /** Type to use for parameters that can be used to initialise a ValueTreeState */
    using Parameters = std::vector<std::unique_ptr<juce::RangedAudioParameter>>;

    inline namespace ParamUtils
    {
        juce::String freqValToString(float freqVal);
        float stringToFreqVal(const juce::String &s);

        juce::String percentValToString(float percentVal);
        float stringToPercentVal(const juce::String &s);

        juce::String gainValToString(float gainVal);
        float stringToGainVal(const juce::String &s);

        juce::String ratioValToString(float ratioVal);
        float stringToRatioVal(const juce::String &s);

        juce::String timeMsValToString(float timeMsVal);
        float stringToTimeMsVal(const juce::String &s);

        juce::String semitonesValToString(float semitonesVal, bool snapToInt);
        float stringToSemitonesVal(const juce::String &s);

        juce::String floatValToString(float floatVal);
        float stringToFloatVal(const juce::String &s);

        template<int decimalPlaces>
        inline juce::String floatValToStringDecimal(float value)
        {
            return juce::String(value, decimalPlaces, false);
        }
    }
}

#include "param_types.h"

namespace MarsDSP::Params
{
    inline namespace ParamUtils
    {
        using VTSParam = juce::AudioProcessorValueTreeState::Parameter;

        /** Returns a parameter of a given type from the AudioProcessorValueTreeState */
        template<typename ParameterPointerType>
        ParameterPointerType getParameterPointer(const juce::AudioProcessorValueTreeState &vts,
                                                 const ParameterID &parameterID)
        {
            static_assert(std::is_pointer_v<ParameterPointerType>, "Parameter pointer type must be a pointer!");
            static_assert(std::is_base_of_v<juce::RangedAudioParameter, std::remove_pointer_t<ParameterPointerType> >,
                          "Parameter type must be derived from juce::RangedAudioParameter");

#if JUCE_VERSION < 0x070000
            auto *baseParameter = vts.getParameter(parameterID);
#else
            auto *baseParameter = vts.getParameter(parameterID.getParamID());
#endif
            jassert(baseParameter != nullptr); // parameter was not found in the ValueTreeState!

            auto *typedParameter = dynamic_cast<ParameterPointerType>(baseParameter);
            jassert(typedParameter != nullptr); // parameter has the incorrect type!

            return typedParameter;
        }

        /** Loads a parameter of a given type from the AudioProcessorValueTreeState */
        template<typename ParameterPointerType>
        void loadParameterPointer(ParameterPointerType &parameter, const juce::AudioProcessorValueTreeState &vts,
                                  const ParameterID &parameterID)
        {
            parameter = getParameterPointer<ParameterPointerType>(vts, parameterID);
        }

        /**
         * Useful alias for `params.push_back (std::make_unique<ParamType> (args...));`
         */
        template<typename ParamType, typename... Args>
        void emplace_param(Parameters &params, Args &&... args)
        {
            params.push_back(std::make_unique<ParamType>(std::forward<Args>(args)...));
        }

        /** Useful method for creating a juce::NormalisableRange with a centre value */
        template<typename T>
        inline juce::NormalisableRange<T> createNormalisableRange(T start, T end, T centre)
        {
            auto range = juce::NormalisableRange{start, end};
            range.setSkewForCentre(centre);

            return range;
        }

        inline void createFreqParameter(Parameters &params, const ParameterID &id, const juce::String &name, float min,
                                        float max, float centre, float defaultValue)
        {
            auto freqRange = createNormalisableRange(min, max, centre);
            emplace_param<FreqHzParameter>(params, id, name, freqRange, defaultValue);
        }

        inline void createPercentParameter(Parameters &params, const ParameterID &id, const juce::String &name,
                                           float defaultValue)
        {
            emplace_param<PercentParameter>(params, id, name, defaultValue);
        }

        inline void createBipolarPercentParameter(Parameters &params, const ParameterID &id, const juce::String &name,
                                                  float defaultValue)
        {
            emplace_param<PercentParameter>(params, id, name, defaultValue, true);
        }

        inline void createGainDBParameter(Parameters &params, const ParameterID &id, const juce::String &name, float min,
                                          float max, float defaultValue, float centerValue)
        {
            juce::NormalisableRange range{min, max};
            if (centerValue > -1000.0f)
                range.setSkewForCentre(centerValue);

            emplace_param<GainDBParameter>(params, id, name, range, defaultValue);
        }

        inline void createTimeMsParameter(Parameters &params, const ParameterID &id, const juce::String &name,
                                          const juce::NormalisableRange<float> &range, float defaultValue)
        {
            emplace_param<TimeMsParameter>(params, id, name, range, defaultValue);
        }

        inline void createRatioParameter(Parameters &params, const ParameterID &id, const juce::String &name,
                                         const juce::NormalisableRange<float> &range, float defaultValue)
        {
            emplace_param<RatioParameter>(params, id, name, range, defaultValue);
        }

        inline juce::String freqValToString(float freqVal)
        {
            if (freqVal <= 1000.0f)
                return juce::String(freqVal, 2, false) + " Hz";

            return juce::String(freqVal / 1000.0f, 2, false) + " kHz";
        }

        inline float stringToFreqVal(const juce::String &s)
        {
            auto freqVal = s.getFloatValue();

            if (s.getLastCharacter() == 'k' || s.endsWith("kHz") || s.endsWith("khz"))
                freqVal *= 1000.0f;

            return freqVal;
        }

        inline juce::String percentValToString(float percentVal)
        {
            auto percentStr = juce::String(int(percentVal * 100.0f));
            return percentStr + "%";
        }

        inline float stringToPercentVal(const juce::String &s) { return s.getFloatValue() / 100.0f; }

        inline juce::String gainValToString(float gainVal)
        {
            auto gainStr = juce::String(gainVal, 2, false);
            return gainStr + " dB";
        }

        inline float stringToGainVal(const juce::String &s) { return s.getFloatValue(); }

        inline juce::String ratioValToString(float ratioVal)
        {
            auto ratioStr = juce::String(ratioVal, 2, false);
            return ratioStr + " : 1";
        }

        inline float stringToRatioVal(const juce::String &s) { return s.getFloatValue(); }

        inline juce::String timeMsValToString(float timeMsVal)
        {
            if (timeMsVal < 1000.0f)
                return juce::String(timeMsVal, 2, false) + " ms";

            auto timeSecStr = juce::String(timeMsVal / 1000.0f, 2, false);
            return timeSecStr + " s";
        }

        inline float stringToTimeMsVal(const juce::String &s)
        {
            auto timeVal = s.getFloatValue();

            if (s.endsWith(" s") || s.endsWith(" S")
                || s.endsWith(" seconds") || s.endsWith(" Seconds"))
                timeVal *= 1000.0f;

            return timeVal;
        }

        inline juce::String semitonesValToString(float semitonesVal, bool snapToInt)
        {
            auto semitonesStr = snapToInt
                                    ? juce::String(static_cast<int>(semitonesVal)) + " st"
                                    : juce::String(semitonesVal, 2, false) + " st";
            if (semitonesVal > 0.0f)
                semitonesStr = "+" + semitonesStr;
            return semitonesStr;
        }

        inline float stringToSemitonesVal(const juce::String &s) { return s.getFloatValue(); }

        inline juce::String floatValToString(float floatVal)
        {
            return floatValToStringDecimal<2>(floatVal);
        }

        inline float stringToFloatVal(const juce::String &s) { return s.getFloatValue(); }
    }
}
#endif
