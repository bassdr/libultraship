#pragma once
#include "ISynthBackend.h"

namespace Ship {

class SynthBackend final : public ISynthBackend {
public:
    void LoadSoundFont(const std::string&) override {}
    void NoteOn(uint8_t, uint8_t, uint8_t) override {}
    void NoteOff(uint8_t, uint8_t) override {}
    void ProgramChange(uint8_t, uint16_t) override {}
    void PitchBend(uint8_t, float) override {}
    void ControlChange(uint8_t, uint8_t, uint16_t) override {}
    void Render(float*, uint32_t) override {}
    bool BypassN64Synthesis() const override { return false; }
};

} // namespace Ship