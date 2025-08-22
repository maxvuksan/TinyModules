
#pragma once
#include <JuceHeader.h>
#include "TextList.h"
#include "CustomLookAndFeel.h"
#include "SearchBar.h"

struct SettingsData {

	juce::Value

		wireTension;

};

/*
	Provides an interface to change settings
*/
class Popup_Settings : public juce::Component {

	public:

		Popup_Settings();

		void paint(juce::Graphics& g);
		void resized();

		static const SettingsData& GetSettings();

	private:
		
		static Popup_Settings* instance;

		juce::ValueTree state;
		juce::PropertyPanel props;

		SettingsData settings;

};