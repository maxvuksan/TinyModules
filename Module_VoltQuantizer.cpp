#include "Module_VoltQuantizer.h"
#include "DSPUtility.h"


Module_VoltQuantizer::Module_VoltQuantizer() : Module(2, "quantizer") {

    pianoSwitch = std::make_unique<PianoSwitch>();
    addAndMakeVisible(*pianoSwitch);
    SetComponentBounds(*pianoSwitch, 0, 1, 2, 4);

    Component_CreateInputSocket ("v in", 0, 5);
    Component_CreateOutputSocket("v out", 1, 5);
    Component_CreateOutputSocket("trig", 1, 6);
}

void Module_VoltQuantizer::Reset() {
    previousBestMatch = -1;
}
void Module_VoltQuantizer::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 2);
}

void Module_VoltQuantizer::Process(){

    const float* input = GetInputReadPtr(0);
    float* vOctOut = GetOutputWritePtr(0);
    float* trigOut = GetOutputWritePtr(1);

    int triggerSampleCount = 99;

    for (int i = 0; i < GetOutputBuffer(0).getNumSamples(); ++i) {

        float voltageIn = input[i];

        // Convert voltage to floating point semitone value

        float semitoneFloat = DSP::VoltageToSemitone(voltageIn);

        int octave = static_cast<int>(std::floor(semitoneFloat / 12.0f));
        int semitoneInOctave = static_cast<int>(std::round(semitoneFloat)) % 12;

        if (semitoneInOctave < 0){
            semitoneInOctave += 12;
        }
        // Find nearest enabled note in keyStates

        int bestMatch = -1;
        int bestDistance = 128;

        for (int j = 0; j < 12; ++j) {

            if (!pianoSwitch->GetKeyState(j)) {
                continue;
            }

            int dist = std::abs(j - semitoneInOctave);
            
            if (dist < bestDistance) {
                bestDistance = dist;
                bestMatch = j;
            }
        }

        // update graphic...
        juce::MessageManager::callAsync([this, bestMatch]() {
            if (pianoSwitch) {
                pianoSwitch->SetHighlightedKey(bestMatch);
            }
        });

        // new note triggered
        if (previousBestMatch != bestMatch) {
            triggerSampleCount = 0;
            trigOut[i] = 5.0;
        }
        else if(triggerSampleCount < 8) {
            triggerSampleCount++;
            trigOut[i] = 5.0;
        }
        else {
            triggerSampleCount = 99;
            trigOut[i] = 0.0;
        }


        previousBestMatch = bestMatch;

        if (bestMatch >= 0) {
            float quantizedVoltage = (octave * 12 + bestMatch) / 12.0f;

            vOctOut[i] = quantizedVoltage;
        }
        else {
            vOctOut[i] = voltageIn; // pass-through if no notes are enabled
        }
    }
}
