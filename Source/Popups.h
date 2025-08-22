#pragma once
#include <unordered_map>
#include <JuceHeader.h>

#include "Popup_ModuleBrowser.h"
#include "Popup_Settings.h"

enum PopupType {

	NO_POPUP,
	POPUP_BROWSER,
	POPUP_SETTINGS,
};


/*
	an interface to open and close popup windows. is the parent component for all popups
*/
class Popups : public juce::Component {

	public:

		Popups();

		void resized() override;
		void paint(juce::Graphics& g);

		static bool IsPopupOpen();
		
		/*
			@returns the type of the active popup, will return the NO_POPUP enum if no popup active
		*/
		static PopupType GetPopup();

		/*
			opens a specific popup, closes all other types

			@param popupType	the associated popup enum we want to open
		*/
		static void OpenPopup(PopupType popupType);

		/*
			closes the active popup, regardless of its type
		*/
		static void ClosePopup();

	private:

		static Popups* instance;

		PopupType currentPopup;

		Popup_ModuleBrowser popupBrowser;
		Popup_Settings popupSettings;
};
