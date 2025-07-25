#pragma once
#include <JuceHeader.h>

enum ConnectionType {
    CONNECT_MONO,   // 1    channel
    CONNECT_POLY,   // 16   channels
};


class WireComponent : public juce::Component
{
    public:

        void SetStartEnd(juce::Point<float> start, juce::Point<float> end);
        void SetWireColourIndex(int index);
        void SetConnectionType(ConnectionType connectionType);
        
        void paint(juce::Graphics& g) override;

    private:

        ConnectionType connectionType;
        int colourIndex;
        juce::Point<float> startPos, endPos;
};
