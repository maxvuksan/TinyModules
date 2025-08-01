#pragma once
#include <JuceHeader.h>



struct CustomTheme {

    juce::Colour colour_backgroundRackOdd = juce::Colour::fromRGB(17, 19, 29);
    juce::Colour colour_backgroundRackEven = juce::Colour::fromRGB(19, 22, 34);
    juce::Colour colour_backgroundRackLines = juce::Colour::fromRGBA(0,0,0,0);

    juce::Colour colour_blockHeader = juce::Colour::fromRGB(114, 112, 143);
    juce::Colour colour_blockHeaderText = juce::Colour::fromRGB(12, 15, 34);

    juce::Colour colour_knobOutline = juce::Colour::fromRGB(41, 47, 82);
    juce::Colour colour_knobFill = juce::Colour::fromRGB(12, 15, 34);
    juce::Colour colour_knobPointer = juce::Colour::fromRGB(93, 109, 150);
    
    juce::Colour colour_blockOutline = juce::Colour::fromRGBA(68, 78, 127, 0);
    juce::Colour colour_blockFill = juce::Colour::fromRGB(33, 35, 60);
    juce::Colour colour_blockFillDark = juce::Colour::fromRGB(55, 53, 89);

    juce::Colour colour_floatingBlock = juce::Colour::fromRGBA(197, 60, 170, 14);

    juce::Colour colour_switchOn  = juce::Colour::fromRGB(255, 180, 100);
    juce::Colour colour_switchOnHighlight = juce::Colour::fromRGB(255, 56, 112);
    juce::Colour colour_switchOutline = juce::Colour::fromRGB(128, 208, 255);

    juce::Colour colour_switchOff = juce::Colour::fromRGB(20 * 3, 14 * 3, 10 * 3);
    juce::Colour colour_switchOffHighlight = juce::Colour::fromRGB(33 * 3, 4 * 3, 23 * 3);

    juce::Colour colour_displayPanel = juce::Colour::fromRGB(12, 15, 34);

    juce::Typeface::Ptr typeface;   
    juce::Font font;    // applied from typeface

    float borderRadius = 1.0f;
};




class CustomLookAndFeel : public juce::LookAndFeel_V4
{
    public:

        static CustomTheme* GetTheme();

        CustomLookAndFeel();
        ~CustomLookAndFeel();
        juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;

    private:

        // should be called after JUCE has been setup
        static void Init();
        // should be called before JUCE has fully shutdown
        static void Destruct();

        static CustomTheme* theme;
};
