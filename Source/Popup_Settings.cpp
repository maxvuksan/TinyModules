#include "Popup_Settings.h"

Popup_Settings* Popup_Settings::instance;

Popup_Settings::Popup_Settings() {

    instance = this;

    // init state ----------------------------------------------------

    state = juce::ValueTree("settings");
    if (!state.hasProperty("wireTension")) {
        state.setProperty("wireTension", 0.5, nullptr); // default
    }

    // link to state 

    settings.wireTension.referTo(state.getPropertyAsValue("wireTension", nullptr));


    // Prepare Values bound to your state (or PropertiesFile, etc.)
    settings.wireTension.referTo(state.getPropertyAsValue("wireTension", nullptr));


    juce::Array<juce::PropertyComponent*> comps;

    comps.add(new juce::SliderPropertyComponent(settings.wireTension, "Wire Tension", 0, 1, 0.01));


    props.addProperties(comps);

    addAndMakeVisible(props);
}

void Popup_Settings::paint(juce::Graphics& g) {

	g.setColour(CustomLookAndFeel::GetTheme()->colour_headerBar);
	g.fillRect(getLocalBounds());

    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobPointer);
    g.drawRect(getLocalBounds());
}

void Popup_Settings::resized() {
    props.setBounds(getLocalBounds().reduced(12));
}

const SettingsData& Popup_Settings::GetSettings() {
    return instance->settings;
}