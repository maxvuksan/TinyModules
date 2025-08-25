#pragma once
#include <JuceHeader.h>

class KnobModulationWheel : public juce::Component
{
public:
    KnobModulationWheel();

    // value in [-1.0, 1.0]  (maps to angle in [-pi, +pi])
    void setValue(float v, juce::NotificationType nt = juce::sendNotification);
    float getValue() const noexcept { return value; }

    // a simple callback if you want to react
    std::function<void(float)> onChange;

    // painting
    void paint(juce::Graphics& g) override;

    // interaction (rotary vertical drag style)
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

private:

    int wireColourIndex = 0;

    float value = 0.0f;                  // [-1..1]
    float startDragValue = 0.0f;

    juce::Point<int> dragStartPos{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobModulationWheel)
};
