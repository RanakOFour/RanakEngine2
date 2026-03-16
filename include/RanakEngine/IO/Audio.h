#ifndef IOAUDIO_H
#define IOAUDIO_H

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "SDL3/SDL_audio.h"

namespace RanakEngine::Asset
{
    class Audio;
};

namespace RanakEngine::IO
{
    class Audio
    {
        private:
        SDL_AudioDeviceID m_audioDevice;

        std::map<std::weak_ptr<Asset::Audio>,
                 SDL_AudioStream*,
                 std::owner_less<std::weak_ptr<Asset::Audio>>> m_activeStreams;

        public:
        Audio();
        ~Audio();

        bool Play(std::weak_ptr<Asset::Audio> _audio, bool _loop);
        void Stop(std::weak_ptr<Asset::Audio> _audio);
        void Pause(std::weak_ptr<Asset::Audio> _audio);
        void Resume(std::weak_ptr<Asset::Audio> _audio);
        void SetAudioVolume(std::weak_ptr<Asset::Audio> _audio, float _volume);
        void StopAllAudio();
    };
}

#endif