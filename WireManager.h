#pragma once
#include <JuceHeader.h>
#include "WireComponent.h"

class WireManager : public juce::Component
{

    public:

        WireManager();

        WireComponent* createWire();
        void removeWire(WireComponent* wire);

        static WireManager* instance;

    private:

        static int currentWireColourIndex;

        std::vector<std::unique_ptr<WireComponent>> wires;
        std::vector<std::unique_ptr<WireComponent>> vacantWires;
};
