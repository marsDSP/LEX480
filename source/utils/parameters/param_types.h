#pragma once

#ifndef LEX480_PARAM_TYPES_H
#define LEX480_PARAM_TYPES_H

#include <JuceHeader.h>
#include "../ptr_optional.h"
#include "param_utils.h"

namespace MarsDSP::Params
{
    struct ModParameterMixin
    {
        virtual ~ModParameterMixin() = default;

        /** Returns true if this parameter supports monophonic modulation. */
        virtual bool supportsMonophonicModulation() { return false; }

        /** Returns true if this parameter supports polyphonic modulation. */
        virtual bool supportsPolyphonicModulation() { return false; }

        /** Base function for applying monophonic modulation to a parameter. */
        [[maybe_unused]] virtual void applyMonophonicModulation (double /*value*/)
        {
        }

        /** Base function for applying polyphonic modulation to a parameter. */
        [[maybe_unused]] virtual void applyPolyphonicModulation (int32_t /*note_id*/, int16_t /*port_index*/, int16_t /*channel*/, int16_t /*key*/, double /*value*/)
        {
        }
    };

    class FloatParameter : public AudioParameterFloat,
                           public ModParameterMixin
    {
    public:
        FloatParameter(const ParameterID &parameterID,
                       const String &parameterName,
                       const NormalisableRange<float> &valueRange,
                       float defaultValue,
                       const std::function<String(float)> &valueToTextFunction,
                       std::function<float(const String &)> &&textToValueFunction);

        using Ptr = Utils::OptionalPointer<FloatParameter>;

        /**
         * Sets the parameter value.
         * This will result in a call @c setValueNotifyingHost, so make sure that's what you want.
         * Especially if calling this from the audio thread!
         */
        void setParameterValue(float newValue) { AudioParameterFloat::operator=(newValue); }

        /** Returns the default value for the parameter. */
        float getDefaultValue() const override { return unsnappedDefault; }

        /** TRUE! */
        bool supportsMonophonicModulation() override { return true; }

        /** Applies monphonic modulation to this parameter. */
        void applyMonophonicModulation(double value) override;

        /** Returns the current parameter value accounting for any modulation that is currently applied. */
        float getCurrentValue() const noexcept;

        /** Returns the current parameter value accounting for any modulation that is currently applied. */
        operator float() const noexcept { return getCurrentValue(); } // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here

    private:
        const float unsnappedDefault;
        const NormalisableRange<float> normalisableRange;

        float modulationAmount = 0.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatParameter)
    };

    /** Wrapper of juce::AudioParameterChoice that does not support modulation. */
    class ChoiceParameter : public AudioParameterChoice,
                            public ModParameterMixin
    {
    public:
        ChoiceParameter(const ParameterID &parameterID, const String &parameterName,
                        const StringArray &parameterChoices, int defaultItemIndex)
            : AudioParameterChoice(parameterID, parameterName, parameterChoices, defaultItemIndex),
              defaultChoiceIndex(defaultItemIndex)
        {
        }

        using Ptr = Utils::OptionalPointer<ChoiceParameter>;

        /** Returns the default value for the parameter. */
        int getDefaultIndex() const noexcept { return defaultChoiceIndex; }

        /**
         * Sets the parameter value.
         * This will result in a call @c setValueNotifyingHost, so make sure that's what you want.
         * Especially if calling this from the audio thread!
         */
        void setParameterValue(int newValue) { AudioParameterChoice::operator=(newValue); }

    private:
        const int defaultChoiceIndex = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChoiceParameter)
    };

    /*/**
     * A Choice parameter based off of an enum class type.
     *
     * By default, underscores in enum names will be replaced with spaces.
     * For custom behaviour, replace the charMap argument with a custom
     * character map.
     #1#
    template<typename EnumType>
    class EnumChoiceParameter : public ChoiceParameter
    {
    public:
        // Ideally we could use any fully specified enum, but since there's no way to enforce
        // that, let's stick to "flags" enums.
        static_assert(magic_enum::detail::is_flags_v<EnumType>,
                      "In order to enforce consistent serialization/deserialization, enum types should be constructed as flags.")
        ;

        EnumChoiceParameter(const ParameterID &parameterID,
                            const String &parameterName,
                            EnumType defaultChoice,
                            const std::initializer_list<std::pair<char, char> > &charMap = {{'_', ' '}})
            : ChoiceParameter(
                parameterID,
                parameterName,
                EnumHelpers::createStringArray<EnumType>(charMap),
                static_cast<int>(*magic_enum::enum_index(defaultChoice)))
        {
        }

        EnumType get() const noexcept
        {
            return magic_enum::enum_value<EnumType>((size_t) getIndex());
        }

        /**
         * Sets the parameter value.
         * This will result in a call @c setValueNotifyingHost, so make sure that's what you want.
         * Especially if calling this from the audio thread!
         #1#
        void setParameterValue(EnumType newValue)
        {
            AudioParameterChoice::operator=(static_cast<int>(*magic_enum::enum_index(newValue)));
        }

        using Ptr = OptionalPointer<EnumChoiceParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnumChoiceParameter)
    };*/

    /** Wrapper of juce::AudioParameterBool that does not support modulation. */
    class BoolParameter : public AudioParameterBool,
                          public ModParameterMixin
    {
    public:
        BoolParameter(const ParameterID &parameterID, const String &parameterName, bool defaultBoolValue)
            : AudioParameterBool(parameterID, parameterName, defaultBoolValue)
        {
        }

        using Ptr = Utils::OptionalPointer<BoolParameter>;

        /**
         * Sets the parameter value.
         * This will result in a call @c setValueNotifyingHost, so make sure that's what you want.
         * Especially if calling this from the audio thread!
         */
        void setParameterValue(bool newValue) { AudioParameterBool::operator=(newValue); }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolParameter)
    };

    /** A float parameter which specifically stores a percentage value. */
    class PercentParameter : public FloatParameter
    {
    public:
        PercentParameter(const ParameterID &parameterID,
                         const String &paramName,
                         float defaultValue = 0.5f,
                         bool isBipolar = false)
            : FloatParameter(parameterID,
                             paramName,
                             NormalisableRange{isBipolar ? -1.0f : 0.0f, 1.0f},
                             defaultValue,
                             &percentValToString,
                             &stringToPercentVal)
        {
        }

        using Ptr = Utils::OptionalPointer<PercentParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PercentParameter)
    };

    /** A float parameter which specifically stores a gain value in Decibels. */
    class GainDBParameter : public FloatParameter
    {
    public:
        GainDBParameter(const ParameterID &parameterID,
                        const String &paramName,
                        const NormalisableRange<float> &paramRange,
                        float defaultValue)
            : FloatParameter(parameterID,
                             paramName,
                             paramRange,
                             defaultValue,
                             &gainValToString,
                             &stringToGainVal)
        {
        }

        using Ptr = Utils::OptionalPointer<GainDBParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainDBParameter)
    };

    /** A float parameter which specifically stores a frequency value in Hertz. */
    class FreqHzParameter : public FloatParameter
    {
    public:
        FreqHzParameter(const ParameterID &parameterID,
                        const String &paramName,
                        const NormalisableRange<float> &paramRange,
                        float defaultValue)
            : FloatParameter(parameterID,
                             paramName,
                             paramRange,
                             defaultValue,
                             &freqValToString,
                             &stringToFreqVal)
        {
        }

        using Ptr = Utils::OptionalPointer<FreqHzParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FreqHzParameter)
    };

    /** A float parameter which specifically stores a time value in milliseconds. */
    class TimeMsParameter : public FloatParameter
    {
    public:
        TimeMsParameter(const ParameterID &parameterID,
                        const String &paramName,
                        const NormalisableRange<float> &paramRange,
                        float defaultValue)
            : FloatParameter(parameterID,
                             paramName,
                             paramRange,
                             defaultValue,
                             &timeMsValToString,
                             &stringToTimeMsVal)
        {
        }

        using Ptr = Utils::OptionalPointer<TimeMsParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeMsParameter)
    };

    /** A float parameter which specifically stores a ratio value. */
    class RatioParameter : public FloatParameter
    {
    public:
        RatioParameter(const ParameterID &parameterID,
                       const String &paramName,
                       const NormalisableRange<float> &paramRange,
                       float defaultValue)
            : FloatParameter(parameterID,
                             paramName,
                             paramRange,
                             defaultValue,
                             &ratioValToString,
                             &stringToRatioVal)
        {
        }

        using Ptr = Utils::OptionalPointer<RatioParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatioParameter)
    };

    /** A float parameter which specifically stores a semitones value. */
    class SemitonesParameter : public FloatParameter
    {
    public:
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE (
        "-Wcomma"
        )
        SemitonesParameter(const ParameterID &parameterID,
                           const juce::String &paramName,
                           juce::NormalisableRange<float> paramRange,
                           float defaultValue,
                           bool snapToInt = false)
            : FloatParameter(
                parameterID,
                paramName,
                (paramRange.interval = snapToInt ? 1.0f : paramRange.interval, paramRange),
                defaultValue,
                [snapToInt](float val)
                {
                    return semitonesValToString(val, snapToInt);
                },
                &stringToSemitonesVal)
        {
        }

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        using Ptr = Utils::OptionalPointer<SemitonesParameter>;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SemitonesParameter)
    };

    inline FloatParameter::FloatParameter (const ParameterID& parameterID,
                                const String& parameterName,
                                const NormalisableRange<float>& valueRange,
                                float defaultFloatValue,
                                const std::function<String (float)>& valueToTextFunction,
                                std::function<float (const String&)>&& textToValueFunction)
#if JUCE_VERSION < 0x070000
    : juce::AudioParameterFloat (
        parameterID,
        parameterName,
        valueRange,
        defaultFloatValue,
        juce::String(),
        AudioProcessorParameter::genericParameter,
        valueToTextFunction == nullptr
            ? std::function<juce::String (float v, int)>()
            : [valueToTextFunction] (float v, int)
            { return valueToTextFunction (v); },
        std::move (textToValueFunction)),
#else
    : juce::AudioParameterFloat (
        parameterID,
        parameterName,
        valueRange,
        defaultFloatValue,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction (
                [valueToTextFunction] (float v, int)
                { return valueToTextFunction (v); })
            .withValueFromStringFunction (std::move (textToValueFunction))),
#endif
      unsnappedDefault (valueRange.convertTo0to1 (defaultFloatValue)),
      normalisableRange (valueRange)
    {
    }

    inline void FloatParameter::applyMonophonicModulation (double modulationValue)
    {
        modulationAmount = static_cast<float>(modulationValue);
    }

    inline float FloatParameter::getCurrentValue() const noexcept
    {
        return normalisableRange.convertFrom0to1 (jlimit (0.0f, 1.0f, normalisableRange.convertTo0to1 (get()) + modulationAmount));
    }
}
#endif
