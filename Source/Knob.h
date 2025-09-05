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
        
        /*
            @returns the value of the knob after any modulation is applied
        */
        double GetValue(int sampleNum);

        void RecomputeWireGraphics();

        /*
            creates a modulation wheel from a socket to this knob

            @param      otherSocket         the socket outputting the modulation signal
            @param      wireColourIndex     the colour of the wire (modulation wheel will match this colour)
            @param      modulationValue     a default strength value assigned to the modulation wheel
        */
        void AddConnectedWire(WireSocket* otherSocket, int wireColourIndex = 0, float modulationValue = 0);
        void RemoveConnectedWire(WireSocket* otherSocket);
        void RemoveAllConnectedWires();

        /*
            iterates over each modulation wheel, returning the one connected to otherSocket
        */
        KnobModulationWheel* GetModulationWheel(WireSocket* otherOutSocket);

        void PositionModulationWheels();

        void paint(juce::Graphics& g) override;
        void moved() override;
        void resized() override;
        void valueChanged() override;
        void mouseDown(const juce::MouseEvent& e) override;

        void SetModule(Module* _module) { this->module = _module; }
        Module* GetModule() { return module; }

        void SetLabel(const std::string& _label);
        const std::string& GetLabel() { return label; }

    private:

        std::vector<ConnectedWire> connectedWires;

        KnobColourType colourType;

        Module* module;
        std::string label;

        float modScalePositive = 0;
        float modScaleNegative = 0;
        double minVal;
        double maxVal;

        float defaultValue;
        bool isMenuOpen = false;
        bool visuallyEnabled = false;
};