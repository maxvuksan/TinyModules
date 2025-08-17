#pragma once
#include "WireSocket.h"

namespace DSP {

    enum WaveType { 
        Sine, 
        Square, 
        Saw, 
        Triangle, 
    };

    const float NOTE_VOLTAGES[]{
        0.0 / 12.0,   // C4  = 0.000 V
        1.0 / 12.0,   // C#4 = 0.083 V
        2.0 / 12.0,   // D4  = 0.167 V
        3.0 / 12.0,   // D#4 = 0.250 V
        4.0 / 12.0,   // E4  = 0.333 V
        5.0 / 12.0,   // F4  = 0.417 V
        6.0 / 12.0,   // F#4 = 0.500 V
        7.0 / 12.0,   // G4  = 0.583 V
        8.0 / 12.0,   // G#4 = 0.667 V
        9.0 / 12.0,   // A4  = 0.750 V
        10.0 / 12.0,  // A#4 = 0.833 V
        11.0 / 12.0   // B4  = 0.917 V
    };

    float AverageSamples(const float* readBufferPointer, int numberOfSamples);

    float SampleWaveform(WaveType type, float phase);

    float GetPhaseIncrement(float frequency, double sampleRate);
    void IncrementPhase(float& phase, float phaseIncrement);

    float Lerp(float current, float target, float amount = 0.01f);

    // converts BPM (beats per minute) to frequency (hertz)
    float BpmToFrequency(float bpm);

    float VoltageToSemitone(float voltage);
    float SemitoneToVoltage(float semitone);
    float VoltageToFrequency(float voltage);
    float FrequencyToVoltage(float frequency);
};
