#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo ""
echo "╔══════════════════════════════════════╗"
echo "║        Sord IR — Build Script        ║"
echo "╚══════════════════════════════════════╝"
echo ""

# ── Check dependencies ────────────────────────────────────────────────────────
if ! command -v cmake &>/dev/null; then
    echo "ERROR: cmake not found. Install with: brew install cmake"
    exit 1
fi

if ! command -v git &>/dev/null; then
    echo "ERROR: git not found."
    exit 1
fi

# ── Clone JUCE if missing ─────────────────────────────────────────────────────
if [ ! -d "JUCE" ]; then
    echo "Cloning JUCE..."
    git clone https://github.com/juce-framework/JUCE.git --depth=1 --branch 8.0.4
    echo "JUCE cloned."
fi

# ── Configure ─────────────────────────────────────────────────────────────────
echo "Configuring (Universal Binary: arm64 + x86_64)..."
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="12.0"

# ── Build ─────────────────────────────────────────────────────────────────────
echo ""
echo "Building..."
cmake --build build --config Release -j$(sysctl -n hw.ncpu)

# ── Sign (ad-hoc — no Apple Developer account needed) ─────────────────────────
VST3="build/SordIR_artefacts/Release/VST3/Sord IR.vst3"
AU="build/SordIR_artefacts/Release/AU/Sord IR.component"

echo ""
echo "Signing (ad-hoc)..."
codesign --force --deep --sign - "$VST3" 2>/dev/null && echo "  VST3 signed." || echo "  VST3 sign skipped."
codesign --force --deep --sign - "$AU"   2>/dev/null && echo "  AU signed."   || echo "  AU sign skipped."

# ── Verify architectures ──────────────────────────────────────────────────────
echo ""
echo "Architectures:"
lipo -info "$VST3/Contents/MacOS/Sord IR" 2>/dev/null || true

# ── Package ───────────────────────────────────────────────────────────────────
echo ""
echo "Packaging..."
mkdir -p dist
cp -r "$VST3" dist/
cp -r "$AU"   dist/
cp install.sh dist/
cp README.md  dist/ 2>/dev/null || true

cd dist
zip -r "../Sord_IR_v1.4.0_mac.zip" . -x "*.DS_Store"
cd ..

echo ""
echo "╔══════════════════════════════════════╗"
echo "║           Build complete!            ║"
echo "╠══════════════════════════════════════╣"
echo "║  dist/Sord IR.vst3                   ║"
echo "║  dist/Sord IR.component              ║"
echo "║  Sord_IR_v1.4.0_mac.zip              ║"
echo "╚══════════════════════════════════════╝"
echo ""
echo "To install locally, run:"
echo "  bash install.sh"
echo ""
