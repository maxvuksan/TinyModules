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
    setResizable(true, false);
    addAndMakeVisible(rackViewport);

    addAndMakeVisible(moduleBrowser);
    moduleBrowser.SetOpenState(false);




    auto* topLevel = juce::TopLevelWindow::getTopLevelWindow(0);
    if (topLevel) {
        topLevel->setUsingNativeTitleBar(true);
    }

    addAndMakeVisible(headerBar);
    headerBar.setBounds(0, 0, getWidth(), 40);

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
    g.fillAll(CustomLookAndFeel::GetTheme()->colour_backgroundRackOdd);
}

void Juce_SketchRackAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    headerBar.setBounds(getLocalBounds().removeFromTop(40));
    rackViewport.setBounds(getLocalBounds()); // Fill the plugin window

    moduleBrowser.setBounds(rackViewport.getLocalBounds().withTrimmedTop(headerBar.getHeight()).reduced(100, 50));
}

bool Juce_SketchRackAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        NewModulePopup::SetOpenState(false);
        return true;
    }

    return false; // Pass through unhandled keys
}