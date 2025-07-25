#pragma once; 
#include <JuceHeader.h>
#include "WireSocket.h"
#include "Globals.h"

struct ComponentConfig {

    juce::Point<int> coordinate;
    juce::Point<int> size;
};


struct ComponentKnob {

    ComponentConfig config;
    juce::Slider slider;
};

/*
    
*/
struct ComponentSocket {

    ComponentConfig config;
    WireSocket socket;
};




struct ComponentScope {
    ComponentConfig config;
    juce::AudioVisualiserComponent visualiser;

    ComponentScope() : visualiser(1)  // 1 channel, or 2 for stereo
    {
        visualiser.setBufferSize(128);  // Set size as needed
        visualiser.setSamplesPerBlock(16); // or match your processing block size
        visualiser.setColours(juce::Colours::black, GLOBAL_WIRE_COLOUR_POOL[0]);
    }
};