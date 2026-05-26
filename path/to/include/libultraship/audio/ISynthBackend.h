#pragma once
#include <cstdint>
#include <string>

namespace Ship {

class ISynthBackend {
public:
    virtual ~ISynthBackend() = default;

    // Called once at startup with the path to the SF2 file to load.
    // For the N64 passthrough backend this is a no-op.
    virtual void LoadSoundFont(const std::string& path) = 0;

    // MIDI-like note events. channel is 0-15.
    virtual void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) = 0;
    virtual void NoteOff(uint8_t channel, uint8_t note) = 0;

    // preset encodes both bank (high byte) and program (low byte).
    virtual void ProgramChange(uint8_t channel, uint16_t preset) = 0;

    // semitones is a signed float: +1.0 = one semitone up.
    // Range needed: approximately -12.0 to +12.0.
    virtual void PitchBend(uint8_t channel, float semitones) = 0;

    // Standard MIDI CC. value is 0-16383 (14-bit).
    virtual void ControlChange(uint8_t channel, uint8_t cc, uint16_t value) = 0;

    // Fill `out` with `frameCount` stereo interleaved float32 samples.
    // Called from the audio thread; must be real-time safe.
    virtual void Render(float* out, uint32_t frameCount) = 0;

    // Return true if this backend produces its own audio and the N64
    // synthesis pipeline should be skipped.
    virtual bool BypassN64Synthesis() const = 0;
};

} // namespace Ship
