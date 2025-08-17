#include "Module_Sequencer.h"

Module_Sequencer::Module_Sequencer() : Module::Module(7, "trigger sequencer") {


    KnobConfiguration config;
    config.min = -5.0f;
    config.max = 5.0f;
    config.defaultValue = 0.0f;
    config.increment = 0.01f;
    config.colourType = KNOB_COL_CONTROL_SELECTED;

    Component_CreateKnob("v/oct", 5, 0, &config);

    for (int i = 0; i < 4; i++) {
        
        for (int x = 0; x < 8; x++) {
            // assign default knob values
            vOctValues[x][i] = config.defaultValue;
        }

        switchGrid[i] = std::make_unique<SwitchGrid>(8, 1);
        addAndMakeVisible(*switchGrid[i]);
        SetComponentBounds(*switchGrid[i], 1, i + 1, 4, 1);
        
        stepIndex[i] = 0;
    }

    selectedX = 0;
    selectedY = 0;
    previousSelectedY = -1;
    switchGrid[selectedY]->SetOutlined(selectedX, 0);





    // assign callbacks for selecting cells
    for (int i = 0; i < 4; i++) {

        // after switchGrid[i] creation

        switchGrid[i]->onClick = [this, i](int x, int y, SwitchGrid* grid) {

            // Save knob value into the current cell
            vOctValues[selectedX][selectedY] = Component_GetKnobValue("v/oct");

            // Remove outline from all grids
            for (int j = 0; j < 4; j++) {
                if (switchGrid[j]) {
                    switchGrid[j]->SetOutlined(-1, -1);
                    switchGrid[j]->repaint();
                }
            }

            previousSelectedY = selectedY;

            // Apply outline to clicked
            selectedX = x;
            selectedY = i;
            switchGrid[i]->SetOutlined(x, y);
            switchGrid[i]->repaint();

            Component_GetKnob("v/oct")->SetManualValue(vOctValues[x][i]);
        };
    }


    Component_CreateInputSocket("init", 0, 0); // sets the step index to -1 (ready for first step)
    Component_CreateInputSocket("step", 1, 0); 


    Component_CreateOutputSocket("trig", 6, 1);
    Component_CreateOutputSocket("#Trig R2", 6, 2);
    Component_CreateOutputSocket("#Trig R3", 6, 3);
    Component_CreateOutputSocket("#Trig R4", 6, 4);
    Component_CreateOutputSocket("trig all", 6, 0);

    Component_CreateOutputSocket("v/oct0", 5, 1);
    Component_CreateOutputSocket("v/oct1", 5, 2);
    Component_CreateOutputSocket("v/oct2", 5, 3);
    Component_CreateOutputSocket("v/oct3", 5, 4);

}


juce::var Module_Sequencer::SerializeCustom()
{
    juce::DynamicObject* rootObj = new juce::DynamicObject();

    // Serialize vOctValues
    juce::Array<juce::var> vOctArray;

    for (int x = 0; x < 8; ++x)
    {
        juce::Array<juce::var> row;
        for (int y = 0; y < 4; ++y)
        {
            row.add(vOctValues[x][y]);
        }
        vOctArray.add(row);
    }

    rootObj->setProperty("vOctValues", vOctArray);

    // Serialize each switch grid
    juce::Array<juce::var> switchGridArray;
    for (int i = 0; i < 4; ++i)
    {
        if (switchGrid[i])
            switchGridArray.add(switchGrid[i]->Serialize());
        else
            switchGridArray.add(juce::var()); // Empty if null
    }

    rootObj->setProperty("switchGrids", switchGridArray);

    return rootObj;
}

void Module_Sequencer::DeserializeCustom(const juce::var& data)
{
    if (auto* obj = data.getDynamicObject())
    {
        // Deserialize vOctValues
        auto vOctArray = obj->getProperty("vOctValues");
        if (vOctArray.isArray())
        {
            auto* arr = vOctArray.getArray();
            for (int x = 0; x < 8 && x < arr->size(); ++x)
            {
                auto row = arr->getReference(x);
                if (row.isArray())
                {
                    auto* rowArr = row.getArray();
                    for (int y = 0; y < 4 && y < rowArr->size(); ++y)
                    {
                        vOctValues[x][y] = (float)rowArr->getReference(y);
                    }
                }
            }
        }

        // Deserialize each switch grid
        auto switchGridArray = obj->getProperty("switchGrids");
        if (switchGridArray.isArray())
        {
            auto* arr = switchGridArray.getArray();
            for (int i = 0; i < 4 && i < arr->size(); ++i)
            {
                if (switchGrid[i])
                    switchGrid[i]->Deserialize(arr->getReference(i));
            }
        }
    }
}



void Module_Sequencer::Reset() {
    previousClockHigh = false;
    previousStepResetHigh = false;
    StepReset();
}

void Module_Sequencer::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 2, 9);
}

void Module_Sequencer::Process() {


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

            // send trigger outputs

            bool anyTriggerSent = false;

            for (int i = 0; i < 4; i++) {

                // switch must be on to recieve trigger output
                if (!switchGrid[i]->GetSwitchState(stepIndex[i], 0)) {
                    continue;
                }

                anyTriggerSent = true;

                float* writePtr = GetOutputWritePtr(i);
                std::fill_n(writePtr, GetOutputBuffer(0).getNumSamples(), 5.0f); // 5V
            }

            // send global trigger

            if (anyTriggerSent) {

                float* writePtr = GetOutputWritePtr(4);
                std::fill_n(writePtr, GetOutputBuffer(0).getNumSamples(), 5.0f); // 5V
            }

        }


        // output v/oct signals if needed
        for (int i = 0; i < 4; i++) {

            if (stepIndex[i] < 0) {
                return;
            }

            // cancel processing if no connections
            if (!Component_GetSocket("v/oct" + std::to_string(i))->HasConnection()) {
                continue;
            }

            float* vOctPtr = GetOutputWritePtr(5 + i);
        
            float vOctVal = vOctValues[stepIndex[i]][i];

            // read directly from knob instead (because result has not been saved to array)
            if (selectedX == stepIndex[i] && selectedY == i) {
                vOctVal = Component_GetKnobValue("v/oct");
            }

            std::fill_n(vOctPtr, GetOutputBuffer(0).getNumSamples(), vOctVal);
            
        }

        previousClockHigh = currentClockHigh;
    }

}

void Module_Sequencer::Step() {

    for (int i = 0; i < 4; i++) {

        stepIndex[i]++;

        if (stepIndex[i] >= 8) {
            stepIndex[i] = 0;
        }

        switchGrid[i]->SetHighlighted(stepIndex[i], 0);

        // triggering repaint but we are in wrong thread

        juce::MessageManager::callAsync([this] {

            for (int i = 0; i < 4; i++) {
                if (switchGrid[i]) {
                    switchGrid[i]->repaint();
                }
            }
        });
    }
}

void Module_Sequencer::StepReset() {

    for (int i = 0; i < 4; i++) {

        stepIndex[i] = -1;

        switchGrid[i]->SetHighlighted(stepIndex[i], 0);

        juce::MessageManager::callAsync([this] {

            for (int i = 0; i < 4; i++) {
                if (switchGrid[i]) {
                    switchGrid[i]->repaint();
                }
            }
        });
    }
}

