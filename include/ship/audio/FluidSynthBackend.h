#pragma once
#if ENABLE_FLUIDSYNTH

#include "ISynthBackend.h"
#include "AudioEvent.h"
#include <fluidsynth.h>
#include <vector>
#include <mutex>
#include <atomic>

namespace Ship {

class FluidSynthBackend final : public ISynthBackend {
public:
    // sampleRate must match the SDL output rate (typically 44100 or 48000).
    explicit FluidSynthBackend(double sampleRate);
    ~FluidSynthBackend() override;

    void LoadSoundFont(const std::string& path) override;
    void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void NoteOff(uint8_t channel, uint8_t note) override;
    void ProgramChange(uint8_t channel, uint16_t preset) override;
    void PitchBend(uint8_t channel, float semitones) override;
    void ControlChange(uint8_t channel, uint8_t cc, uint16_t value) override;
    void Render(float* out, uint32_t frameCount) override;
    bool BypassN64Synthesis() const override { return true; }

    // Pitch bend range in semitones sent to FluidSynth on channel init.
    // Must match what SynthEventTranslator uses. Default: 12 semitones.
    static constexpr float kPitchBendRangeSemitones = 12.0f;

private:
    void InitChannel(uint8_t channel);

    fluid_settings_t*  mSettings  = nullptr;
    fluid_synth_t*     mSynth     = nullptr;
    int                mSfontId   = FLUID_FAILED;
    double             mSampleRate;

    // Protects fluid_synth_* calls from concurrent access.
    // The audio thread calls Render(); the game thread calls NoteOn/Off/etc.
    std::mutex         mSynthMutex;

    // Which channels have had InitChannel() called.
    bool               mChannelInited[16] = {};
};

} // namespace Ship

#endif // #if ENABLE_FLUIDSYNTH
