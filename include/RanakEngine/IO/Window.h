#ifndef WINDOW_H
#define WINDOW_H

#include <memory>

#include "RanakEngine/Math.h"

#include "GL/glew.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

namespace RanakEngine::IO
{
    /**
     * @class Window
     * @brief Wraps an SDL3 window with an associated OpenGL context and audio stream.
     *
     * Owns the SDL_Window and SDL_GLContext for the lifetime of the application.
     * Also holds the primary SDL_AudioStream used for mixing engine audio output.
     *
     * Typical usage: constructed once by IO::Manager::Init(), then accessed via
     * IO::Manager::GetWindow().
     */
    class Window
    {
        private:
        std::shared_ptr<SDL_Window>      m_sdlWindow;    ///< Managed SDL window handle.
        SDL_GLContext                     m_sdlglContext; ///< OpenGL context bound to this window.
        std::shared_ptr<SDL_AudioStream>  m_audioStream;  ///< Primary audio output stream.
        Vector4 m_clearColour = Vector4(0.4f, 0.4f, 0.8f, 1.0f); ///< RGBA colour used for glClear().
        Vector2 m_screenSize  = Vector2(1920.0f, 1080.0f);        ///< Current window dimensions in pixels.
        float   m_aspectRatio = 1920.0f / 1080.0f;                ///< width / height aspect ratio.

        public:
        /**
         * @brief Creates the SDL window, OpenGL context, and audio stream.
         * @param _screenSize Initial window size in pixels.
         * @param _title      Window title bar string.
         */
        Window(Vector2 _screenSize, std::string _title = "RanakEngine");
        ~Window();

        /** @brief Swaps the front and back OpenGL buffers (presents the frame). */
        void Swap();

        /** @brief Returns true when the mouse cursor is inside the window client area. */
        bool IsMouseInside();
        /**
         * @brief Enables or disables SDL relative-mouse mode for this window.
         * @param _value True to capture the mouse and suppress normal cursor movement.
         */
        void SetRelativeWindow(bool _value);
        /** @brief Returns true when relative-mouse mode is active. */
        bool GetRelativeWindow();

        /** @brief Resizes the window and updates the OpenGL viewport. @param _size New size in pixels. */
        void SetScreenSize(Vector2 _size);
        /** @brief Returns the current window size in pixels. */
        Vector2 GetScreenSize();

        void SetScreenPosition(Vector2 _position);
        Vector2 GetScreenPosition();

        /** @brief Sets the background clear colour. @param _colour RGBA colour (0.0–1.0). */
        void SetClearColour(Vector4 _colour);
        /** @brief Returns the current background clear colour. */
        Vector4 GetClearColour();

        /** @brief Returns a raw pointer to the underlying SDL_Window. */
        SDL_Window*      GetSDLWindow()  const { return m_sdlWindow.get(); }
        /** @brief Returns the SDL_GLContext associated with this window. */
        SDL_GLContext     GetGLContext()  const { return m_sdlglContext; }
        /** @brief Returns a raw pointer to the primary SDL_AudioStream. */
        SDL_AudioStream* GetAudioStream() const { return m_audioStream.get(); }
    };
}

#endif