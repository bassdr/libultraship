#if ENABLE_FLUIDSYNTH
#include "ship/audio/FluidSynthBackend.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <algorithm>

namespace Ship {

FluidSynthBackend::FluidSynthBackend(double sampleRate)
    : mSampleRate(sampleRate) {

    mSettings = new_fluid_settings();
    fluid_settings_setnum(mSettings, "synth.sample-rate", sampleRate);
    fluid_settings_setint(mSettings, "synth.midi-channels", 16);
    // Disable FluidSynth's internal audio driver — we call write_float ourselves.
    fluid_settings_setstr(mSettings, "audio.driver", "file");
    // Use linear interpolation for a balance of quality and CPU cost.
    // Switch to FLUID_INTERP_4THORDER if quality needs to improve.
    fluid_settings_setint(mSettings, "synth.interpolation",
                          FLUID_INTERP_LINEAR);

    mSynth = new_fluid_synth(mSettings);
    if (!mSynth) {
        SPDLOG_ERROR("[FluidSynthBackend] Failed to create synth");
    }
}

FluidSynthBackend::~FluidSynthBackend() {
    if (mSynth)    delete_fluid_synth(mSynth);
    if (mSettings) delete_fluid_settings(mSettings);
}

void FluidSynthBackend::LoadSoundFont(const std::string& path) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth) return;
    mSfontId = fluid_synth_sfload(mSynth, path.c_str(), /*reset_presets=*/1);
    if (mSfontId == FLUID_FAILED) {
        SPDLOG_ERROR("[FluidSynthBackend] Failed to load SF2: {}", path);
    } else {
        SPDLOG_INFO("[FluidSynthBackend] Loaded SF2: {} (id={})", path, mSfontId);
    }
}

void FluidSynthBackend::InitChannel(uint8_t channel) {
    if (mChannelInited[channel]) return;
    mChannelInited[channel] = true;

    // Set pitch bend range via RPN 0 (MIDI spec).
    // CC 101 = RPN MSB, CC 100 = RPN LSB, CC 6 = Data Entry MSB (semitones),
    // CC 38 = Data Entry LSB (cents, 0 here).
    int ch = static_cast<int>(channel);
    fluid_synth_cc(mSynth, ch, 101, 0);   // RPN MSB = 0
    fluid_synth_cc(mSynth, ch, 100, 0);   // RPN LSB = 0 → select Pitch Bend Range
    fluid_synth_cc(mSynth, ch, 6,
                   static_cast<int>(kPitchBendRangeSemitones)); // range in semitones
    fluid_synth_cc(mSynth, ch, 38, 0);    // cents = 0
    // Null RPN so accidental CC6 messages don't change the range later.
    fluid_synth_cc(mSynth, ch, 101, 127);
    fluid_synth_cc(mSynth, ch, 100, 127);
}

void FluidSynthBackend::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth) return;
    InitChannel(channel);
    fluid_synth_noteon(mSynth, channel, note, velocity);
}

void FluidSynthBackend::NoteOff(uint8_t channel, uint8_t note) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth) return;
    fluid_synth_noteoff(mSynth, channel, note);
}

void FluidSynthBackend::ProgramChange(uint8_t channel, uint16_t preset) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth) return;
    InitChannel(channel);
    int bank    = (preset >> 8) & 0xFF;
    int program =  preset       & 0xFF;
    fluid_synth_bank_select(mSynth, channel, bank);
    fluid_synth_program_change(mSynth, channel, program);
}

void FluidSynthBackend::PitchBend(uint8_t channel, float semitones) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth) return;
    // Map semitones → 14-bit MIDI pitch bend (0–16383, centre=8192).
    float ratio = semitones / kPitchBendRangeSemitones;
    int val = static_cast<int>(ratio * 8192.0f) + 8192;
    val = std::clamp(val, 0, 16383);
    fluid_synth_pitch_bend(mSynth, channel, val);
}

void FluidSynthBackend::ControlChange(uint8_t channel, uint8_t cc, uint16_t value) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth) return;
    // FluidSynth CC takes a 7-bit value; pass high byte of 14-bit value.
    fluid_synth_cc(mSynth, channel, cc, (value >> 7) & 0x7F);
}

void FluidSynthBackend::Render(float* out, uint32_t frameCount) {
    std::lock_guard<std::mutex> lock(mSynthMutex);
    if (!mSynth || mSfontId == FLUID_FAILED) {
        // No soundfont loaded: output silence.
        std::memset(out, 0, frameCount * 2 * sizeof(float));
        return;
    }
    // fluid_synth_write_float writes interleaved stereo float32 to out.
    fluid_synth_write_float(mSynth,
                            static_cast<int>(frameCount),
                            out, 0, 2,   // left:  offset=0, stride=2
                            out, 1, 2);  // right: offset=1, stride=2
}

} // namespace Ship
#endif // #if ENABLE_FLUIDSYNTH
