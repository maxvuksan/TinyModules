#include "WireSocket.h"

namespace DSP {

    enum WaveType { 
        Sine, 
        Square, 
        Saw, 
        Triangle, 
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
