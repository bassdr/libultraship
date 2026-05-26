#pragma once
#include <cstdint>

namespace Ship {

struct AudioEvent {
    // Sample-accurate timestamp: number of output samples elapsed
    // since the audio thread started. Used to order events correctly
    // when multiple events arrive in the same Render() call.
    uint64_t sampleTime;

    enum class Type : uint8_t {
        NoteOn,
        NoteOff,
        PitchBend,
        ControlChange,
        ProgramChange,
    };

    Type type;
    uint8_t channel; // 0-15

    union {
        struct { uint8_t note; uint8_t velocity; } noteOn;
        struct { uint8_t note;                   } noteOff;
        struct { float   semitones;              } pitchBend;    // signed, ±12 semitones
        struct { uint8_t cc; uint16_t value;     } controlChange;
        struct { uint16_t preset;                } programChange; // high=bank, low=program
    };
};

} // namespace Ship