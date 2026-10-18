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


## Effects, play modes and soundbanks
The editor includes a dedicated FX panel with switches and controls for compressor, distortion, EQ, delay, reverb, flanger, phaser and a first-pass vocoder-style modulation effect. Mono, legato selection and portamento controls are present. Presets can be saved and opened as .wasp files; use OPEN BANK to load a folder containing .wasp presets.

The Factory soundbank is under `Soundbanks/Factory` and includes eight original WASP-inspired starter presets: Acid Bite, Neon Pulse, Brass Stab, Deep Sub, Wasp Lead, Tech Pluck, Dark Drone and Retro Bass. The Windows build artifact packages this folder alongside the VST3.

**Prototype limitations:** the vocoder effect is currently only a simple modulation placeholder, not a full external modulator/carrier vocoder. The FX chain and mono/legato behavior need compilation and audio testing before release. LUFS figures are estimates and not yet validated against EBU R128.
