#include "CustomLookAndFeel.h"

CustomTheme* CustomLookAndFeel::theme;

void CustomLookAndFeel::Init() {

	theme = new CustomTheme;
}

CustomTheme* CustomLookAndFeel::GetTheme() {

	jassert(theme != nullptr);
	return theme;
}

void CustomLookAndFeel::Destruct() {
	delete theme;
	theme = nullptr;
}

CustomLookAndFeel::CustomLookAndFeel()
{
	if (theme == nullptr) {
		CustomLookAndFeel::Init();
	}

    // Load the font from binary data
    theme->typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::RalewayMedium_ttf,
        BinaryData::RalewayMedium_ttfSize);

	theme->font = juce::Font(theme->typeface).withHeight(16.0f);
}

CustomLookAndFeel::~CustomLookAndFeel() {
	if (theme != nullptr) {
		CustomLookAndFeel::Destruct();
	}
}

juce::Typeface::Ptr CustomLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    // Return the custom font instead of the default one
    return theme->typeface;
}