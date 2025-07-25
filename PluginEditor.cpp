/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Juce_SketchRackAudioProcessorEditor::Juce_SketchRackAudioProcessorEditor (Juce_SketchRackAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1400, 1000);
    addAndMakeVisible(rackViewport);

    rackViewport.addAndMakeVisible(&rack, false);

    audioProcessor.SetRackView(&rack);
    audioProcessor.SetProcessingManager(&rack.processingManager);
}

Juce_SketchRackAudioProcessorEditor::~Juce_SketchRackAudioProcessorEditor()
{

}

//==============================================================================
void Juce_SketchRackAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(GLOBAL_BG_COLOUR_DARK);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Midi Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void Juce_SketchRackAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    rackViewport.setBounds(getLocalBounds()); // Fill the plugin window

}
