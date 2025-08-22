/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Popups.h"
#include "HeaderBar.h"
#include "Globals.h"
#include "Module.h"
#include "RackView.h"

//==============================================================================
/**
*/
class Juce_SketchRackAudioProcessorEditor  : public juce::AudioProcessorEditor
{
    friend class Juce_SketchRackAudioProcessor;

public:
    Juce_SketchRackAudioProcessorEditor (Juce_SketchRackAudioProcessor&);
    ~Juce_SketchRackAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key);
    
    HeaderBar headerBar;
    Popups popups;

    juce::Viewport rackViewport;
    RackView rack;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Juce_SketchRackAudioProcessor& audioProcessor;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Juce_SketchRackAudioProcessorEditor)
};
