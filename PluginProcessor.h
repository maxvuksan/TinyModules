/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include "CustomLookAndFeel.h"
#include <JuceHeader.h>

class RackView;
class ProcessingManager;

struct AudioConfig {

    double sampleRate = 0;
    int blockSize = 0;
};


//==============================================================================
/**
*/
class Juce_SketchRackAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Juce_SketchRackAudioProcessor();
    ~Juce_SketchRackAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void SetRackView(RackView* rackView);
    void SetProcessingManager(ProcessingManager* processingManager);

private:

    CustomLookAndFeel customLookAndFeel;

    RackView* rackView;
    ProcessingManager* processingManager;

    AudioConfig audioConfig;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Juce_SketchRackAudioProcessor)
};
