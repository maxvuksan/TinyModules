#pragma once
#include <JuceHeader.h>



struct CustomTheme {

    juce::Colour colour_headerBar = juce::Colour::fromRGB(28, 29, 41);
    juce::Colour colour_headerBarText = juce::Colour::fromRGB(255, 255, 255);

    juce::Colour colour_backgroundRackOdd = juce::Colour::fromRGB(19 * 0.7, 22 * 0.7, 33 * 0.7);
    juce::Colour colour_backgroundRackEven = juce::Colour::fromRGB(19 * 0.8, 22 * 0.8, 33 * 0.8);
    juce::Colour colour_backgroundRackLines = juce::Colour::fromRGBA(0,0,0,0);

    juce::Colour colour_blockHeader = juce::Colour::fromRGB(114, 112, 143);
    juce::Colour colour_blockHeaderText = juce::Colour::fromRGB(12, 15, 34);

    juce::Colour colour_knobOutline = juce::Colour::fromRGB(41, 47, 53);
    juce::Colour colour_knobFill = juce::Colour::fromRGB(18, 20, 30);
    juce::Colour colour_knobPointer = juce::Colour::fromRGB(125, 120, 140);
    
    juce::Colour colour_blockOutline = juce::Colour::fromRGBA(68, 78, 127, 0);
    juce::Colour colour_blockFill = juce::Colour::fromRGB(28, 29, 41);
    juce::Colour colour_blockFillDark = juce::Colour::fromRGB(55, 53, 89);
    juce::Colour colour_blockFillLight = juce::Colour::fromRGB(28 + 15, 29 + 15, 41 + 13);

    juce::Colour colour_floatingBlock = juce::Colour::fromRGB(33, 35, 60);

    juce::Colour colour_switchOn  = juce::Colour::fromRGB(255, 180, 100);
    juce::Colour colour_switchOnHighlight = juce::Colour::fromRGB(255, 56, 112);
    juce::Colour colour_switchOutline = juce::Colour::fromRGB(128, 208, 255);

    juce::Colour colour_switchOff = juce::Colour::fromRGB(20 * 3, 14 * 3, 10 * 3);
    juce::Colour colour_switchOffHighlight = juce::Colour::fromRGB(33 * 3, 4 * 3, 23 * 3);

    juce::Colour colour_displayPanel = juce::Colour::fromRGB(18, 20, 30);

    juce::Colour colour_selectionOutline = juce::Colour::fromRGB(255, 180, 100);
    juce::Colour colour_selectionFill = juce::Colour::fromRGBA(255, 180, 100, 30);

    juce::Typeface::Ptr typeface;   
    juce::Font font;    // applied from typeface


    // NOTE: the size of the wire colour array should be the same between themes.
    const std::vector<juce::Colour> colour_wires = {
        juce::Colour::fromRGB(255, 56, 112),
        juce::Colour::fromRGB(56, 156, 255),
        juce::Colour::fromRGB(240, 152, 53),
        juce::Colour::fromRGB(132, 92, 242)
    };

    const std::vector<juce::Colour> colour_wiresOff = {
        juce::Colour::fromRGB(255 / 3, 56 / 3, 112 / 3),
        juce::Colour::fromRGB(56 / 3, 156 / 3, 255 / 3),
        juce::Colour::fromRGB(240 / 3, 152 / 3, 53 / 3),
        juce::Colour::fromRGB(132 / 3, 92 / 3, 242 / 3)
    };


    float borderRadius = 1.0f;
};




class CustomLookAndFeel : public juce::LookAndFeel_V4
{
    public:

        static CustomTheme* GetTheme();

        CustomLookAndFeel();
        ~CustomLookAndFeel();
        juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;




        // Style overrides ______________________________________________________________

        void drawButtonBackground(juce::Graphics& g,
            juce::Button& button,
            const juce::Colour& backgroundColour,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

        void drawButtonText(juce::Graphics& g,
            juce::TextButton& button,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

        int getPopupMenuBorderSize() override;

        juce::Font getPopupMenuFont() override;

        void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
            int standardHeight, int& idealWidth, int& idealHeight) override;

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
            bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
            const juce::String& shortcutKeyText, const juce::Drawable* icon,
            const juce::Colour* textColour) override;

        void fillTextEditorBackground(juce::Graphics&, int w, int h, juce::TextEditor&) override;

        void drawTextEditorOutline(juce::Graphics&, int w, int h, juce::TextEditor&) override;


    private:

        // should be called after JUCE has been setup
        static void Init();
        // should be called before JUCE has fully shutdown
        static void Destruct();

        void ApplyCustomTheme();

        static CustomLookAndFeel instance;
        static CustomTheme* theme;
};
