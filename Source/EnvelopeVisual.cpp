#include "EnvelopeVisual.h"
#include "CustomLookAndFeel.h"

EnvelopeVisual::EnvelopeVisual()
{
    rebuildPath();
}

void EnvelopeVisual::resized()
{
    rebuildPath();
}

void EnvelopeVisual::paint(juce::Graphics& g)
{
    const auto* theme = CustomLookAndFeel::GetTheme();

    auto panel = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(theme->colour_displayPanel);
    g.fillRect(panel);

    // Draw the ADSR envelope as a stroked path
    g.setColour(theme->colour_switchOn);
    g.strokePath(envelopePath, juce::PathStrokeType(2.0f));
}

/*
    Sets the controls for the envelope; the path will be recalculated.
    attack/decay/release are non-negative durations (relative widths on the X axis).
    sustain is a level in [0..1].
*/
void EnvelopeVisual::SetControls(float attack, float sustain, float decay, float release)
{
    // Clamp and normalise inputs
    const auto a = juce::jmax(0.0f, attack);
    const auto d = juce::jmax(0.0f, decay);
    const auto r = juce::jmax(0.0f, release);
    const auto s = juce::jlimit(0.0f, 1.0f, sustain);

    // Early-out if nothing changed
    if (a == attackTime && d == decayTime && r == releaseTime && s == sustainLevel)
        return;

    attackTime = a;
    decayTime = d;
    releaseTime = r;
    sustainLevel = s;

    rebuildPath();
    repaint();
}

void EnvelopeVisual::rebuildPath()
{
    envelopePath.clear();

    // Match this to the stroke you use in paint()
    const float strokePx = 2.0f;
    const float aa = 0.5f; // tiny cushion for antialiasing

    // Area you fill/clip in paint()
    auto panel = getLocalBounds().toFloat().reduced(1.0f);

    // Inset path by half the stroke so it renders fully inside the clip
    auto r = panel.reduced(strokePx * 0.5f + aa);
    if (r.getWidth() <= 1.0f || r.getHeight() <= 1.0f)
        return;

    const float width = r.getWidth();
    const float x0 = r.getX();

    // --- NEW: draw only in the middle 70% vertically (15% pad top & bottom)
    const float drawScale = 0.70f;
    const float fullH = r.getHeight();
    const float drawH = fullH * drawScale;
    const float yPad = (fullH - drawH) * 0.5f;

    const float yBottomDraw = r.getBottom() - yPad; // bottom of drawable band

    auto levelToY = [yBottomDraw, drawH](float level) -> float
        {
            // level: 0 = bottom of band, 1 = top of band
            level = juce::jlimit(0.0f, 1.0f, level);
            return yBottomDraw - level * drawH;
        };
    // --- end NEW

    // Map A/D/R durations to proportional widths; sustain gets the leftover width.
    const float sumADR = attackTime + decayTime + releaseTime;

    float wA = 0.0f, wD = 0.0f, wR = 0.0f, wS = 0.0f;

    if (sumADR > 0.0f)
    {
        wA = width * (attackTime / sumADR);
        wD = width * (decayTime / sumADR);
        wR = width * (releaseTime / sumADR);
        wS = juce::jmax(0.0f, width - (wA + wD + wR)); // whatever space is left
    }
    else
    {
        // If all are zero, show only a sustain plateau across the whole width
        wS = width;
    }

    // Build the path: start at 0 level, attack to 1, decay to sustain, sustain plateau, release to 0.
    float x = x0;

    // Start point at 0 level
    envelopePath.startNewSubPath(x, levelToY(0.0f));

    // Attack
    if (wA > 0.0f)
    {
        x += wA;
        envelopePath.lineTo(x, levelToY(1.0f));
    }

    // Decay
    if (wD > 0.0f)
    {
        x += wD;
        envelopePath.lineTo(x, levelToY(sustainLevel));
    }
    else if (wA > 0.0f || sumADR == 0.0f)
    {
        envelopePath.lineTo(x, levelToY(sustainLevel));
    }

    // Sustain plateau
    if (wS > 0.0f)
    {
        x += wS;
        envelopePath.lineTo(x, levelToY(sustainLevel));
    }

    // Release
    if (wR > 0.0f)
    {
        x += wR;
        envelopePath.lineTo(x, levelToY(0.0f));
    }
    else if (x < x0 + width)
    {
        x = x0 + width;
        envelopePath.lineTo(x, levelToY(sustainLevel));
    }
}
