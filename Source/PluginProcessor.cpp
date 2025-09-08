/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Juce_SketchRackAudioProcessor::Juce_SketchRackAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    processingManager = nullptr;
    rackView = nullptr;
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);
}

Juce_SketchRackAudioProcessor::~Juce_SketchRackAudioProcessor()
{

}

//==============================================================================
const juce::String Juce_SketchRackAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Juce_SketchRackAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Juce_SketchRackAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Juce_SketchRackAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Juce_SketchRackAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Juce_SketchRackAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Juce_SketchRackAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Juce_SketchRackAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Juce_SketchRackAudioProcessor::getProgramName (int index)
{
    return {};
}

void Juce_SketchRackAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Juce_SketchRackAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    audioConfig.sampleRate = sampleRate;
    audioConfig.blockSize = samplesPerBlock;


//    if (processingManager == nullptr || rackView == nullptr || !processingManager->IsProcessingEnabled()) {
  //      return;
    //}

}

void Juce_SketchRackAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Juce_SketchRackAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Juce_SketchRackAudioProcessor::SetRackView(RackView* rackView) {
    this->rackView = rackView;
}
void Juce_SketchRackAudioProcessor::SetProcessingManager(ProcessingManager* processingManager) {
    this->processingManager = processingManager;
}


void Juce_SketchRackAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    if (processingManager == nullptr || rackView == nullptr || !processingManager->IsProcessingEnabled()) {
        return;
    }

    rackView->SetAudioConfig(audioConfig);

    processingManager->PrepareAll(audioConfig.sampleRate, audioConfig.blockSize);
    processingManager->ProcessAll();


    if (rackView->outputBlock == nullptr) {
        return;
    }

    // Get the output buffer from the final output module

    auto& outputBuffer = rackView->outputBlock->GetOutputBuffer(0);

    if (&outputBuffer == nullptr) {
        buffer.clear();
        return;
    }

    // Ensure safety
    jassert(outputBuffer.getNumChannels() >= 1);
    jassert(outputBuffer.getNumSamples() == buffer.getNumSamples());

    // Copy mono output to all output channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        buffer.copyFrom(ch, 0, outputBuffer, 0, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool Juce_SketchRackAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Juce_SketchRackAudioProcessor::createEditor()
{
    return new Juce_SketchRackAudioProcessorEditor (*this);
}

//==============================================================================
void Juce_SketchRackAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Juce_SketchRackAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Juce_SketchRackAudioProcessor();
}
