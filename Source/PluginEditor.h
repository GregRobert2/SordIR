#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

// ═══════════════════════════════════════════════════════════════════════════
//  INSTRUMENT ICONS  — clean geometric silhouettes, no fioriture
//  Design space: approx 60 × 36 units, drawn to be legible at 28 px
// ═══════════════════════════════════════════════════════════════════════════
namespace InstrumentIcons {

// All icons designed in a ~64x38 unit space, drawn as recognisable silhouettes.

// TRUMPET — classic 3-valve side profile
static juce::Path trumpet()
{
    juce::Path p;
    // Mouthpiece cup
    p.addRoundedRectangle (0.f, 14.f, 7.f, 4.f, 1.5f);
    p.addRoundedRectangle (6.f, 15.f, 5.f, 2.5f, 1.f);
    // Three valve casings
    p.addRoundedRectangle (11.f,  7.f, 5.5f, 18.f, 2.2f);
    p.addRoundedRectangle (18.f,  7.f, 5.5f, 18.f, 2.2f);
    p.addRoundedRectangle (25.f,  7.f, 5.5f, 18.f, 2.2f);
    // Piston caps
    p.addRoundedRectangle (12.f,  4.f, 3.5f, 5.f,  1.8f);
    p.addRoundedRectangle (19.f,  4.f, 3.5f, 5.f,  1.8f);
    p.addRoundedRectangle (26.f,  4.f, 3.5f, 5.f,  1.8f);
    // Upper stay tube
    p.addRoundedRectangle (11.f, 14.f, 19.f, 2.f,  0.8f);
    // Lower stay tube
    p.addRoundedRectangle (11.f, 16.5f,19.f, 2.f,  0.8f);
    // Small return loop (left)
    p.startNewSubPath (11.f, 14.f);
    p.lineTo          ( 9.f, 14.f);
    p.cubicTo         ( 3.f, 14.f,  3.f, 22.f,  9.f, 22.f);
    p.lineTo          (11.f, 22.f);
    // Large forward loop (right)
    p.startNewSubPath (30.f, 15.f);
    p.lineTo          (34.f, 15.f);
    p.cubicTo         (42.f, 13.f, 42.f,  5.f, 36.f,  5.f);
    p.cubicTo         (32.f,  5.f, 30.f,  8.f, 30.f, 12.f);
    // Bell flare
    p.startNewSubPath (38.f, 14.5f);
    p.lineTo          (43.f, 14.f);
    p.cubicTo         (50.f, 12.f, 56.f,  7.f, 60.f,  7.f);
    p.lineTo          (64.f,  7.f);
    p.cubicTo         (66.f,  5.5f, 66.f,  9.f, 66.f, 12.f);
    p.cubicTo         (66.f, 17.f, 64.f, 17.f, 60.f, 17.f);
    p.cubicTo         (56.f, 17.f, 50.f, 20.f, 43.f, 20.f);
    p.lineTo          (38.f, 20.f);
    p.closeSubPath ();
    return p;
}

// TROMBONE — two long parallel tubes, U-bow right, cross-brace, bell left
// All coordinates positive (bell on left, slide extends right)
static juce::Path trombone()
{
    juce::Path p;
    // Bell flare (left end) — wide opening facing left
    p.startNewSubPath (18.f,  5.f);
    p.cubicTo         (14.f,  5.f, 10.f,  2.f,  6.f,  0.f);
    p.lineTo          ( 2.f,  0.f);
    p.cubicTo         (-1.f, -1.f, -1.f,  4.f, -1.f,  7.f);
    p.cubicTo         (-1.f, 10.f,  2.f, 12.f,  6.f, 12.f);
    p.cubicTo         (10.f, 12.f, 14.f, 11.f, 18.f,  9.5f);
    p.closeSubPath ();
    // Upper tube
    p.addRoundedRectangle (17.f,  5.f, 50.f, 3.5f, 1.5f);
    // Lower tube
    p.addRoundedRectangle (17.f, 18.f, 40.f, 3.5f, 1.5f);
    // Cross brace
    p.addRoundedRectangle (32.f,  8.5f, 3.f,  9.f, 1.f);
    // U-bow (right end)
    p.startNewSubPath (66.f,  8.5f);
    p.cubicTo         (74.f,  8.5f, 74.f, 22.f, 66.f, 22.f);
    p.lineTo          (66.f, 21.5f);
    p.cubicTo         (73.f, 21.5f, 73.f,  9.f, 66.f,  9.f);
    p.closeSubPath ();
    // Mouthpiece (right end of upper tube)
    p.addRoundedRectangle (67.f,  6.f, 7.f, 3.5f, 1.5f);
    return p;
}

// FRENCH HORN — 3 concentric coil arcs + rotary valves + bell right
static juce::Path horn()
{
    juce::Path p;
    const float cx = 19.f, cy = 19.f;
    const float pi = juce::MathConstants<float>::pi;
    // Three coil arcs (outer to inner, each almost full circle, open at bottom-right)
    const float startA = pi * 0.1f, endA = pi * 1.9f;
    for (int i = 0; i < 3; ++i) {
        float r = 13.f - i * 4.f;
        p.addCentredArc (cx, cy, r, r, 0.f, startA, endA, true);
    }
    // Three rotary valves (small filled circles on the coil body)
    p.addEllipse (15.f, 10.f, 4.5f, 4.5f);
    p.addEllipse (21.f,  7.f, 4.5f, 4.5f);
    p.addEllipse (27.f, 10.f, 4.5f, 4.5f);
    // Mouthpipe (top-left)
    p.addRoundedRectangle (0.f, 5.f, 9.f, 2.8f, 1.f);
    // Connecting pipe to bell
    p.addRoundedRectangle (30.f, 16.f, 5.f, 3.f, 1.f);
    // Bell flare (right-facing)
    p.startNewSubPath (33.f, 16.f);
    p.lineTo          (37.f, 16.f);
    p.cubicTo         (44.f, 14.f, 50.f, 9.f, 54.f, 9.f);
    p.lineTo          (59.f,  9.f);
    p.cubicTo         (62.f,  8.f, 62.f, 12.f, 62.f, 15.f);
    p.cubicTo         (62.f, 19.f, 60.f, 19.f, 57.f, 19.f);
    p.cubicTo         (52.f, 19.f, 46.f, 22.f, 38.f, 22.f);
    p.lineTo          (33.f, 22.f);
    p.closeSubPath ();
    return p;
}

// TUBA — upright front view: wide bell top, oval body, 4 pistons
static juce::Path tuba()
{
    juce::Path p;
    // Bell mouth
    p.startNewSubPath ( 5.f, 14.f);
    p.lineTo          ( 5.f,  8.f);
    p.cubicTo         ( 5.f,  2.f, 21.f,  2.f, 21.f,  2.f);
    p.cubicTo         (21.f,  2.f, 37.f,  2.f, 37.f,  8.f);
    p.lineTo          (37.f, 14.f);
    p.closeSubPath ();
    // Body (ellipse)
    p.addEllipse (3.f, 8.f, 36.f, 26.f);
    // Valve block
    p.addRoundedRectangle (4.f, 31.f, 34.f, 8.f, 2.f);
    // 4 pistons
    p.addRoundedRectangle ( 4.f, 28.f, 7.f, 13.f, 3.f);
    p.addRoundedRectangle (13.f, 28.f, 7.f, 13.f, 3.f);
    p.addRoundedRectangle (22.f, 28.f, 7.f, 13.f, 3.f);
    p.addRoundedRectangle (31.f, 28.f, 7.f, 13.f, 3.f);
    // Side mouthpipe
    p.addRoundedRectangle (39.f, 17.f, 7.f, 3.f, 1.f);
    return p;
}

// STRINGS (CELLO) — front silhouette: double-bout body, f-holes, neck, scroll
static juce::Path strings()
{
    juce::Path p;
    // Body
    p.startNewSubPath (24.f, 46.f);
    p.cubicTo         ( 5.f, 46.f,  3.f, 34.f,  8.f, 28.f);
    p.cubicTo         ( 3.f, 26.f,  3.f, 20.f,  8.f, 18.f);
    p.cubicTo         ( 2.f, 12.f,  4.f,  0.f, 24.f,  0.f);
    p.lineTo          (28.f,  0.f);
    p.cubicTo         (48.f,  0.f, 50.f, 12.f, 44.f, 18.f);
    p.cubicTo         (49.f, 20.f, 49.f, 26.f, 44.f, 28.f);
    p.cubicTo         (49.f, 34.f, 47.f, 46.f, 28.f, 46.f);
    p.closeSubPath ();
    // Neck
    p.startNewSubPath (22.f,  0.f); p.lineTo (20.f, -15.f);
    p.lineTo          (32.f, -15.f); p.lineTo (30.f,  0.f);
    // Scroll
    p.startNewSubPath (20.f, -15.f);
    p.cubicTo         (13.f, -19.f, 12.f, -26.f, 18.f, -26.f);
    p.cubicTo         (24.f, -26.f, 26.f, -19.f, 22.f, -17.f);
    p.cubicTo         (21.f, -15.f, 20.f, -15.f, 20.f, -15.f);
    // Left f-hole
    p.addEllipse (11.f, 16.f, 4.f, 4.f);
    p.startNewSubPath (13.f, 20.f); p.lineTo (13.f, 27.f);
    p.addEllipse (11.f, 27.f, 4.f, 4.f);
    // Right f-hole
    p.addEllipse (37.f, 16.f, 4.f, 4.f);
    p.startNewSubPath (39.f, 20.f); p.lineTo (39.f, 27.f);
    p.addEllipse (37.f, 27.f, 4.f, 4.f);
    // Bridge
    p.addRoundedRectangle (20.f, 22.f, 12.f, 2.5f, 0.5f);
    return p;
}

// CONCERT HALL — vaulted arch cross-section with acoustic panels
static juce::Path space()
{
    juce::Path p;
    // Outer arch walls
    p.startNewSubPath ( 0.f, 44.f); p.lineTo ( 0.f, 20.f);
    p.cubicTo         ( 0.f,  3.f,  9.f,  0.f, 22.f,  0.f);
    p.cubicTo         (35.f,  0.f, 44.f,  3.f, 44.f, 20.f);
    p.lineTo          (44.f, 44.f);
    p.closeSubPath ();
    // Inner void (hollow arch)
    p.startNewSubPath ( 4.f, 44.f); p.lineTo ( 4.f, 22.f);
    p.cubicTo         ( 4.f,  7.f, 11.f,  5.f, 22.f,  5.f);
    p.cubicTo         (33.f,  5.f, 40.f,  7.f, 40.f, 22.f);
    p.lineTo          (40.f, 44.f);
    // Stage
    p.addRoundedRectangle (10.f, 34.f, 24.f, 5.f, 1.f);
    // Three acoustic reflector panels
    p.addRoundedRectangle ( 7.f, 11.f, 4.f, 14.f, 1.f);
    p.addRoundedRectangle (20.f,  8.f, 4.f, 17.f, 1.f);
    p.addRoundedRectangle (33.f, 11.f, 4.f, 14.f, 1.f);
    return p;
}

// BRICASTI — 1U rack hardware unit
static juce::Path bricasti()
{
    juce::Path p;
    // Rack ears
    p.addRoundedRectangle ( 0.f,  4.f,  7.f, 28.f, 2.f);
    p.addRoundedRectangle (57.f,  4.f,  7.f, 28.f, 2.f);
    // Ear screw holes (subtractive — drawn as circles same colour as bg, handled in paint)
    // Front panel
    p.addRoundedRectangle ( 6.f,  7.f, 52.f, 22.f, 1.5f);
    // Display window (separate, darker fill handled in paint via winding)
    p.addRoundedRectangle ( 9.f, 10.f, 20.f, 16.f, 1.f);
    // Knobs × 3
    p.addEllipse (35.f, 12.f, 5.5f, 5.5f);
    p.addEllipse (43.f, 12.f, 5.5f, 5.5f);
    p.addEllipse (51.f, 12.f, 5.5f, 5.5f);
    // Small buttons row
    p.addRoundedRectangle (35.f, 21.f, 4.f, 3.5f, 0.8f);
    p.addRoundedRectangle (43.f, 21.f, 4.f, 3.5f, 0.8f);
    p.addRoundedRectangle (51.f, 21.f, 4.f, 3.5f, 0.8f);
    return p;
}

} // namespace InstrumentIcons



// ─── IRCategory ───────────────────────────────────────────────────────────────
struct IRCategory {
    juce::String name, displayName;
    juce::Path   icon;
    std::vector<int> presetIndices;
};

// ─── FolderRow ────────────────────────────────────────────────────────────────
class FolderRow : public juce::Component {
public:
    std::function<void()> onSelect;
    juce::String name;
    juce::Path   icon;
    int          presetCount = 0;
    bool         isSelected  = false;
    FolderRow(const juce::String& n, juce::Path ic, int cnt)
        : name(n), icon(std::move(ic)), presetCount(cnt) {}
    void paint(juce::Graphics&) override;
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void mouseUp   (const juce::MouseEvent& e) override { if (e.mouseWasClicked() && onSelect) onSelect(); }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FolderRow)
};

// ─── PresetRow ────────────────────────────────────────────────────────────────
class PresetRow : public juce::Component {
public:
    std::function<void()> onClick;
    juce::String name;
    int  globalIndex = 0;
    bool isActive    = false;
    PresetRow(const juce::String& n, int idx) : name(n), globalIndex(idx) {}
    void paint(juce::Graphics&) override;
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void mouseUp   (const juce::MouseEvent& e) override { if (e.mouseWasClicked() && onClick) onClick(); }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetRow)
};

// ─── UserPresetRow ────────────────────────────────────────────────────────────
class UserPresetRow : public juce::Component {
public:
    std::function<void()> onLoad, onDelete;
    juce::String presetName;
    bool isActive = false;
    UserPresetRow(const juce::String& n) : presetName(n) {}
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void mouseUp   (const juce::MouseEvent& e) override { if (e.mouseWasClicked() && onLoad) onLoad(); }
private:
    juce::TextButton deleteBtn {"×"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UserPresetRow)
};

// ─── EQDisplay ────────────────────────────────────────────────────────────────
class EQDisplay : public juce::Component, private juce::Timer {
public:
    explicit EQDisplay(juce::AudioProcessorValueTreeState& apvts);
    ~EQDisplay() override;
    void paint          (juce::Graphics&) override;
    void mouseDown      (const juce::MouseEvent&) override;
    void mouseDrag      (const juce::MouseEvent&) override;
    void mouseUp        (const juce::MouseEvent&) override;
    void mouseMove      (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseExit      (const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

private:
    struct BandState {
        float normX = 0.5f, normY = 0.5f;
        float q = 0.707f, qVisual = 0.707f;
    };
    std::array<BandState, 4> bands;
    juce::AudioProcessorValueTreeState& apvts;
    int dragging = -1, hoveredBand = -1;
    const float freqMin[4] = { 20.f,  200.f,  800.f, 4000.f };
    const float freqMax[4] = {500.f, 2000.f, 8000.f,20000.f };
    static constexpr float GAIN_RANGE = 12.f;

    float freqToNormX(int b, float f) const;
    float normXToFreq(int b, float nx) const;
    float gainToNormY(float g) const;
    float normYToGain(float ny) const;
    juce::Point<float> bandToPixel(int b) const;
    int   getBandAtPoint(juce::Point<float>) const;
    float getBandQ(int band) const;
    void  setBandQ(int band, float q);
    void  syncFromAPVTS();
    void  pushToAPVTS(int band);
    void  drawCurve(juce::Graphics&) const;
    void  drawQBand(juce::Graphics&, int b, juce::Point<float> pt) const;
    void  timerCallback() override;
    void  showFreqEntry(int band);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQDisplay)
};

// ─── VUMeter ──────────────────────────────────────────────────────────────────
class VUMeter : public juce::Component, private juce::Timer {
public:
    VUMeter();
    void setLevel(float rms);
    void paint(juce::Graphics&) override;
private:
    void timerCallback() override;
    std::atomic<float> rmsAtomic {0.f};
    float displayLevel = 0.f, peak = 0.f;
    int   peakHold = 0;
};

// ─── Main Editor ──────────────────────────────────────────────────────────────
class SordIRAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::FileDragAndDropTarget,
                                    private juce::Timer {
public:
    explicit SordIRAudioProcessorEditor(SordIRAudioProcessor&);
    ~SordIRAudioProcessorEditor() override;
    void paint  (juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag(const juce::StringArray&) override;
    void filesDropped(const juce::StringArray&, int, int) override;

private:
    void timerCallback() override;
    SordIRAudioProcessor& processor;
    SordLookAndFeel laf;

    juce::TextButton importButton;
    juce::TextButton tabIR, tabUser;
    bool showingUserTab = false;

    juce::Component irBrowser;
    juce::Viewport folderViewport, presetViewport;
    juce::Component folderList, presetList;
    juce::OwnedArray<FolderRow>  folderRows;
    juce::OwnedArray<PresetRow>  presetRows;
    std::vector<IRCategory> categories;
    int selectedCategoryIndex = 0;

    juce::Label currentIRLabel, irDurationLabel;

    juce::Component userPanel;
    juce::Viewport  userScrollView;
    juce::Component userListContainer;
    juce::OwnedArray<UserPresetRow> userPresetRows;
    juce::TextButton savePresetButton;
    juce::Label      savePresetField;
    juce::String     savePresetName {"My Preset"};
    juce::Label      noPresetsLabel;

    juce::Slider predelayKnob, sizeKnob, dryWetKnob;
    juce::Label  predelayLabel, sizeLabel, dryWetLabel;
    juce::Label  predelayVal,   sizeVal,   dryWetVal;

    juce::ToggleButton eqOnButton;
    EQDisplay          eqDisplay;
    // NOTE: NO eqKnob[] — EQ bands are dragged directly in the display

    juce::Slider gainKnob, outputFader;
    juce::Label  gainLabel, gainVal;
    VUMeter      vuMeter;
    juce::ToggleButton bypassButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment>
        predelayAttach, sizeAttach, dryWetAttach, gainAttach, faderAttach;
    std::unique_ptr<ButtonAttachment> eqOnAttach;

    static constexpr int HEADER_H   = 110;
    static constexpr int TABBAR_H   = 30;
    static constexpr int BROWSER_H  = 220;
    static constexpr int WAVEFORM_H = 60;
    static constexpr int CONTROLS_H = 96;
    static constexpr int FOOTER_H   = 32;
    static constexpr int OUT_W      = 86;
    static constexpr int FOLDER_W   = 180;

    void buildCategories();
    void buildFolderRows();
    void selectCategory(int index);
    void buildPresetRows(int categoryIndex);
    void selectIR(int globalIndex);
    void switchTab(bool userTab);
    void refreshUserPanel();
    void promptSavePreset();
    void paintWaveform(juce::Graphics&, juce::Rectangle<int>);
    void updateIRDurationLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SordIRAudioProcessorEditor)
};
