#include "ship/audio/Audio.h"

#include <stdexcept>
#include "ship/core/Context.h"
#include "ship/config/Config.h"
#include "ship/controller/controldeck/ControlDeck.h"

namespace Ship {

Audio::~Audio() {
    SPDLOG_TRACE("destruct audio");
}

void Audio::InitAudioPlayer() {
    switch (GetCurrentAudioBackend()) {
        case AudioBackend::SDL:
            mAudioPlayer = std::make_shared<SDLAudioPlayer>(this->mAudioSettings);
            break;
        default:
            mAudioPlayer = std::make_shared<NullAudioPlayer>(this->mAudioSettings);
            break;
    }

    if (mAudioPlayer && !mAudioPlayer->Init()) {
        // Failed to initialize system audio player.
        // Fallback to Null if the native system player does not work.
        SetCurrentAudioBackend(AudioBackend::NUL);
    }
}

void Audio::OnInit(const nlohmann::json& /*initArgs*/) {
    mAvailableAudioBackends = std::make_shared<std::vector<AudioBackend>>();
    mAvailableAudioBackends->push_back(AudioBackend::SDL);

    SetAudioChannels(GetSavedAudioChannelsSetting());
    SetCurrentAudioBackend(GetSavedAudioBackend());
}

std::shared_ptr<AudioPlayer> Audio::GetAudioPlayer() {
    return mAudioPlayer;
}

AudioBackend Audio::GetCurrentAudioBackend() {
    return mAudioBackend;
}

AudioBackend Audio::GetSavedAudioBackend() {
    auto config = GetConfig();
    std::string backendName = config->GetString("Window.AudioBackend");

    // The sdl player is SDL3 and the only backend; every retired name migrates to it.
    if (backendName == "pulse" || backendName == "wasapi" || backendName == "coreaudio" || backendName == "sdl3") {
        config->SetString("Window.AudioBackend", "sdl");
        config->Save();
        return AudioBackend::SDL;
    }

    if (backendName == "sdl") {
        return AudioBackend::SDL;
    }

    // Null is the runtime fallback when no device opens, never a saved choice: the picker is
    // disabled with one backend, so a persisted "null" would be an unrecoverable mute.
    if (backendName == "null") {
        config->SetString("Window.AudioBackend", "sdl");
        config->Save();
        return AudioBackend::SDL;
    }

    SPDLOG_TRACE("Could not find AudioBackend matching value from config file ({}). Returning default AudioBackend.",
                 backendName);

    return AudioBackend::SDL;
}

void Audio::SetCurrentAudioBackend(AudioBackend backend) {
    auto config = GetConfig();
    mAudioBackend = backend;

    switch (backend) {
        case AudioBackend::SDL:
            config->SetString("Window.AudioBackend", "sdl");
            break;
        case AudioBackend::NUL:
            config->SetString("Window.AudioBackend", "null");
            break;
        default:
            config->SetString("Window.AudioBackend", "");
    }
    config->Save();

    InitAudioPlayer();
}

std::shared_ptr<Config> Audio::GetConfig() const {
    if (!mConfig) {
        throw std::runtime_error("Audio requires Config dependency");
    }
    if (!mConfig->IsInitialized()) {
        throw std::runtime_error("Audio requires Config to be initialized");
    }
    return mConfig;
}

std::shared_ptr<std::vector<AudioBackend>> Audio::GetAvailableAudioBackends() {
    return mAvailableAudioBackends;
}

void Audio::SetAudioChannels(AudioChannelsSetting channels) {
    if (mAudioSettings.ChannelSetting != channels) {
        mAudioSettings.ChannelSetting = channels;
        // Reinitialize the existing audio player with the new channel configuration
        if (mAudioPlayer) {
            mAudioPlayer->SetAudioChannels(channels);
        }
    }
}

AudioChannelsSetting Audio::GetAudioChannels() const {
    return mAudioSettings.ChannelSetting;
}

AudioChannelsSetting Audio::GetSavedAudioChannelsSetting() {
    int32_t channelsSetting =
        GetConfig()->GetInt("CVars." CVAR_AUDIO_CHANNELS_SETTING, static_cast<int32_t>(AudioChannelsSetting::audioMax));
    switch (channelsSetting) {
        case AudioChannelsSetting::audioMatrix51:
            return AudioChannelsSetting::audioMatrix51;
        case AudioChannelsSetting::audioRaw51:
            return AudioChannelsSetting::audioRaw51;
        case AudioChannelsSetting::audioStereo:
        case AudioChannelsSetting::audioMax:
        default:
            return AudioChannelsSetting::audioStereo;
    }
}

} // namespace Ship
