#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <memory>

#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/IO/KBInfo.h"
#include "RanakEngine/IO/MouseInfo.h"
#include "SDL3/SDL_dialog.h"
#include "SDL3/SDL_events.h"

//#include "SDL3/SDL.h"

namespace RanakEngine::Core
{
    class Manager;
    class Scene;
}

namespace RanakEngine::IO
{
    class Window;
    class Audio;

    /**
     * @class Manager
     * @brief Singleton that manages input polling, window lifecycle, audio routing, and file dialogs.
     *
     * IO::Manager is the engine's I/O layer.  Each frame it polls SDL events into
     * KBInfo / MouseInfo structures and exposes them via GetKeyDown*() and
     * GetMouseInfo().  It also owns the Window and Audio subsystems.
     *
     * File dialogs are handled asynchronously through SDL's dialog API; call
     * OpenFileDialog() and check the returned path (empty while the dialog is open).
     *
     * Access via IO::Manager::Instance() after creation with IO::Manager::Init().
     */
    class Manager
    {
        friend Core::Manager;
        private:
        static inline std::weak_ptr<IO::Manager> m_self; ///< Singleton weak self-reference.
        std::weak_ptr<Core::Manager>  m_core;   ///< Sibling core manager.
        std::shared_ptr<Window>       m_window; ///< Owned window/GL context.
        std::shared_ptr<Audio>        m_audio;  ///< Owned audio stream router.

        KBInfo    m_kbInfo,          m_lastFrameKBInfo;     ///< Current and previous keyboard state.
        MouseInfo m_mouseInfo,       m_lastFrameMouseInfo;  ///< Current and previous mouse state.

        bool m_quitSignalCalled; ///< True after an SDL_QUIT event is received.

        static inline bool        m_fileDialogEnded   = true; ///< False while a native file dialog is open.
        static inline std::string m_defaultFilePath   = "./"; ///< Default directory for file dialogs.

        static inline const SDL_DialogFileFilter m_FileDialogFilters[] = {
            { "Lua files",   "lua"   },
            { "Scene files", "scene" },
            { "All files",   "*"     }
        };

        /** @brief SDL callback invoked when the user selects a path in the file dialog. */
        static void SDLCALL FileDialogSelected(void* userdata, const char* const* filelist, int filter);

        void SetCore(std::weak_ptr<Core::Manager> _core);
        Manager(Vector2 _screenSize);

        public:
        ~Manager();

        /**
         * @brief Creates the singleton IO::Manager, Window, and Audio subsystems.
         * @param _screenSize Initial window size in pixels.
         * @return Shared pointer to the newly created manager.
         */
        static std::shared_ptr<IO::Manager> Init(Vector2 _screenSize);
        /** @brief Returns a weak pointer to the singleton instance. */
        static std::weak_ptr<IO::Manager> Instance();

        /**
         * @brief Opens a native file-open dialog and blocks until a selection is made.
         * @return The selected file path, or empty string if cancelled.
         */
        std::string OpenFileDialog();
        /**
         * @brief Opens a native file-save dialog and blocks until a selection is made.
         * @return The selected file path, or empty string if cancelled.
         */
        std::string SaveFileDialog();

        /**
         * @brief Polls all pending SDL events, updating keyboard and mouse state.
         * @return Vector of raw SDL_Event objects for further processing by callers.
         */
        std::vector<SDL_Event> UpdateInputs();

        /**
         * @brief Returns true if the key was pressed down this frame only (rising edge).
         * @param _key ASCII character of the key to query.
         */
        bool GetKeyDownThisFrame(char _key);
        /**
         * @brief Returns true while the key is held down (including repeats).
         * @param _key ASCII character of the key to query.
         */
        bool GetKeyDown(char _key);
        /** @brief Returns true if the engine window currently has focus. */
        bool WindowFocused();

        /** @brief Updates the window and OpenGL viewport to the new size. @param _size New size in pixels. */
        void SetScreenSize(Vector2 _size);
        /** @brief Returns the current window size in pixels. */
        Vector2 GetScreenSize();

        void SetScreenPosition(Vector2 _position);
        Vector2 GetScreenPosition();

        /** @brief Sets the OpenGL clear colour used at the start of each frame. @param _colour RGBA colour. */
        void SetClearColour(Vector4 _colour);
        /** @brief Returns the current clear colour. */
        Vector4 GetClearColour();

        /**
         * @brief Clears the framebuffer and renders all entities in _sceneToDraw.
         * @param _sceneToDraw Scene whose rules have already called Draw().
         */
        void Draw(std::shared_ptr<Core::Scene> _sceneToDraw);

        /** @brief Returns a weak pointer to the Window subsystem. */
        std::weak_ptr<Window> GetWindow();
        /** @brief Returns a weak pointer to the Audio subsystem. */
        std::weak_ptr<Audio> GetAudio();

        /** @brief Returns true when an SDL_QUIT event has been received. */
        bool GetQuitSignal() { return m_quitSignalCalled; };

        /** @brief Returns a reference to the current-frame mouse state. */
        MouseInfo& GetMouseInfo();
        /** @brief Returns a reference to the current-frame keyboard state. */
        KBInfo& GetKBInfo();
        /** @brief Returns a reference to the previous-frame mouse state. */
        MouseInfo& GetLastFrameMouseInfo() { return m_lastFrameMouseInfo; };
        /** @brief Returns a reference to the previous-frame keyboard state. */
        KBInfo& GetLastFrameKBInfo() { return m_lastFrameKBInfo; };
    };
}

#endif