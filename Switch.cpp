#include "CustomLookAndFeel.h"

#include "Switch.h"

Switch::Switch() {

    setInterceptsMouseClicks(true, false);
}

void Switch::SetInnerLabel(const std::string& innerLabel) {
    this->innerLabel = innerLabel;
}

void Switch::paint(juce::Graphics& g) {

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Background
    g.setColour(state ? CustomLookAndFeel::GetTheme()->colour_switchOn : CustomLookAndFeel::GetTheme()->colour_switchOff);
    g.fillRect(bounds.reduced(0, 16));

    // Text
    g.setColour(juce::Colours::white);
    g.setFont(CustomLookAndFeel::GetTheme()->font);
    g.drawText(innerLabel, bounds, juce::Justification::centred, true);
}

void Switch::mouseDown(const juce::MouseEvent& e) {

    state = !state;
    repaint();
}

bool Switch::GetState() const 
{ 
    return state; 
}

void Switch::SetState(bool newState) {
    this->state = newState;
    this->stateRecentlyEstablished = true;
    repaint();
}

void Switch::SetStateRecentlyEstablished(bool newState) {
    this->stateRecentlyEstablished = newState;
}

bool Switch::GetStateRecentlyEstablished() {
    return this->stateRecentlyEstablished;
}