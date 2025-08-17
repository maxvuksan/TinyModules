#include "Globals.h"
#include "PianoSwitch.h"
#include "CustomLookAndFeel.h"
#include "DSPUtility.h"

PianoSwitch::PianoSwitch() {

    for (int i = 0; i < 7; ++i) whiteKeys[i] = false;
    for (int i = 0; i < 5; ++i) blackKeys[i] = false;

}

juce::var PianoSwitch::Serialize()
{
    juce::DynamicObject* obj = new juce::DynamicObject();

    juce::Array<juce::var> whiteArray;
    for (int i = 0; i < 7; ++i){
        whiteArray.add(whiteKeys[i]);
    }

    juce::Array<juce::var> blackArray;
    for (int i = 0; i < 5; ++i) {
        blackArray.add(blackKeys[i]);
    }

    obj->setProperty("whiteKeys", whiteArray);
    obj->setProperty("blackKeys", blackArray);

    return juce::var(obj);
}

void PianoSwitch::Deserialize(const juce::var& data)
{
    if (!data.isObject()) return;
    const juce::DynamicObject* obj = data.getDynamicObject();
    if (obj == nullptr) return;

    const juce::var whiteVar = obj->getProperty("whiteKeys");
    const juce::var blackVar = obj->getProperty("blackKeys");

    if (whiteVar.isArray())
    {
        auto whiteArray = *whiteVar.getArray();
        for (int i = 0; i < juce::jmin(7, whiteArray.size()); ++i) {
            whiteKeys[i] = (bool)whiteArray[i];
        }
    }

    if (blackVar.isArray())
    {
        auto blackArray = *blackVar.getArray();
        for (int i = 0; i < juce::jmin(5, blackArray.size()); ++i) {
            blackKeys[i] = (bool)blackArray[i];
        }
    }

    ComputeKeyStates();
    repaint();
}


void PianoSwitch::paint(juce::Graphics& g)
{
    const int whiteKeyHeight = GLOBAL_COMPONENT_HEIGHT / 2;
    const int whiteKeyWidth = GLOBAL_COMPONENT_WIDTH * 2 - 8;
    const int spacing = 2;

    const int startX = 4;
    const int startY = 8;

    const int blackKeyHeight = whiteKeyHeight * 0.9f;
    const int blackKeyWidth = whiteKeyWidth * 0.65f;

    const auto* theme = CustomLookAndFeel::GetTheme();

    g.setColour(theme->colour_displayPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1), theme->borderRadius);


    // Draw 7 vertical white keys (C D E F G A B)
    for (int i = 0; i < 7; ++i)
    {
        float y = startY + i * (whiteKeyHeight + spacing);
        juce::Rectangle<float> rect(
            startX + 3, y,
            whiteKeyWidth - 6,
            whiteKeyHeight
        );

        whiteKeyRects[i] = rect; // <-- Store hitbox

        if (highlightedKeyIndex == i && highlightedIsWhiteKey) {

            g.setColour(whiteKeys[i]
                ? theme->colour_switchOnHighlight
                : theme->colour_switchOffHighlight);
        }
        else {
            g.setColour(whiteKeys[i]
                ? theme->colour_switchOn
                : theme->colour_switchOff);
        }

        g.fillRect(rect.reduced(1.0));
    }

    const int blackKeyMap[5] = { 0, 1, 3, 4, 5 };

    for (int i = 0; i < 5; ++i)
    {
        int whiteIndex = blackKeyMap[i];
        float y = startY + whiteIndex * (whiteKeyHeight + spacing) + (whiteKeyHeight - blackKeyHeight) / 2;

        juce::Rectangle<float> rect(
            startX + 30, y + (whiteKeyHeight / 2),
            blackKeyWidth, blackKeyHeight
        );

        blackKeyRects[i] = rect; // <-- Store hitbox

        if (highlightedKeyIndex == i && !highlightedIsWhiteKey) {

            g.setColour(blackKeys[i]
                ? theme->colour_switchOnHighlight
                : theme->colour_switchOffHighlight);
        }
        else {
            g.setColour(blackKeys[i]
                ? theme->colour_switchOn
                : theme->colour_switchOff);
        }

        g.fillRect(rect.reduced(1.0f));

        g.setColour(theme->colour_displayPanel);
        g.drawRect(rect, 3.0);
    }
}

void PianoSwitch::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.position;

    // Check black keys first (on top)
    for (int i = 0; i < 5; ++i)
    {
        if (blackKeyRects[i].contains(pos))
        {
            blackKeys[i] = !blackKeys[i];
            repaint();
            ComputeKeyStates();
            return;
        }
    }

    // Check white keys next
    for (int i = 0; i < 7; ++i)
    {
        if (whiteKeyRects[i].contains(pos))
        {
            whiteKeys[i] = !whiteKeys[i];
            repaint();
            ComputeKeyStates();
            return;
        }
    }
}

bool PianoSwitch::GetKeyState(int index) {
    return keyStates[index];
}

void PianoSwitch::SetHighlightedKey(int index) {
    if (index < 0 || index > 11) {
        highlightedKeyIndex = -1;
        repaint();
        return;
    }

    // { isWhite, arrayIndex }
    static const std::pair<bool, int> keyMap[12] = {
        { true,  0 }, // C
        { false, 0 }, // C#
        { true,  1 }, // D
        { false, 1 }, // D#
        { true,  2 }, // E
        { true,  3 }, // F
        { false, 2 }, // F#
        { true,  4 }, // G
        { false, 3 }, // G#
        { true,  5 }, // A
        { false, 4 }, // A#
        { true,  6 }  // B
    };

    // state is already active
    if (highlightedKeyIndex == keyMap[index].second && highlightedIsWhiteKey == keyMap[index].first) {
        return;
    }

    highlightedKeyIndex = index;
    highlightedIsWhiteKey = keyMap[index].first;
    highlightedKeyIndex = keyMap[index].second;

    repaint();
}

int PianoSwitch::WriteVoltagesToPolyphonicBuffer(juce::AudioBuffer<float>& writeBuffer, int numberOfSamples, float vOctShift) {

    cachedVoltages.clear();

    for (int i = 0; i < 12; i++) {

        if (keyStates[i]) {

            cachedVoltages.push_back(DSP::NOTE_VOLTAGES[i]);
        }
    }

    for (int vol = 0; vol < cachedVoltages.size(); vol++) {
     
        // write voltages to buffer, only write the number of notes pressed, leave rest
        for (int i = 0; i < numberOfSamples; i++) {
            writeBuffer.getWritePointer(vol)[i] = cachedVoltages[vol] + vOctShift;
        }
    }

    // return the number of channels we set, this should be used to mark the cable with this many active channels
    return cachedVoltages.size();
}


void PianoSwitch::ComputeKeyStates() {
    
    keyStates[0] = whiteKeys[0]; // C
    keyStates[1] = blackKeys[0]; // C#
    keyStates[2] = whiteKeys[1]; // D
    keyStates[3] = blackKeys[1]; // D#
    keyStates[4] = whiteKeys[2]; // E
    keyStates[5] = whiteKeys[3]; // F
    keyStates[6] = blackKeys[2]; // F#
    keyStates[7] = whiteKeys[4]; // G
    keyStates[8] = blackKeys[3]; // G#
    keyStates[9] = whiteKeys[5]; // A
    keyStates[10] = blackKeys[4]; // A#
    keyStates[11] = whiteKeys[6]; // B
}