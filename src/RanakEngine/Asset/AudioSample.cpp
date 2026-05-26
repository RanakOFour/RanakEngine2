#include "RanakEngine/Asset/AudioSample.h"
#include "RanakEngine/Log.h"

#include <stdexcept>

namespace RanakEngine::Asset
{
    AudioSample::AudioSample(std::string _path)
    : AssetBase(_path, AssetType::AUDIO)
    , m_buffer(nullptr)
    , m_bufferSize(0)
    {
        // Load audio file using SDL3
        // SDL_LoadWAV returns true on success, false on failure
        if (!SDL_LoadWAV(_path.c_str(), &m_spec, &m_buffer, &m_bufferSize))
        {
            Log::Error("Failed to load audio file: " + _path + " - " + std::string(SDL_GetError()));

            throw std::runtime_error("Failed to load audio: " + _path);
        }
        
        Log::Debug("Loaded audio: " + _path + 
            "Loaded audio: " + _path + 
            " | Channels: " + std::to_string(m_spec.channels) +
            " | Sample Rate: " + std::to_string(m_spec.freq) + " Hz" +
            " | Format: " + std::to_string(m_spec.format));
    }
    
    AudioSample::~AudioSample()
    {
        if (m_buffer != nullptr)
        {
            SDL_free(m_buffer);
            m_buffer = nullptr;
        }
    }
}