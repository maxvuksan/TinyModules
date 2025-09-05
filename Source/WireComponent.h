#pragma once
#include <JuceHeader.h>
#include "Connection.h"




class WireComponent : public juce::Component
{
    public:

        WireComponent();

        void SetStartEnd(juce::Point<float> start, juce::Point<float> end, bool endIsKnob = false);

        void SetWireColourIndex(int index);
        int GetWireColourIndex();
        void SetConnectionType(ConnectionType connectionType);
        
        void paint(juce::Graphics& g) override;

    private:

        bool endIsKnob;
        ConnectionType connectionType;
        int colourIndex;
        juce::Point<float> startPos, endPos;
};
