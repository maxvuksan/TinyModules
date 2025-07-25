#include "DSPUtility.h"
#include "Module.h"

namespace DSP {


    float AverageSamples(const float* readBufferPointer, int numberOfSamples) {

        float average = 0;

        for (int i = 0; i < numberOfSamples; ++i) {
            average += readBufferPointer[i];
        }

        average /= (float)numberOfSamples;

        return average;
    }

    float GetPhaseIncrement(float frequency, double sampleRate) {
        return (2.0f * juce::MathConstants<float>::pi * frequency) / sampleRate;
    }

    void IncrementPhase(float& phase, float phaseIncrement) {

        phase += phaseIncrement;

        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;
    }

    float SampleWaveform(WaveType type, float phase) {

        float sample = 0.0f;

        switch (type) {

            case Sine:
                sample = std::sin(phase);
                break;
            case Square:
                sample = std::sin(phase) >= 0.0f ? 1.0f : -1.0f;
                break;
            case Saw:
                sample = (2.0f / juce::MathConstants<float>::pi) * (phase - juce::MathConstants<float>::pi);
                break;
            case Triangle:
                sample = std::asin(std::sin(phase)) * (2.0f / juce::MathConstants<float>::pi);
                break;
        }

        return sample;
    }


    float Lerp(float current, float target, float amount) {
        return current + (target - current) * amount;
    }


    float BpmToFrequency(float bpm) {
        return bpm / 60.0f;
    }


    float VoltageToSemitone(float voltage) {
        return voltage * 12.0f;
    }

    float SemitoneToVoltage(float semitone) {
        return semitone / 12.0f;
    }

    float VoltageToFrequency(float voltage) {
        return 440.0f * std::pow(2.0f, voltage);
    }
    float FrequencyToVoltage(float frequency) {
        return std::log2(frequency / 440.0);
    }
};