#pragma once
#include "ISynthBackend.h"
#include "SynthBackend.h"
#include <memory>
#include <mutex>

namespace Ship {

class SynthBackendManager {
public:
    static SynthBackendManager& Instance();

    // Replace the active backend. Thread-safe.
    // Must NOT be called from the audio thread.
    // Passing nullptr reverts to N64SynthBackend.
    void SetBackend(std::shared_ptr<ISynthBackend> backend);

    // Get the active backend. Always returns non-null.
    std::shared_ptr<ISynthBackend> GetBackend();

private:
    SynthBackendManager();
    std::shared_ptr<ISynthBackend> mBackend;
    std::mutex mMutex;
};

} // namespace Ship