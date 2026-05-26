#ifndef AUDIOSAMPLE_H
#define AUDIOSAMPLE_H

#include "RanakEngine/Asset/AssetBase.h"
#include "SDL3/SDL_audio.h"

namespace RanakEngine::Asset
{
    /**
     * @class AudioSample
     * @brief Asset class for loading and managing audio files.
     * 
     * Loads audio data from disk and provides access to the audio buffer
     * and format specification for playback through IOManager.
     * 
     * @see Asset
     * @see IOManager
     */
    class AudioSample : public AssetBase
    {
        private:
        SDL_AudioSpec m_spec;   ///< Audio format specification
        uint8_t* m_buffer;      ///< Raw audio data buffer
        uint32_t m_bufferSize;  ///< Size of the audio buffer in bytes
        
        public:
        /**
         * @brief Constructs an Audio resource from a file.
         * 
         * @param _path Path to the audio file to load.
         */
        AudioSample(std::string _path);

        /**
         * @brief Destructs the Audio resource and frees the buffer.
         */
        ~AudioSample();
        
        /**
         * @brief Gets the audio format specification.
         * 
         * @return const SDL_AudioSpec& Reference to the audio spec.
         */
        const SDL_AudioSpec& GetSpec() const { return m_spec; };

        /**
         * @brief Gets the raw audio data buffer.
         * 
         * @return uint8_t* Pointer to the audio data.
         */
        uint8_t* GetBuffer() const { return m_buffer; };

        /**
         * @brief Gets the size of the audio buffer.
         * 
         * @return uint32_t Size in bytes.
         */
        uint32_t GetBufferSize() const { return m_bufferSize; };
    };
}

#endif