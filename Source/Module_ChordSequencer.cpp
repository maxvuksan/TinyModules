#include "Module_ChordSequencer.h"
#include "DSPUtility.h"


Module_ChordSequencer::Module_ChordSequencer() : Module(8, "chords") {


    for (int i = 0; i < 4; i++) {
        pianoSwitch[i] = std::make_unique<PianoSwitch>();

        addAndMakeVisible(*pianoSwitch[i]);
        SetComponentBounds(*pianoSwitch[i], i * 2, 1, 2, 4);
    }

    Component_CreateInputSocket("init", 0, 0);
    Component_CreateInputSocket("step", 1, 0);
    Component_CreateOutputSocket("v/oct", 7, 0, CONNECT_POLY);

    Component_CreateKnob("glide", 3, 0, nullptr);

    KnobConfiguration knobConfig;
    knobConfig.defaultValue = 0;
    knobConfig.min = -4;
    knobConfig.max = 4;
    knobConfig.increment = 1;

    Component_CreateKnob("oct#0", 0, 5, &knobConfig);
    Component_CreateKnob("oct#1", 2, 5, &knobConfig);
    Component_CreateKnob("oct#2", 4, 5, &knobConfig);
    Component_CreateKnob("oct#4", 6, 5, &knobConfig);
}

juce::var Module_ChordSequencer::SerializeCustom()
{
    juce::DynamicObject* rootObj = new juce::DynamicObject();

    // Array to store all 4 piano switches
    juce::Array<juce::var> pianoArray;

    for (int i = 0; i < 4; ++i)
    {
        if (pianoSwitch[i]){
            pianoArray.add(pianoSwitch[i]->Serialize());
        }
        else {
            pianoArray.add(juce::var()); // empty placeholder
        }
    }

    rootObj->setProperty("pianoSwitches", pianoArray);

    return rootObj;
}

void Module_ChordSequencer::DeserializeCustom(const juce::var& data)
{
    if (auto* obj = data.getDynamicObject())
    {
        auto pianoArray = obj->getProperty("pianoSwitches");
        
        if (pianoArray.isArray())
        {
            auto* arr = pianoArray.getArray();

            for (int i = 0; i < 4 && i < arr->size(); ++i)
            {
                if (pianoSwitch[i]) {
                    pianoSwitch[i]->Deserialize(arr->getReference(i));
                }
            }
        }
    }
}


void Module_ChordSequencer::Reset() {
}
void Module_ChordSequencer::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 2, 1);
}

void Module_ChordSequencer::Process() {

    for (int n = 0; n < GetInputBuffer(0).getNumSamples(); n++) {

        bool currentClockHigh = GetInputReadPtr(1)[n] > 0.5f;
        bool currentStepResetHigh = GetInputReadPtr(0)[n] > 0.5f;

        if (!previousStepResetHigh && currentStepResetHigh) {
            StepReset();
        }
        previousStepResetHigh = currentStepResetHigh;

        // Rising edge detection: previously low, now high
        if (!previousClockHigh && currentClockHigh) {

            Step();  // Advance the sequencer

        }
        previousClockHigh = currentClockHigh;
    }

    if (stepIndex < 0) {
        return;
    }

    juce::AudioBuffer<float>& vOctOut = GetOutputBuffer(0);

    int numActiveVoices = pianoSwitch[stepIndex]->WriteVoltagesToPolyphonicBuffer(vOctOut, vOctOut.getNumSamples(), Component_GetKnobValue("oct#" + std::to_string(stepIndex)));
    Component_GetSocket("v/oct")->SetNumActiveVoices(numActiveVoices);
}

void Module_ChordSequencer::Step() {
    stepIndex++;

    if (stepIndex >= 4) {
        stepIndex = 0;
    }
}

void Module_ChordSequencer::StepReset() {
    stepIndex = -1;
}

