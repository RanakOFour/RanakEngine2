#ifndef IOAUDIO_H
#define IOAUDIO_H

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "SDL3/SDL_audio.h"

namespace RanakEngine::Asset
{
    class AudioSample;
};

namespace RanakEngine::IO
{
    /**
     * @class Audio
     * @brief Manages per-asset SDL audio streams for playback, pause, and volume control.
     *
     * Each Asset::AudioSample that is currently playing is associated with a dedicated
     * SDL_AudioStream in m_activeStreams.  Multiple sounds can be active
     * simultaneously.  Streams are destroyed when Stop() or StopAllAudio() is called.
     *
     * Owned and accessed via IO::Manager::GetAudio().
     */
    class Audio
    {
        private:
        SDL_AudioDeviceID m_audioDevice; ///< The SDL audio device all streams are bound to.

        /** Map from asset weak_ptr -> active SDL_AudioStream* using owner_less comparison. */
        std::map<std::weak_ptr<Asset::AudioSample>,
                 SDL_AudioStream*,
                 std::owner_less<std::weak_ptr<Asset::AudioSample>>> m_activeStreams;

        public:
        Audio();
        ~Audio();

        /**
         * @brief Starts playing the audio asset.
         * @param _audio Weak pointer to the Asset::AudioSample to play.
         * @param _loop  When true the stream loops continuously until Stop() is called.
         * @return True on success, false if the asset is expired or stream creation fails.
         */
        bool Play(std::weak_ptr<Asset::AudioSample> _audio, bool _loop);
        /**
         * @brief Stops and destroys the active stream for the given asset.
         * @param _audio Weak pointer to the Asset::AudioSample to stop.
         */
        void Stop(std::weak_ptr<Asset::AudioSample> _audio);
        /**
         * @brief Pauses playback without destroying the stream.
         * @param _audio Weak pointer to the Asset::AudioSample to pause.
         */
        void Pause(std::weak_ptr<Asset::AudioSample> _audio);
        /**
         * @brief Resumes a previously paused stream.
         * @param _audio Weak pointer to the Asset::AudioSample to resume.
         */
        void Resume(std::weak_ptr<Asset::AudioSample> _audio);
        /**
         * @brief Adjusts the playback volume for the given asset's stream.
         * @param _audio  Weak pointer to the Asset::AudioSample.
         * @param _volume Linear volume factor (0.0 = silent, 1.0 = full, >1.0 = amplified).
         */
        void SetAudioVolume(std::weak_ptr<Asset::AudioSample> _audio, float _volume);
        /** @brief Stops and destroys all active audio streams. */
        void StopAllAudio();
    };
}

#endif