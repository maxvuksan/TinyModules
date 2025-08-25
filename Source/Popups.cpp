#include "Popups.h"

Popups* Popups::instance;

Popups::Popups() {

    instance = this;

    setInterceptsMouseClicks(false, true);

    addAndMakeVisible(popupBrowser);
    addAndMakeVisible(popupSettings);
}


void Popups::resized() {

    OpenPopup(NO_POPUP);

    int headerSize = 40;
    popupBrowser.setBounds(getLocalBounds().withTrimmedTop(headerSize).reduced(700, 50));
    popupSettings.setBounds(getLocalBounds().withTrimmedTop(headerSize).reduced(700, 50));

}

void Popups::paint(juce::Graphics& g) {

    if (currentPopup == NO_POPUP) {
        return;
    }

    // provide fade panel to highlight popup

    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill.withAlpha(0.4f));
    g.fillRect(getLocalBounds());
}



PopupType Popups::GetPopup() {
    return instance->currentPopup;
}

bool Popups::IsPopupOpen() {

    if (instance->currentPopup == NO_POPUP) {
        return false;
    }
    return true;
}

void Popups::OpenPopup(PopupType type) {

    if (instance->currentPopup == type) {
        return;
    }

    instance->currentPopup = type;

    // turn off all popups...
    instance->popupBrowser.setVisible(false);
    instance->popupSettings.setVisible(false);


    // turn back on correct popup...
    switch (type) {
        
        case POPUP_BROWSER:
            instance->popupBrowser.setVisible(true);
            break;

        case POPUP_SETTINGS:
            instance->popupSettings.setVisible(true);
            break;
    }       

    instance->repaint();
}

void Popups::ClosePopup() {
    OpenPopup(NO_POPUP);
}