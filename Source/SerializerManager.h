#pragma once
#include <JuceHeader.h>
#include "RackView.h"
/*
	responsible for saving the plugin state (when used as a VST) 
	and saving and loading patches from a file. 
*/
class SerializerManager {

	public:
        
        /*
            Converts the RackView state to a tree structure which can be written to a file
        */
        static juce::var SerializePatch(const RackView& state);
        /*
            Given serialized state, loads the state
        */
        static void DeserializePatch(const juce::var& patchData, RackView& rack);

        /*
            Writes the RackView state to a file
        */
        static void SaveToFile(const RackView& state, const juce::File& file);
        /*
            Loads a file into the RackView state
        */
        static void LoadFromFile(const juce::File& file, RackView& rack);


        /*
            Saves the current RackView patch into the plugin's state memory block,
            which is used by the host for VST/AU state saving (e.g., DAW project).
        */
        static void SaveToPluginState(const RackView& state, juce::MemoryBlock& dest);
        /*
            Loads a previously saved plugin state from the host (e.g., VST project),
            restoring the RackView patch from the given memory block.
        */
        static void LoadFromPluginState(const void* data, int size, RackView& rack);

};