#include "ship/audio/SynthBackendManager.h"

namespace Ship {

SynthBackendManager& SynthBackendManager::Instance() {
    static SynthBackendManager sInstance;
    return sInstance;
}

SynthBackendManager::SynthBackendManager()
    : mBackend(std::make_shared<SynthBackend>()) {}

void SynthBackendManager::SetBackend(std::shared_ptr<ISynthBackend> backend) {
    std::lock_guard<std::mutex> lock(mMutex);
    mBackend = backend ? backend : std::make_shared<SynthBackend>();
}

std::shared_ptr<ISynthBackend> SynthBackendManager::GetBackend() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mBackend;
}

} // namespace Ship