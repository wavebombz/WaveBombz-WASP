# WaveBombz WASP

A JUCE 8 / C++17 software synthesizer plugin for Windows, built as VST3 and Standalone.

## Visual direction
Dark graphite panels, yellow highlights, dimensional rotary controls and a compact modulation-routing section.

## Current prototype
- Three oscillator layers with selectable waveforms and mix controls
- Filter cutoff and resonance, FM, ring modulation and drive
- Amp ADSR, filter ADSR modulation and LFO controls
- Envelope/LFO routing controls and master level
- Momentary and integrated loudness estimates plus peak display
- GitHub Actions Windows build; VST3 zip uploaded as a workflow artifact

## Build
The Windows GitHub Actions workflow fetches JUCE 8.0.8, configures CMake, builds Release and uploads WaveBombz-WASP-Windows-VST3.zip as an artifact.

Open Actions → Build WaveBombz WASP → latest successful run → Artifacts to download the plugin.

## Meter note
The current LUFS display is an early K-weighted RMS-based estimate, not yet a certified EBU R128 implementation. Peak display is sample peak rather than true peak. The modulation routing UI is an initial prototype; per-knob drag-and-drop assignment and depth scaling are planned follow-up work.
