#include "CustomLookAndFeel.h"

CustomTheme* CustomLookAndFeel::theme;
CustomLookAndFeel CustomLookAndFeel::instance;

void CustomLookAndFeel::Init() {

	theme = new CustomTheme;

    instance.ApplyCustomTheme();
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


void CustomLookAndFeel::ApplyCustomTheme() {

    auto* th = GetTheme();
}





void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = GetTheme()->borderRadius;

    // Base color
    juce::Colour baseColour = GetTheme()->colour_blockFill;

    if (shouldDrawButtonAsDown)
        baseColour = GetTheme()->colour_blockFillDark;
    else if (shouldDrawButtonAsHighlighted)
        baseColour = GetTheme()->colour_selectionFill;

    // Fill only, no border
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);
}

void CustomLookAndFeel::drawButtonText(juce::Graphics& g,
    juce::TextButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    // Use the custom theme font
    auto font = GetTheme()->font;
    g.setFont(font);

    juce::Colour textColour = GetTheme()->colour_headerBarText;

    if (!button.isEnabled())
        textColour = textColour.withMultipliedAlpha(0.5f);

    g.setColour(textColour);
    g.drawFittedText(button.getButtonText(),
        button.getLocalBounds(),
        juce::Justification::centred,
        1);
}


int CustomLookAndFeel::getPopupMenuBorderSize() { return 0; }

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    if (auto* th = GetTheme(); th && th->font.getTypefacePtr() != nullptr)
        return th->font.withHeight(16.0f);
    return juce::Font(16.0f);
}

void CustomLookAndFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
    int /*standardHeight*/, int& idealWidth, int& idealHeight)
{
    if (isSeparator) { idealWidth = 0; idealHeight = 8; return; }
    auto f = getPopupMenuFont();
    idealHeight = juce::roundToInt(f.getHeight() + 10.0f);
    idealWidth = juce::roundToInt(f.getStringWidthFloat(text) + 48.0f);
}

void CustomLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto r = juce::Rectangle<float>(0, 0, (float)width, (float)height);

    g.setColour(theme->colour_backgroundRackEven);
    g.fillRoundedRectangle(r, theme->borderRadius);

    g.setColour(theme->colour_knobPointer);
    g.drawRect(r, theme->borderRadius);
}

void CustomLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
    bool hasSubMenu, const juce::String& text,
    const juce::String& shortcutKeyText, const juce::Drawable* icon,
    const juce::Colour* textColour)
{
    auto r = area.reduced(1, 1);

    if (isSeparator)
    {
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.fillRect(r.withHeight(1).withY(area.getCentreY()));
        return;
    }

    if (isHighlighted)
    {
        g.setColour(theme->colour_blockFillLight);
        g.fillRect(r.toFloat());
    }

    // Left column for tick/icon
    auto left = r.removeFromLeft(4);
    if (isTicked)
    {
        g.setColour(textColour ? *textColour : findColour(juce::PopupMenu::highlightedTextColourId));
        g.fillEllipse(left.toFloat().reduced(6));
    }
    else if (icon != nullptr)
    {
        icon->drawWithin(g, left.toFloat().reduced(2), juce::RectanglePlacement::centred, isActive ? 1.0f : 0.4f);
    }

    // Label
    g.setColour(textColour ? *textColour
        : (isHighlighted ? theme->colour_headerBarText
            : theme->colour_knobPointer));
    g.setFont(getPopupMenuFont());
    auto labelArea = r;
    if (shortcutKeyText.isNotEmpty() || hasSubMenu)
        labelArea = r.removeFromLeft(juce::jmax(r.getWidth() - 60, 0));
    g.drawFittedText(text, labelArea, juce::Justification::centredLeft, 1);

    // Shortcut or chevron
    if (hasSubMenu)
    {
        auto c = r.removeFromRight(16).toFloat();
        juce::Path p;
        float s = 6.0f;
        p.startNewSubPath(c.getCentreX() - s * 0.4f, c.getCentreY() - s * 0.6f);
        p.lineTo(c.getCentreX() + s * 0.2f, c.getCentreY());
        p.lineTo(c.getCentreX() - s * 0.4f, c.getCentreY() + s * 0.6f);
        g.fillPath(p);
    }
    else if (shortcutKeyText.isNotEmpty())
    {
        //g.setColour(g.getCurrentColour().withAlpha(0.7f));
        g.drawFittedText(shortcutKeyText, r, juce::Justification::centredRight, 1);
    }
}


void CustomLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor&)
{
    auto r = juce::Rectangle<float>(0, 0, (float)w, (float)h);

    g.setColour(theme->colour_backgroundRackOdd);
    g.fillRoundedRectangle(r, GetTheme()->borderRadius);
}

void CustomLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& te)
{
    auto r = juce::Rectangle<float>(0.5f, 0.5f, (float)w - 1.0f, (float)h - 1.0f);

    auto outline = te.hasKeyboardFocus(true)
        ? theme->colour_headerBarText
        : theme->colour_knobPointer;

    g.setColour(outline);
    g.drawRoundedRectangle(r, GetTheme()->borderRadius, 1.0f);
}