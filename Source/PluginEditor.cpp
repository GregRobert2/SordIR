#include "PluginEditor.h"
using namespace juce;

static const int PLUGIN_W = 1060;
static const int PLUGIN_H = 980;   // shorter now — no EQ knob row
static const int MIN_W    = 820;
static const int MIN_H    = 820;

// ═══════════════════════════════════════════════════════════════════════════
//  FolderRow
// ═══════════════════════════════════════════════════════════════════════════
void FolderRow::paint(Graphics& g)
{
    const float w = (float)getWidth(), h = (float)getHeight();
    if (isSelected) {
        g.setColour(SordLookAndFeel::gold.withAlpha(0.12f));
        g.fillRect(0.f, 0.f, w, h);
        g.setColour(SordLookAndFeel::gold);
        g.fillRect(0.f, 0.f, 3.f, h);
    } else if (isMouseOver()) {
        g.setColour(Colours::white.withAlpha(0.04f));
        g.fillRect(0.f, 0.f, w, h);
    }
    if (!icon.isEmpty()) {
        const float boxSize = 28.f;
        auto ib = icon.getBounds();
        if (ib.getWidth() > 0 && ib.getHeight() > 0) {
            float scale = boxSize / jmax(ib.getWidth(), ib.getHeight());
            AffineTransform t = AffineTransform::scale(scale)
                .translated(10.f - ib.getX() * scale,
                             h * 0.5f - (ib.getY() + ib.getHeight() * 0.5f) * scale);
            g.setColour(isSelected
                ? SordLookAndFeel::gold.withAlpha(0.9f)
                : SordLookAndFeel::textFaint.brighter(0.7f));
            g.fillPath(icon, t);
        }
    }
    g.setFont(Font(FontOptions().withName("Barlow")
                                .withHeight(12.f)
                                .withStyle(isSelected ? "SemiBold" : "Regular")));
    g.setColour(isSelected ? SordLookAndFeel::gold : SordLookAndFeel::textMain);
    g.drawText(name, 46, 0, (int)w - 58, (int)h, Justification::centredLeft);
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(9.f)));
    g.setColour(isSelected ? SordLookAndFeel::gold.withAlpha(0.7f)
                           : SordLookAndFeel::textFaint.brighter(0.4f));
    g.drawText(String(presetCount), (int)w - 22, 0, 18, (int)h, Justification::centredRight);
    g.setColour(SordLookAndFeel::border);
    g.drawLine(6.f, h - 0.5f, w - 6.f, h - 0.5f, 0.5f);
}

// ═══════════════════════════════════════════════════════════════════════════
//  PresetRow
// ═══════════════════════════════════════════════════════════════════════════
void PresetRow::paint(Graphics& g)
{
    const float w = (float)getWidth(), h = (float)getHeight();
    if (isActive) {
        g.setColour(SordLookAndFeel::gold.withAlpha(0.10f));
        g.fillRect(0.f, 0.f, w, h);
        g.setColour(SordLookAndFeel::gold);
        g.fillRect(0.f, 0.f, 2.f, h);
        Path arr;
        float ay = h * 0.5f;
        arr.addTriangle(w-14.f, ay-4.f, w-14.f, ay+4.f, w-8.f, ay);
        g.setColour(SordLookAndFeel::gold.withAlpha(0.6f));
        g.fillPath(arr);
    } else if (isMouseOver()) {
        g.setColour(Colours::white.withAlpha(0.03f));
        g.fillRect(0.f, 0.f, w, h);
    }
    g.setFont(Font(FontOptions().withName("Barlow").withHeight(12.f)));
    g.setColour(isActive ? SordLookAndFeel::gold.brighter(0.2f) : SordLookAndFeel::textMain);
    g.drawText(name, 14, 0, (int)w-28, (int)h, Justification::centredLeft);
    g.setColour(SordLookAndFeel::border);
    g.drawLine(8.f, h-0.5f, w-4.f, h-0.5f, 0.5f);
}

// ═══════════════════════════════════════════════════════════════════════════
//  UserPresetRow
// ═══════════════════════════════════════════════════════════════════════════
void UserPresetRow::paint(Graphics& g)
{
    const float w=(float)getWidth(), h=(float)getHeight();
    if (isActive) {
        g.setColour(SordLookAndFeel::gold.withAlpha(0.09f)); g.fillRect(0.f,0.f,w,h);
        g.setColour(SordLookAndFeel::gold); g.fillRect(0.f,0.f,2.f,h);
    } else if (isMouseOver()) {
        g.setColour(Colours::white.withAlpha(0.03f)); g.fillRect(0.f,0.f,w,h);
    }
    g.setFont(Font(FontOptions().withName("Barlow").withHeight(12.f)));
    g.setColour(isActive ? SordLookAndFeel::gold.brighter(0.15f) : SordLookAndFeel::textMain);
    g.drawText(presetName, 12, 0, (int)w-40, (int)h, Justification::centredLeft);
}
void UserPresetRow::resized()
{
    if (getNumChildComponents() == 0) {
        addAndMakeVisible(deleteBtn);
        deleteBtn.setColour(TextButton::buttonColourId, Colours::transparentBlack);
        deleteBtn.setColour(TextButton::textColourOffId, SordLookAndFeel::textFaint.brighter(0.3f));
        deleteBtn.onClick = [this]{ if (onDelete) onDelete(); };
    }
    deleteBtn.setBounds(getWidth()-28, 3, 22, getHeight()-6);
}

// ═══════════════════════════════════════════════════════════════════════════
//  EQDisplay
// ═══════════════════════════════════════════════════════════════════════════
EQDisplay::EQDisplay(AudioProcessorValueTreeState& a) : apvts(a)
{
    syncFromAPVTS();
    setInterceptsMouseClicks(true, false);
    startTimerHz(60);   // 60 Hz for smooth Q animation
}
EQDisplay::~EQDisplay() { stopTimer(); }

void EQDisplay::timerCallback()
{
    bool changed = false;
    for (int i = 0; i < 4; ++i) {
        float target = getBandQ(i);
        float diff   = target - bands[i].qVisual;
        if (std::abs(diff) > 0.001f) {
            bands[i].qVisual += diff * 0.18f;  // smooth towards target
            changed = true;
        }
    }
    if (changed) repaint();
}

void EQDisplay::syncFromAPVTS()
{
    const String fID[] = {"eq0Freq","eq1Freq","eq2Freq","eq3Freq"};
    const String gID[] = {"eq0Gain","eq1Gain","eq2Gain","eq3Gain"};
    for (int i = 0; i < 4; ++i) {
        bands[i].normX  = freqToNormX(i, *apvts.getRawParameterValue(fID[i]));
        bands[i].normY  = gainToNormY(*apvts.getRawParameterValue(gID[i]));
        bands[i].q      = getBandQ(i);
        bands[i].qVisual= bands[i].q;
    }
}

float EQDisplay::freqToNormX(int b, float f) const
{ return std::log10(f/freqMin[b]) / std::log10(freqMax[b]/freqMin[b]); }
float EQDisplay::normXToFreq(int b, float nx) const
{ return freqMin[b] * std::pow(freqMax[b]/freqMin[b], jlimit(0.f,1.f,nx)); }
float EQDisplay::gainToNormY(float g) const  { return 0.5f - g / (GAIN_RANGE * 2.f); }
float EQDisplay::normYToGain(float ny) const { return (0.5f - ny) * (GAIN_RANGE * 2.f); }

Point<float> EQDisplay::bandToPixel(int b) const
{
    const float w = (float)getWidth(), h = (float)getHeight();
    float freq = normXToFreq(b, bands[b].normX);
    float normLog = std::log10(freq / 20.f) / std::log10(20000.f / 20.f);
    float px = normLog * w;
    float gain = normYToGain(bands[b].normY);
    float py   = h * 0.5f - (gain / GAIN_RANGE) * h * 0.5f;
    return { jlimit(0.f, w, px), jlimit(0.f, h, py) };
}

int EQDisplay::getBandAtPoint(Point<float> pos) const
{
    for (int i = 0; i < 4; ++i)
        if (pos.getDistanceFrom(bandToPixel(i)) < 14.f) return i;
    return -1;
}

float EQDisplay::getBandQ(int band) const
{
    const String qID[] = {"eq0Q","eq1Q","eq2Q","eq3Q"};
    return *apvts.getRawParameterValue(qID[band]);
}

void EQDisplay::setBandQ(int band, float q)
{
    const String qID[] = {"eq0Q","eq1Q","eq2Q","eq3Q"};
    q = jlimit(0.1f, 8.f, q);
    if (auto* p = apvts.getParameter(qID[band]))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(q));
    bands[band].q = q;
    // qVisual will smooth towards q in timerCallback
}

void EQDisplay::pushToAPVTS(int band)
{
    const String fID[] = {"eq0Freq","eq1Freq","eq2Freq","eq3Freq"};
    const String gID[] = {"eq0Gain","eq1Gain","eq2Gain","eq3Gain"};
    auto setP = [&](const String& id, float v){
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(v));
    };
    setP(fID[band], normXToFreq(band, bands[band].normX));
    setP(gID[band], normYToGain(bands[band].normY));
}

void EQDisplay::mouseDown(const MouseEvent& e)
{
    int b = getBandAtPoint(e.position);
    if (b >= 0) {
        if (e.mods.isCommandDown() || e.mods.isCtrlDown()) {
            bands[b].normX = 0.5f; bands[b].normY = 0.5f;
            pushToAPVTS(b); setBandQ(b, 0.707f); repaint();
        } else {
            dragging = b;
        }
    }
}
void EQDisplay::mouseDrag(const MouseEvent& e)
{
    if (dragging < 0) return;
    const float w = (float)getWidth(), h = (float)getHeight();
    float norm = jlimit(0.f, 1.f, e.position.x / w);
    float hz   = 20.f * std::pow(1000.f, norm);
    hz = jlimit(freqMin[dragging], freqMax[dragging], hz);
    bands[dragging].normX = freqToNormX(dragging, hz);
    bands[dragging].normY = jlimit(0.01f, 0.99f, e.position.y / h);
    pushToAPVTS(dragging);
    repaint();
}
void EQDisplay::mouseUp(const MouseEvent&) { dragging = -1; }
void EQDisplay::mouseMove(const MouseEvent& e)
{
    int prev = hoveredBand;
    hoveredBand = getBandAtPoint(e.position);
    if (hoveredBand != prev) repaint();
}
void EQDisplay::mouseExit(const MouseEvent&) { hoveredBand = -1; repaint(); }
void EQDisplay::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
    int band = getBandAtPoint(e.position);
    if (band < 0) band = hoveredBand;
    if (band < 0) return;
    setBandQ(band, jlimit(0.1f, 8.f, getBandQ(band) + wheel.deltaY * 0.8f));
    repaint();
}
void EQDisplay::mouseDoubleClick(const MouseEvent& e)
{
    int band = getBandAtPoint(e.position);
    if (band >= 0) showFreqEntry(band);
}
void EQDisplay::showFreqEntry(int band)
{
    const String fID[] = {"eq0Freq","eq1Freq","eq2Freq","eq3Freq"};
    float currentFreq = normXToFreq(band, bands[band].normX);
    String currentStr = currentFreq >= 1000.f
        ? String(currentFreq/1000.f, 2) + " kHz"
        : String((int)currentFreq) + " Hz";
    auto* alert = new AlertWindow("Set Frequency",
        "Band " + String(band+1) + " frequency:", MessageBoxIconType::NoIcon);
    alert->addTextEditor("freq", currentStr, "Hz or kHz:");
    alert->addButton("OK", 1, KeyPress(KeyPress::returnKey));
    alert->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));
    alert->enterModalState(true,
        ModalCallbackFunction::create([this, band, fID, alert](int result){
            if (result == 1) {
                String txt = alert->getTextEditorContents("freq").trim().toLowerCase();
                float hz = txt.contains("k")
                    ? txt.retainCharacters("0123456789.").getFloatValue() * 1000.f
                    : txt.retainCharacters("0123456789.").getFloatValue();
                hz = jlimit(freqMin[band], freqMax[band], hz);
                if (hz > 0.f) {
                    bands[band].normX = freqToNormX(band, hz);
                    pushToAPVTS(band); repaint();
                }
            }
            delete alert;
        }), false);
}

// ── Real IIR biquad response ──────────────────────────────────────────────
static float biquadMagDb(float b0,float b1,float b2,float a1,float a2, float w)
{
    float cW=std::cos(w), c2W=std::cos(2.f*w), sW=std::sin(w), s2W=std::sin(2.f*w);
    float nR=b0+b1*cW+b2*c2W, nI=b1*sW+b2*s2W;
    float dR=1.f+a1*cW+a2*c2W, dI=a1*sW+a2*s2W;
    float den2 = dR*dR + dI*dI;
    if (den2 < 1e-18f) return 0.f;
    return 10.f * std::log10(jmax(1e-18f, (nR*nR+nI*nI)/den2));
}

static std::vector<float> computeEQResponse(
    const AudioProcessorValueTreeState& apvts,
    int numPx, float sampleRate = 44100.f)
{
    const char* fIDs[] = {"eq0Freq","eq1Freq","eq2Freq","eq3Freq"};
    const char* gIDs[] = {"eq0Gain","eq1Gain","eq2Gain","eq3Gain"};
    const char* qIDs[] = {"eq0Q",   "eq1Q",   "eq2Q",   "eq3Q"  };
    struct Biquad { float b0,b1,b2,a1,a2; };
    std::array<Biquad,4> filters;
    for (int band = 0; band < 4; ++band) {
        float freq = *apvts.getRawParameterValue(fIDs[band]);
        float gain = *apvts.getRawParameterValue(gIDs[band]);
        float q    = *apvts.getRawParameterValue(qIDs[band]);
        float A    = std::pow(10.f, gain/40.f);
        float w0   = 2.f * MathConstants<float>::pi * freq / sampleRate;
        float cosW = std::cos(w0), sinW = std::sin(w0);
        float alpha = sinW / (2.f * q);
        if (band == 0) {
            float b0=A*((A+1.f)-(A-1.f)*cosW+2.f*std::sqrt(A)*alpha);
            float b1=2.f*A*((A-1.f)-(A+1.f)*cosW);
            float b2=A*((A+1.f)-(A-1.f)*cosW-2.f*std::sqrt(A)*alpha);
            float a0=(A+1.f)+(A-1.f)*cosW+2.f*std::sqrt(A)*alpha;
            float a1=-2.f*((A-1.f)+(A+1.f)*cosW);
            float a2=(A+1.f)+(A-1.f)*cosW-2.f*std::sqrt(A)*alpha;
            filters[band]={b0/a0,b1/a0,b2/a0,a1/a0,a2/a0};
        } else if (band == 3) {
            float b0=A*((A+1.f)+(A-1.f)*cosW+2.f*std::sqrt(A)*alpha);
            float b1=-2.f*A*((A-1.f)+(A+1.f)*cosW);
            float b2=A*((A+1.f)+(A-1.f)*cosW-2.f*std::sqrt(A)*alpha);
            float a0=(A+1.f)-(A-1.f)*cosW+2.f*std::sqrt(A)*alpha;
            float a1=2.f*((A-1.f)-(A+1.f)*cosW);
            float a2=(A+1.f)-(A-1.f)*cosW-2.f*std::sqrt(A)*alpha;
            filters[band]={b0/a0,b1/a0,b2/a0,a1/a0,a2/a0};
        } else {
            float b0=1.f+alpha*A, b1=-2.f*cosW, b2=1.f-alpha*A;
            float a0=1.f+alpha/A, a1b=-2.f*cosW, a2=1.f-alpha/A;
            filters[band]={b0/a0,b1/a0,b2/a0,a1b/a0,a2/a0};
        }
    }
    std::vector<float> response(numPx);
    for (int px = 0; px < numPx; ++px) {
        float norm = (float)px / (float)(numPx-1);
        float hz   = 20.f * std::pow(1000.f, norm);
        float w    = 2.f * MathConstants<float>::pi * hz / sampleRate;
        float totalDb = 0.f;
        for (int b = 0; b < 4; ++b)
            totalDb += biquadMagDb(filters[b].b0,filters[b].b1,filters[b].b2,
                                    filters[b].a1,filters[b].a2, w);
        response[px] = totalDb;
    }
    return response;
}

// ── Q bandwidth visualisation ─────────────────────────────────────────────
// Draw a shaded region whose width reflects the band Q visually:
//   low Q  (0.1) → very wide band covers ~4 octaves
//   high Q (8.0) → narrow spike covers ~0.1 octave
// Width in log-freq pixels = log-bandwidth of that filter at -3dB.
void EQDisplay::drawQBand(Graphics& g, int b, Point<float> pt) const
{
    const float w = (float)getWidth(), h = (float)getHeight();
    float q       = bands[b].qVisual;
    float freq    = normXToFreq(b, bands[b].normX);

    // -3 dB bandwidth formula: BW = f0 / Q
    // In log scale: pixel-width proportional to log(f_hi/f_lo) / log(20000/20)
    float f_lo = freq / std::sqrt(1.f + 1.f/(4.f*q*q)) - freq/(2.f*q);
    float f_hi = f_lo + freq / q;
    f_lo = jmax(20.f, f_lo);
    f_hi = jmin(20000.f, f_hi);

    auto freqToPx = [&](float f) {
        return std::log10(f / 20.f) / std::log10(1000.f) * w;
    };
    float xLo = freqToPx(f_lo);
    float xHi = freqToPx(f_hi);
    float bwPx = jmax(4.f, xHi - xLo);

    // Gradient fill: gold glow, stronger in centre, fades at edges
    ColourGradient cg(
        SordLookAndFeel::gold.withAlpha(0.18f), pt.x, 0.f,
        SordLookAndFeel::gold.withAlpha(0.00f), pt.x + bwPx * 0.5f, 0.f,
        false);
    g.setGradientFill(cg);
    g.fillRect(pt.x - bwPx * 0.5f, 0.f, bwPx, h);

    // Bandwidth boundary lines (dashed feel via alpha)
    g.setColour(SordLookAndFeel::gold.withAlpha(0.30f));
    g.drawLine(pt.x - bwPx * 0.5f, 0.f, pt.x - bwPx * 0.5f, h, 0.8f);
    g.drawLine(pt.x + bwPx * 0.5f, 0.f, pt.x + bwPx * 0.5f, h, 0.8f);

    // Arch above dot showing Q width
    float arcH = jlimit(5.f, 30.f, bwPx * 0.25f);
    Path arc;
    arc.startNewSubPath(pt.x - bwPx*0.5f, pt.y);
    arc.cubicTo(pt.x - bwPx*0.5f, pt.y - arcH,
                pt.x + bwPx*0.5f, pt.y - arcH,
                pt.x + bwPx*0.5f, pt.y);
    g.setColour(SordLookAndFeel::gold.withAlpha(0.55f));
    g.strokePath(arc, PathStrokeType(1.2f, PathStrokeType::curved, PathStrokeType::rounded));

    // Tooltip: freq + Q value
    const String fID[] = {"eq0Freq","eq1Freq","eq2Freq","eq3Freq"};
    float f = *apvts.getRawParameterValue(fID[b]);
    String freqStr = f >= 1000.f ? String(f/1000.f,2)+" kHz" : String((int)f)+" Hz";
    String tip = freqStr + "   Q " + String(q, 2);
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(10.f)));
    int tw = g.getCurrentFont().getStringWidth(tip) + 16;
    float tipX = jlimit(4.f, w - tw - 4.f, pt.x - tw * 0.5f);
    g.setColour(SordLookAndFeel::bg2.withAlpha(0.92f));
    g.fillRoundedRectangle(tipX, 4.f, (float)tw, 14.f, 3.f);
    g.setColour(SordLookAndFeel::border);
    g.drawRoundedRectangle(tipX, 4.f, (float)tw, 14.f, 3.f, 0.6f);
    g.setColour(SordLookAndFeel::gold.withAlpha(0.92f));
    g.drawText(tip, (int)tipX, 4, tw, 14, Justification::centred);
}

void EQDisplay::drawCurve(Graphics& g) const
{
    const float w = (float)getWidth(), h = (float)getHeight(), mid = h * 0.5f;
    const float maxDb = GAIN_RANGE;
    int nPx = (int)w;
    auto response = computeEQResponse(apvts, nPx);
    Path fill, curve;
    bool started = false;
    for (int px = 0; px < nPx; ++px) {
        float db = jlimit(-maxDb*1.5f, maxDb*1.5f, response[px]);
        float py = mid - (db / maxDb) * mid;
        py = jlimit(0.f, h, py);
        if (!started) { curve.startNewSubPath((float)px,py); fill.startNewSubPath((float)px,mid); started=true; }
        curve.lineTo((float)px, py);
        fill.lineTo((float)px, py);
    }
    fill.lineTo(w,mid); fill.closeSubPath();
    g.setColour(SordLookAndFeel::gold.withAlpha(0.07f)); g.fillPath(fill);
    g.setColour(SordLookAndFeel::gold.withAlpha(0.90f));
    g.strokePath(curve, PathStrokeType(1.6f, PathStrokeType::curved, PathStrokeType::rounded));

    // Band dots
    for (int i = 0; i < 4; ++i) {
        auto pt = bandToPixel(i);
        bool hov = (i == hoveredBand || i == dragging);
        if (hov) {
            drawQBand(g, i, pt);  // ← Q bandwidth visual
            g.setColour(SordLookAndFeel::gold.withAlpha(0.22f));
            g.fillEllipse(pt.x-10.f, pt.y-10.f, 20.f, 20.f);
        }
        g.setColour(hov ? SordLookAndFeel::gold.brighter(0.4f) : SordLookAndFeel::gold);
        g.fillEllipse(pt.x-5.f, pt.y-5.f, 10.f, 10.f);
        g.setColour(SordLookAndFeel::bg);
        g.drawEllipse(pt.x-5.f, pt.y-5.f, 10.f, 10.f, 1.f);
    }
}

void EQDisplay::paint(Graphics& g)
{
    const float w=(float)getWidth(), h=(float)getHeight(), mid=h*0.5f;
    g.setColour(Colour(0xff090908)); g.fillRoundedRectangle(0,0,w,h,2.f);
    g.setColour(SordLookAndFeel::border); g.drawRoundedRectangle(0.5f,0.5f,w-1.f,h-1.f,2.f,1.f);

    // Grid
    auto dash=[&](float y, Colour c){
        float x=0; while(x<w){ g.setColour(c); g.drawLine(x,y,jmin(x+4.f,w),y,0.5f); x+=9.f; }
    };
    g.setColour(SordLookAndFeel::border.brighter(0.4f)); g.drawLine(0,mid,w,mid,0.8f);
    dash(h*gainToNormY( 6.f), SordLookAndFeel::textFaint.brighter(0.4f));
    dash(h*gainToNormY(12.f), SordLookAndFeel::textFaint.brighter(0.2f));
    dash(h*gainToNormY(-6.f), SordLookAndFeel::textFaint.brighter(0.4f));
    dash(h*gainToNormY(-12.f),SordLookAndFeel::textFaint.brighter(0.2f));

    // Gain labels
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(8.f)));
    auto lbl=[&](float gain,const String& t){
        float y=h*gainToNormY(gain);
        g.setColour(SordLookAndFeel::textFaint.brighter(0.5f));
        g.drawText(t,3,(int)(y-6),22,12,Justification::centredLeft);
    };
    lbl(12.f,"+12"); lbl(6.f,"+6"); lbl(0.f,"0"); lbl(-6.f,"-6"); lbl(-12.f,"-12");

    // Frequency grid markers
    const float markerFreqs[]={100.f,200.f,500.f,1000.f,2000.f,5000.f,10000.f};
    for (float mf : markerFreqs) {
        float nx = std::log10(mf/20.f) / std::log10(1000.f) * w;
        g.setColour(SordLookAndFeel::textFaint.withAlpha(0.4f));
        g.drawLine(nx,0,nx,h,0.3f);
        String s = mf>=1000.f ? String((int)(mf/1000.f))+"k" : String((int)mf);
        g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(7.5f)));
        g.setColour(SordLookAndFeel::textFaint.brighter(0.25f));
        g.drawText(s,(int)(nx-10),(int)(h-13),22,11,Justification::centred);
    }

    drawCurve(g);

    // Band freq labels at dot x positions (bottom of display)
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(8.f)));
    g.setColour(SordLookAndFeel::textFaint.brighter(0.35f));
    const String fID[]= {"eq0Freq","eq1Freq","eq2Freq","eq3Freq"};
    for (int i = 0; i < 4; ++i) {
        auto bp = bandToPixel(i);
        float f = *apvts.getRawParameterValue(fID[i]);
        String fs = f>=1000.f ? String(f/1000.f,1)+" kHz" : String((int)f)+" Hz";
        g.drawText(fs, (int)(bp.x-22), (int)(h-14), 44, 12, Justification::centred);
    }

    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(8.f)));
    g.setColour(SordLookAndFeel::textFaint.brighter(0.3f));
    g.drawText("scroll=Q  dblclick=freq", 3, 3, (int)w-6, 11, Justification::centredRight);
}

// ═══════════════════════════════════════════════════════════════════════════
//  VUMeter
// ═══════════════════════════════════════════════════════════════════════════
VUMeter::VUMeter() { startTimerHz(30); }
void VUMeter::setLevel(float rms) { rmsAtomic.store(rms); }
void VUMeter::timerCallback()
{
    float target = jmap(Decibels::gainToDecibels(rmsAtomic.load(), -60.f), -60.f, 6.f, 0.f, 1.f);
    displayLevel += (target>displayLevel)?(target-displayLevel)*0.55f:(target-displayLevel)*0.04f;
    if (target>peak){peak=target;peakHold=90;}else if(peakHold>0)--peakHold;else peak=jmax(0.f,peak-0.004f);
    repaint();
}
void VUMeter::paint(Graphics& g)
{
    const float w=(float)getWidth(), h=(float)getHeight();
    g.setColour(Colour(0xff090908)); g.fillRoundedRectangle(0,0,w,h,2.f);
    g.setColour(SordLookAndFeel::border); g.drawRoundedRectangle(0.5f,0.5f,w-1.f,h-1.f,2.f,1.f);
    const int nSegs=32; const float segH=(h-4.f)/nSegs;
    for (int i=0;i<nSegs;++i) {
        float normDb=jmap((float)(nSegs-1-i)/(nSegs-1),0.f,1.f,-60.f,6.f);
        float normDisp=jmap(normDb,-60.f,6.f,0.f,1.f);
        float y=2.f+i*segH; bool lit=(displayLevel>=normDisp);
        Colour c=normDb>0.f?(lit?Colour(0xffc05040):Colour(0xff3a1818)):
                  normDb>-6.f?(lit?Colour(0xffb09040):Colour(0xff2a2010)):
                  (lit?SordLookAndFeel::green:Colour(0xff182018));
        g.setColour(c); g.fillRoundedRectangle(2.f,y,w-4.f,segH-0.8f,0.5f);
    }
    if (peak>0.f) {
        float py=2.f+(1.f-peak)*(h-4.f);
        g.setColour(peak>jmap(0.f,-60.f,6.f,0.f,1.f)?SordLookAndFeel::red:SordLookAndFeel::gold);
        g.fillRoundedRectangle(2.f,py,w-4.f,1.8f,0.5f);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  PluginEditor — constructor
// ═══════════════════════════════════════════════════════════════════════════
SordIRAudioProcessorEditor::SordIRAudioProcessorEditor(SordIRAudioProcessor& p)
    : AudioProcessorEditor(p), processor(p), eqDisplay(p.getAPVTS())
{
    setLookAndFeel(&laf);
    setSize(PLUGIN_W, PLUGIN_H);
    setResizable(true, true);
    setResizeLimits(MIN_W, MIN_H, 1400, 1400);

    // Header
    importButton.setButtonText("IMPORT IR");
    addAndMakeVisible(importButton);
    importButton.onClick=[this]{
        auto ch=std::make_shared<FileChooser>("Import IR",File{},"*.wav;*.aif;*.aiff");
        ch->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles,
            [this,ch](const FileChooser& fc){
                auto f=fc.getResult();
                if(f.existsAsFile()){
                    processor.loadCustomIR(f);
                    currentIRLabel.setText("Custom: "+f.getFileNameWithoutExtension(),dontSendNotification);
                    irDurationLabel.setText("",dontSendNotification);
                    for(auto*r:presetRows) r->isActive=false, r->repaint();
                    for(auto*r:folderRows) r->isSelected=false, r->repaint();
                }
            });
    };

    tabIR.setButtonText("IR LIBRARY"); addAndMakeVisible(tabIR);
    tabUser.setButtonText("USER");      addAndMakeVisible(tabUser);
    tabIR.onClick   = [this]{ switchTab(false); };
    tabUser.onClick = [this]{ switchTab(true);  };

    addAndMakeVisible(irBrowser);
    folderViewport.setViewedComponent(&folderList, false);
    folderViewport.setScrollBarsShown(true, false);
    irBrowser.addAndMakeVisible(folderViewport);
    presetViewport.setViewedComponent(&presetList, false);
    presetViewport.setScrollBarsShown(true, false);
    irBrowser.addAndMakeVisible(presetViewport);

    currentIRLabel.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(9.5f)));
    currentIRLabel.setColour(Label::textColourId, SordLookAndFeel::textFaint.brighter(0.6f));
    currentIRLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(currentIRLabel);
    irDurationLabel.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(9.5f)));
    irDurationLabel.setColour(Label::textColourId, SordLookAndFeel::textFaint.brighter(0.4f));
    irDurationLabel.setJustificationType(Justification::centredRight);
    addAndMakeVisible(irDurationLabel);

    buildCategories();
    buildFolderRows();
    selectCategory(0);

    // User panel
    addChildComponent(userPanel);
    savePresetField.setText(savePresetName, dontSendNotification);
    savePresetField.setFont(Font(FontOptions().withName("Barlow").withHeight(12.f)));
    savePresetField.setColour(Label::textColourId, SordLookAndFeel::textMain);
    savePresetField.setColour(Label::backgroundColourId, SordLookAndFeel::bg2);
    savePresetField.setColour(Label::outlineColourId, SordLookAndFeel::border);
    savePresetField.setEditable(true,true,false);
    savePresetField.onTextChange=[this]{savePresetName=savePresetField.getText();};
    userPanel.addAndMakeVisible(savePresetField);
    savePresetButton.setButtonText("SAVE");
    savePresetButton.onClick=[this]{promptSavePreset();};
    userPanel.addAndMakeVisible(savePresetButton);
    noPresetsLabel.setText("No saved presets.", dontSendNotification);
    noPresetsLabel.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(10.f)));
    noPresetsLabel.setColour(Label::textColourId, SordLookAndFeel::textFaint.brighter(0.3f));
    noPresetsLabel.setJustificationType(Justification::centred);
    userPanel.addAndMakeVisible(noPresetsLabel);
    userScrollView.setViewedComponent(&userListContainer, false);
    userScrollView.setScrollBarsShown(true, false);
    userPanel.addAndMakeVisible(userScrollView);

    // Main knobs
    auto setupKnob=[&](Slider& s, Label& l, Label& v, const String& name, const String& val){
        s.setSliderStyle(Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        s.setDoubleClickReturnValue(true, 0.0);
        addAndMakeVisible(s); addAndMakeVisible(l); addAndMakeVisible(v);
        l.setText(name, dontSendNotification);
        l.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(9.f)));
        l.setColour(Label::textColourId, SordLookAndFeel::textFaint.brighter(0.5f));
        l.setJustificationType(Justification::centred);
        v.setText(val, dontSendNotification);
        v.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(10.f)));
        v.setColour(Label::textColourId, SordLookAndFeel::textMain);
        v.setJustificationType(Justification::centred);
    };
    setupKnob(predelayKnob, predelayLabel, predelayVal, "PRE-DELAY", "0 ms");
    setupKnob(sizeKnob,     sizeLabel,     sizeVal,     "SIZE",      "1.00x");
    setupKnob(dryWetKnob,   dryWetLabel,   dryWetVal,   "DRY / WET","100%");
    setupKnob(gainKnob,     gainLabel,     gainVal,     "GAIN",      "0 dB");

    predelayKnob.onValueChange=[this]{
        predelayVal.setText(String(predelayKnob.getValue(),1)+" ms", dontSendNotification);
    };
    sizeKnob.onValueChange=[this]{
        sizeVal.setText(String(sizeKnob.getValue(),2)+"x", dontSendNotification);
        updateIRDurationLabel();
    };
    dryWetKnob.onValueChange=[this]{
        dryWetVal.setText(String((int)(dryWetKnob.getValue()*100))+"%", dontSendNotification);
    };
    gainKnob.onValueChange=[this]{
        gainVal.setText(String(gainKnob.getValue(),1)+" dB", dontSendNotification);
    };

    // EQ — display only, no knob row
    eqOnButton.setButtonText("EQ ON");
    eqOnButton.setToggleState(true, dontSendNotification);
    addAndMakeVisible(eqOnButton);
    addAndMakeVisible(eqDisplay);

    // Output
    outputFader.setSliderStyle(Slider::LinearVertical);
    outputFader.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(outputFader);
    addAndMakeVisible(vuMeter);
    bypassButton.setButtonText("ACTIVE");
    bypassButton.setToggleState(true, dontSendNotification);
    addAndMakeVisible(bypassButton);

    // APVTS attachments
    auto& apvts = processor.getAPVTS();
    predelayAttach = std::make_unique<SliderAttachment>(apvts, ParamID::predelay, predelayKnob);
    sizeAttach     = std::make_unique<SliderAttachment>(apvts, ParamID::size,     sizeKnob);
    dryWetAttach   = std::make_unique<SliderAttachment>(apvts, ParamID::dryWet,   dryWetKnob);
    gainAttach     = std::make_unique<SliderAttachment>(apvts, ParamID::gainOut,  gainKnob);
    faderAttach    = std::make_unique<SliderAttachment>(apvts, ParamID::gainOut,  outputFader);
    eqOnAttach     = std::make_unique<ButtonAttachment>(apvts, ParamID::eqOn,     eqOnButton);

    processor.loadUserPresetsFromDisk();
    refreshUserPanel();
    switchTab(false);
    startTimerHz(30);
    selectIR(0);
}

SordIRAudioProcessorEditor::~SordIRAudioProcessorEditor()
{ stopTimer(); setLookAndFeel(nullptr); }

void SordIRAudioProcessorEditor::timerCallback()
{
    vuMeter.setLevel(processor.getOutputLevel());
}

void SordIRAudioProcessorEditor::updateIRDurationLabel()
{
    float dur = processor.getCurrentIRDuration();
    if (dur <= 0.f) { irDurationLabel.setText("",dontSendNotification); return; }
    int idx = processor.getCurrentIRIndex();
    float sr = 44100.f;
    if (idx >= 0 && idx < processor.getIRPresets().size())
        sr = (float)processor.getIRPresets()[idx].sampleRate;
    String srStr = sr >= 1000.f ? String(sr/1000.f,1)+" kHz" : String((int)sr)+" Hz";
    irDurationLabel.setText(String(dur,2)+" s  ·  "+srStr, dontSendNotification);
}

// ── Categories ────────────────────────────────────────────────────────────────
void SordIRAudioProcessorEditor::buildCategories()
{
    categories.clear();
    const auto& presets = processor.getIRPresets();
    StringArray catOrder;
    for (auto& p : presets)
        if (!catOrder.contains(p.category)) catOrder.add(p.category);
    for (auto& catName : catOrder) {
        IRCategory cat;
        cat.name        = catName;
        cat.displayName = catName.toUpperCase();
        if      (catName == "Trumpet")    cat.icon = InstrumentIcons::trumpet();
        else if (catName == "Trombone")   cat.icon = InstrumentIcons::trombone();
        else if (catName == "Horn")       cat.icon = InstrumentIcons::horn();
        else if (catName == "Tuba")       cat.icon = InstrumentIcons::tuba();
        else if (catName == "Strings")    cat.icon = InstrumentIcons::strings();
        else if (catName == "Spaces")     cat.icon = InstrumentIcons::space();
        else if (catName == "Bricasti")  cat.icon = InstrumentIcons::bricasti();
        for (int i = 0; i < presets.size(); ++i)
            if (presets[i].category == catName) cat.presetIndices.push_back(i);
        categories.push_back(std::move(cat));
    }
}

void SordIRAudioProcessorEditor::buildFolderRows()
{
    folderRows.clear();
    for (auto* c : folderList.getChildren()) folderList.removeChildComponent(c);
    const int rowH = 44;
    for (int i = 0; i < (int)categories.size(); ++i) {
        auto& cat = categories[(size_t)i];
        auto* row = new FolderRow(cat.displayName, cat.icon, (int)cat.presetIndices.size());
        row->isSelected = (i == selectedCategoryIndex);
        int idx = i;
        row->onSelect = [this,idx]{ selectCategory(idx); };
        folderRows.add(row);
        folderList.addAndMakeVisible(row);
        row->setBounds(0, i*rowH, FOLDER_W, rowH);
    }
    folderList.setSize(FOLDER_W, jmax(44, (int)categories.size() * rowH));
}

void SordIRAudioProcessorEditor::selectCategory(int index)
{
    selectedCategoryIndex = index;
    for (int i = 0; i < folderRows.size(); ++i)
        folderRows[i]->isSelected = (i==index), folderRows[i]->repaint();
    buildPresetRows(index);
}

void SordIRAudioProcessorEditor::buildPresetRows(int categoryIndex)
{
    presetRows.clear();
    for (auto* c : presetList.getChildren()) presetList.removeChildComponent(c);
    if (categoryIndex<0 || categoryIndex>=(int)categories.size()) return;
    const auto& cat = categories[(size_t)categoryIndex];
    const auto& allPresets = processor.getIRPresets();
    const int currentIdx = processor.getCurrentIRIndex();
    const int rowH = 32;
    for (int i = 0; i < (int)cat.presetIndices.size(); ++i) {
        int gIdx = cat.presetIndices[(size_t)i];
        auto* row = new PresetRow(allPresets[gIdx].name, gIdx);
        row->isActive = (gIdx == currentIdx);
        row->onClick  = [this,gIdx]{ selectIR(gIdx); };
        presetRows.add(row);
        presetList.addAndMakeVisible(row);
        row->setBounds(0, i*rowH, 300, rowH);
    }
    presetList.setSize(300, jmax(rowH, (int)cat.presetIndices.size()*rowH));
    resized();
}

void SordIRAudioProcessorEditor::selectIR(int globalIndex)
{
    processor.loadIRByIndex(globalIndex);
    if (auto* p = processor.getAPVTS().getParameter(ParamID::irIndex))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)globalIndex));
    for (auto* r : presetRows)
        r->isActive = (r->globalIndex == globalIndex), r->repaint();
    const auto& pr = processor.getIRPresets()[globalIndex];
    currentIRLabel.setText(pr.category+"  —  "+pr.name, dontSendNotification);
    updateIRDurationLabel();
}

void SordIRAudioProcessorEditor::switchTab(bool userTab)
{
    showingUserTab = userTab;
    irBrowser      .setVisible(!userTab);
    currentIRLabel .setVisible(!userTab);
    irDurationLabel.setVisible(!userTab);
    userPanel      .setVisible(userTab);
    if (userTab) refreshUserPanel();
    resized(); repaint();
}

void SordIRAudioProcessorEditor::refreshUserPanel()
{
    userPresetRows.clear();
    for (auto* c : userListContainer.getChildren()) userListContainer.removeChildComponent(c);
    const auto& presets = processor.getUserPresets();
    noPresetsLabel.setVisible(presets.isEmpty());
    const int rowH = 30;
    for (int i = 0; i < presets.size(); ++i) {
        auto* row = new UserPresetRow(presets[i].name);
        int idx = i;
        row->onLoad   = [this,idx]{ processor.loadUserPreset(idx); refreshUserPanel(); };
        row->onDelete = [this,idx]{ processor.deleteUserPreset(idx); refreshUserPanel(); };
        userPresetRows.add(row);
        userListContainer.addAndMakeVisible(row);
        row->setBounds(0, i*rowH, userPanel.getWidth()-16, rowH);
    }
    userListContainer.setSize(jmax(100,userPanel.getWidth()-16),
                               jmax(rowH, presets.size()*rowH));
    resized();
}

void SordIRAudioProcessorEditor::promptSavePreset()
{
    auto name = savePresetName.trim();
    if (name.isEmpty()) name = "My Preset";
    processor.saveUserPreset(name);
    refreshUserPanel();
}

// ═══════════════════════════════════════════════════════════════════════════
//  resized()
// ═══════════════════════════════════════════════════════════════════════════
void SordIRAudioProcessorEditor::resized()
{
    const int W = getWidth(), H = getHeight();
    const int browserW = W - OUT_W;

    importButton.setBounds(W-112, HEADER_H/2-12, 98, 24);
    tabIR  .setBounds(20,  HEADER_H+4, 88, 22);
    tabUser.setBounds(112, HEADER_H+4, 54, 22);

    const int browserY = HEADER_H + TABBAR_H;
    irBrowser.setBounds(0, browserY, browserW, BROWSER_H);
    userPanel.setBounds(0, browserY, browserW, BROWSER_H);

    const int presetW = browserW - FOLDER_W;
    folderViewport.setBounds(0,        0, FOLDER_W, BROWSER_H);
    presetViewport.setBounds(FOLDER_W, 0, presetW,  BROWSER_H);
    for (auto* r : presetRows) r->setSize(presetW-16, r->getHeight());
    if (!presetRows.isEmpty()) presetList.setSize(presetW-16, presetRows.size()*32);

    savePresetField .setBounds(8,  8, browserW-96, 24);
    savePresetButton.setBounds(browserW-84, 8, 72, 24);
    userScrollView  .setBounds(0,  38, browserW-4, BROWSER_H-38);
    noPresetsLabel  .setBounds(0,  38, browserW-4, BROWSER_H-38);
    userListContainer.setSize(jmax(100,browserW-20), jmax(30,userPresetRows.size()*30));

    const int labelY = browserY + BROWSER_H;
    currentIRLabel .setBounds(20,           labelY+4, browserW/2-20, 14);
    irDurationLabel.setBounds(browserW/2,   labelY+4, browserW/2-20, 14);

    const int waveY  = labelY + 20;
    const int knobY  = waveY  + WAVEFORM_H;
    const int eqY    = knobY  + CONTROLS_H;
    const int eqOnH  = 24;
    // EQ display takes all remaining height (no knob row below)
    const int eqDispH = H - eqY - eqOnH - FOOTER_H - 8;

    // Knobs (4: predelay, size, drywet, gain)
    const int knobW = (W - OUT_W) / 4;
    Slider* ks[] = {&predelayKnob, &sizeKnob, &dryWetKnob, &gainKnob};
    Label*  kl[] = {&predelayLabel,&sizeLabel,&dryWetLabel,&gainLabel};
    Label*  kv[] = {&predelayVal,  &sizeVal,  &dryWetVal,  &gainVal };
    for (int i = 0; i < 4; ++i) {
        int cx = i * knobW + knobW/2;
        kl[i]->setBounds(cx-32, knobY+4,  64, 12);
        ks[i]->setBounds(cx-28, knobY+16, 56, 56);
        kv[i]->setBounds(cx-32, knobY+72, 64, 14);
    }

    eqOnButton.setBounds(W-OUT_W-80, eqY+2, 70, eqOnH);
    eqDisplay .setBounds(16, eqY+eqOnH+4, W-OUT_W-24, jmax(80, eqDispH));

    // Output strip
    const int vuTop = browserY + 20;
    const int vuH   = H - vuTop - FOOTER_H - 20;
    vuMeter    .setBounds(W-OUT_W+6,  vuTop, 24, vuH);
    outputFader.setBounds(W-OUT_W+34, vuTop, OUT_W-40, vuH);

    bypassButton.setBounds(W-92, H-FOOTER_H+6, 82, 22);
}

// ═══════════════════════════════════════════════════════════════════════════
//  paint()
// ═══════════════════════════════════════════════════════════════════════════
void SordIRAudioProcessorEditor::paint(Graphics& g)
{
    const int W = getWidth(), H = getHeight();
    g.fillAll(SordLookAndFeel::bg);

    // Header
    ColourGradient hdrGrad(Colour(0xff171714),0.f,0.f,Colour(0xff111110),0.f,(float)HEADER_H,false);
    g.setGradientFill(hdrGrad); g.fillRect(0,0,W,HEADER_H);
    g.setColour(SordLookAndFeel::border); g.drawLine(0.f,(float)HEADER_H,(float)W,(float)HEADER_H,1.f);

    // Logotype
    g.setFont(Font(FontOptions().withName("Cormorant Garamond").withHeight(82.f).withStyle("Light")));
    g.setColour(SordLookAndFeel::gold.withAlpha(0.95f));
    g.drawText("SORD", 20, 6, 340, 94, Justification::centredLeft);
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(14.f)));
    g.setColour(SordLookAndFeel::textFaint.brighter(0.6f));
    g.drawText("IR", 264, 66, 60, 18, Justification::centredLeft);

    // Tab bar
    g.setColour(SordLookAndFeel::bg2); g.fillRect(0,HEADER_H,W,TABBAR_H);
    g.setColour(SordLookAndFeel::border); g.drawLine(0.f,(float)(HEADER_H+TABBAR_H),(float)W,(float)(HEADER_H+TABBAR_H),1.f);
    int atx=showingUserTab?112:20, atw=showingUserTab?54:88;
    g.setColour(SordLookAndFeel::gold); g.fillRect(atx,HEADER_H+TABBAR_H-2,atw,2);

    // Browser background
    const int browserY=HEADER_H+TABBAR_H;
    g.setColour(SordLookAndFeel::bg); g.fillRect(0,browserY,W,BROWSER_H);
    g.setColour(SordLookAndFeel::border); g.drawLine(0.f,(float)(browserY+BROWSER_H),(float)W,(float)(browserY+BROWSER_H),1.f);
    if (!showingUserTab) {
        g.setColour(SordLookAndFeel::bg2); g.fillRect(0,browserY,FOLDER_W,BROWSER_H);
        g.setColour(SordLookAndFeel::border); g.drawLine((float)FOLDER_W,(float)browserY,(float)FOLDER_W,(float)(browserY+BROWSER_H),1.f);
    }

    // Output strip
    g.setColour(SordLookAndFeel::bg2); g.fillRect(W-OUT_W,browserY,OUT_W,H-browserY-FOOTER_H);
    g.setColour(SordLookAndFeel::border); g.drawLine((float)(W-OUT_W),(float)browserY,(float)(W-OUT_W),(float)(H-FOOTER_H),1.f);
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(8.5f).withStyle("Bold")));
    g.setColour(SordLookAndFeel::textFaint.brighter(0.5f));
    g.drawText("OUT",W-OUT_W,browserY+2,OUT_W,12,Justification::centred);

    // Section lines
    const int labelY=browserY+BROWSER_H, waveY=labelY+20;
    const int knobY=waveY+WAVEFORM_H, eqY=knobY+CONTROLS_H;
    g.setColour(SordLookAndFeel::border);
    g.drawLine(0.f,(float)waveY,(float)W,(float)waveY,1.f);
    g.drawLine(0.f,(float)(waveY+WAVEFORM_H),(float)W,(float)(waveY+WAVEFORM_H),1.f);
    g.drawLine(0.f,(float)eqY,(float)W,(float)eqY,1.f);

    // EQ label
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(9.f).withStyle("Bold")));
    g.setColour(SordLookAndFeel::textFaint.brighter(0.35f));
    g.drawText("EQ",20,eqY+5,40,14,Justification::centredLeft);

    // Knob section separators
    const int knobW=(W-OUT_W)/4;
    g.setColour(SordLookAndFeel::border.withAlpha(0.6f));
    for (int i=1;i<4;++i)
        g.drawLine((float)(i*knobW),(float)waveY+WAVEFORM_H+2.f,(float)(i*knobW),(float)eqY-2.f,0.5f);

    // Footer
    g.setColour(SordLookAndFeel::bg2); g.fillRect(0,H-FOOTER_H,W,FOOTER_H);
    g.setColour(SordLookAndFeel::border); g.drawLine(0.f,(float)(H-FOOTER_H),(float)W,(float)(H-FOOTER_H),1.f);
    g.setFont(Font(FontOptions().withName("Barlow Condensed").withHeight(9.f)));
    g.setColour(SordLookAndFeel::textFaint.brighter(0.3f));
    g.drawText("SORD IR  v1.5",20,H-26,120,14,Justification::centredLeft);
    g.drawText("0 dB",W-OUT_W,H-FOOTER_H-16,OUT_W,14,Justification::centred);

    paintWaveform(g, Rectangle<int>(0,waveY,W-OUT_W,WAVEFORM_H));

    // Grain
    g.setColour(Colours::white.withAlpha(0.011f));
    Random rng(42);
    for (int i=0;i<8000;++i) g.fillRect(rng.nextInt(W),rng.nextInt(H),1,1);
}

void SordIRAudioProcessorEditor::paintWaveform(Graphics& g, Rectangle<int> bounds)
{
    const float x=(float)bounds.getX(), y=(float)bounds.getY();
    const float w=(float)bounds.getWidth(), h=(float)bounds.getHeight(), mid=y+h*0.5f;
    g.setColour(Colour(0xff090908)); g.fillRect(bounds);
    Path top,bot; Random rng(123);
    top.startNewSubPath(x,mid); bot.startNewSubPath(x,mid);
    for (int i=0;i<=300;++i){
        float t=(float)i/300.f;
        float env=std::exp(-t*4.5f)*(1.f-std::exp(-t*30.f));
        float amp=env*(h*0.43f)*(0.55f+rng.nextFloat()*0.45f);
        top.lineTo(x+t*w,mid-amp); bot.lineTo(x+t*w,mid+amp);
    }
    Path fp; rng.setSeed(123);
    fp.startNewSubPath(x,mid);
    for (int i=0;i<=300;++i){
        float t=(float)i/300.f,env=std::exp(-t*4.5f)*(1.f-std::exp(-t*30.f)),amp=env*(h*0.43f)*(0.55f+rng.nextFloat()*0.45f);
        fp.lineTo(x+t*w,mid-amp);
    }
    rng.setSeed(123);
    for (int i=300;i>=0;--i){
        float t=(float)i/300.f,env=std::exp(-t*4.5f)*(1.f-std::exp(-t*30.f)),amp=env*(h*0.43f)*(0.55f+rng.nextFloat()*0.45f);
        fp.lineTo(x+t*w,mid+amp);
    }
    fp.closeSubPath();
    g.setColour(SordLookAndFeel::gold.withAlpha(0.09f)); g.fillPath(fp);
    g.setColour(SordLookAndFeel::gold.withAlpha(0.68f));
    g.strokePath(top,PathStrokeType(0.8f)); g.strokePath(bot,PathStrokeType(0.8f));
}

bool SordIRAudioProcessorEditor::isInterestedInFileDrag(const StringArray& f)
{ for(auto& s:f) if(s.endsWithIgnoreCase(".wav")||s.endsWithIgnoreCase(".aif")||s.endsWithIgnoreCase(".aiff")) return true; return false; }

void SordIRAudioProcessorEditor::filesDropped(const StringArray& files,int,int)
{
    for (auto& f : files){
        File file(f);
        if (file.existsAsFile()){
            processor.loadCustomIR(file);
            currentIRLabel.setText("Custom: "+file.getFileNameWithoutExtension(),dontSendNotification);
            irDurationLabel.setText("",dontSendNotification);
            for(auto*r:presetRows) r->isActive=false, r->repaint();
            break;
        }
    }
}
