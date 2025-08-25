#pragma once
#include <JuceHeader.h>
#include "WireSocket.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "KnobModulationWheel.h"

enum KnobColourType {
    KNOB_COL_DEFAULT,               // default knob colour
    KNOB_COL_CONTROL_SELECTED,      // colours this knob to match selected graphic (blue)
};

struct KnobConfiguration {
    double min = 0;
    double max = 2;
    double increment = 0.01;
    double defaultValue = 1;
    bool skewAroundDefault = false;     // makes knob non linear
    KnobColourType colourType = KNOB_COL_DEFAULT;
};


class Knob : public juce::Slider {

	public:

        struct ConnectedWire {

            WireSocket* otherSocket;
            KnobModulationWheel* modulationWheel;
        };

        Knob();
        /* 
            copies and applies the parameters specified
            @param configuration 
        */
		void Configure(KnobConfiguration* configuration); 

        /*
            sets a knob to to be enabled or disabled, this is purley visual and is used to tell the end user when a knob 
            actually has influence on the processing (e.g. An input socket overrides a knob, so the knob is disabled)
        */
        void SetVisuallyEnabled(bool state);

        // the value manually input by the user
        void SetManualValue(double value);

        void AddConnectedWire(WireSocket* otherSocket);
        void RemoveConnectedWire(WireSocket* otherSocket);
        void RemoveAllConnectedWires();

        void PositionModulationWheels();

        void paint(juce::Graphics& g) override;
        void moved() override;
        void resized() override;
        void valueChanged() override;
        void mouseDown(const juce::MouseEvent& e) override;

    private:

        std::vector<ConnectedWire> connectedWires;

        KnobColourType colourType;
        float defaultValue;
        bool isMenuOpen = false;
        bool visuallyEnabled = false;
};