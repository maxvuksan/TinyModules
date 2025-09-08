#include "EnvelopeVisual.h"
#include "CustomLookAndFeel.h"

EnvelopeVisual::EnvelopeVisual()
{

}

void EnvelopeVisual::resized()
{
    if (isEnvelope) {
        RebuildEnvelopePath();
    }
    else {
        RebuildFilterPath();
    }
}

void EnvelopeVisual::paint(juce::Graphics& g)
{
    const auto* theme = CustomLookAndFeel::GetTheme();

    auto panel = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(theme->colour_displayPanel);
    g.fillRect(panel);

    // Draw the ADSR envelope as a stroked path
    g.setColour(theme->colour_switchOn);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

/*
    Sets the controls for the envelope; the path will be recalculated.
    attack/decay/release are non-negative durations (relative widths on the X axis).
    sustain is a level in [0..1].
*/
void EnvelopeVisual::SetEnvelopeControls(float attack, float sustain, float decay, float release)
{
    isEnvelope = true;

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

    RebuildEnvelopePath();
    repaint();
}

void EnvelopeVisual::RebuildEnvelopePath()
{
    path.clear();

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
    path.startNewSubPath(x, levelToY(0.0f));

    // Attack
    if (wA > 0.0f)
    {
        x += wA;
        path.lineTo(x, levelToY(1.0f));
    }

    // Decay
    if (wD > 0.0f)
    {
        x += wD;
        path.lineTo(x, levelToY(sustainLevel));
    }
    else if (wA > 0.0f || sumADR == 0.0f)
    {
        path.lineTo(x, levelToY(sustainLevel));
    }

    // Sustain plateau
    if (wS > 0.0f)
    {
        x += wS;
        path.lineTo(x, levelToY(sustainLevel));
    }

    // Release
    if (wR > 0.0f)
    {
        x += wR;
        path.lineTo(x, levelToY(0.0f));
    }
    else if (x < x0 + width)
    {
        x = x0 + width;
        path.lineTo(x, levelToY(sustainLevel));
    }
}

void EnvelopeVisual::SetFilterControls(float cutoffFrequency, float resonanceLevel, bool cutLowIn)
{

    isEnvelope = false;

    // Clamp/clean inputs
    const float minF = 20.0f;
    const float maxF = 20000.0f;

    const float newCutoff = juce::jlimit(minF, maxF, cutoffFrequency);
    const float newRes = juce::jlimit(0.0f, 1.0f, resonanceLevel);

    if (newCutoff == cutoffHz && newRes == resonance01 && cutLowIn == cutLow)
        return;

    cutoffHz = newCutoff;
    resonance01 = newRes;
    cutLow = cutLowIn;

    RebuildFilterPath();
    repaint();
}

void EnvelopeVisual::RebuildFilterPath()
{
    path.clear();

    const float strokePx = 2.0f, aa = 0.5f;
    auto panel = getLocalBounds().toFloat().reduced(1.0f);
    auto r = panel.reduced(strokePx * 0.5f + aa);
    if (r.getWidth() <= 1.0f || r.getHeight() <= 1.0f) return;

    // Vertical drawing band (same as your envelope)
    const float drawScale = 0.70f;
    const float fullH = r.getHeight();
    const float drawH = fullH * drawScale;
    const float yPad = (fullH - drawH) * 0.5f;

    const float x0 = r.getX();
    const float yBottomDraw = r.getBottom() - yPad;
    const float width = r.getWidth();

    // --- X axis: LOG frequency (unchanged) ---
    const float fMin = 20.0f, fMax = 20000.0f;
    const float logSpan = std::log(fMax / fMin);
    auto xToFreq = [fMin, logSpan, width, x0](float x)
        {
            const float t = juce::jlimit(0.0f, 1.0f, (x - x0) / juce::jmax(1.0f, width));
            return fMin * std::exp(logSpan * t);
        };
    auto freqToX = [fMin, fMax, width, x0](float f)
        {
            f = juce::jlimit(fMin, fMax, f);
            const float t = std::log(f / fMin) / std::log(fMax / fMin);
            return x0 + t * width;
        };

    // --- Y axis: dB (0 dB at top, minDb at bottom) ---
    const float minDb = -48.0f;   // show down to -48 dB
    const float maxDb = 0.0f;     // 0 dB passband
    auto dbToY = [yBottomDraw, drawH, minDb, maxDb](float dB)
        {
            dB = juce::jlimit(minDb, maxDb, dB);
            const float t = (dB - minDb) / (maxDb - minDb); // 0..1 bottom->top
            return yBottomDraw - t * drawH;
        };

    // --- Toy magnitude model (same as before but we’ll convert to dB) ---
    const float fc = cutoffHz;
    const float slope = 4.0f; // visual steepness, 1gentle, 6steeper

    auto baseMagnitude = [this, fc, slope](float f)
        {
            const float r = f / juce::jmax(1.0f, fc);
            const float rn = std::pow(r, slope);
            if (cutLow)   // high-pass
                return rn / std::sqrt(1.0f + rn * rn);      // 0..1
            else          // low-pass
                return 1.0f / std::sqrt(1.0f + rn * rn);    // 0..1
        };

    // Resonance bump in log-frequency domain
    auto resonanceBump = [this, fc](float f)
        {
            if (fc <= 0.0f) return 0.0f;
            const float sigma = 0.15f; // width in log10 decades
            const float d = std::log10(juce::jmax(1e-6f, f) / fc);
            return std::exp(-0.5f * (d * d) / (sigma * sigma)); // 0..1
        };

    // Build path
    const int steps = juce::jlimit(64, 1024, (int)std::round(width));
    const float dx = width / juce::jmax(1, steps - 1);

    float x = x0;
    for (int i = 0; i < steps; ++i, x += dx)
    {
        const float f = xToFreq(x);

        float mag = baseMagnitude(f);
        // Add resonance as blend toward 1.0
        mag = juce::jlimit(0.0f, 1.2f, mag + (1.0f - mag) * (resonanceBump(f) * resonance01));

        // Convert linear magnitude -> dB; guard against log(0)
        const float magClamped = juce::jmax(1.0e-6f, mag);
        const float dB = 20.0f * std::log10(magClamped);

        const float y = dbToY(dB);

        if (i == 0) path.startNewSubPath(x, y);
        else        path.lineTo(x, y);
    }

    // Optional: cutoff marker (vertical line)
    // const float xCut = freqToX(fc);
    // path.startNewSubPath(xCut, dbToY(minDb));
    // path.lineTo(xCut, dbToY(maxDb));
}
