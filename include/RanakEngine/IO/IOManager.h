#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <memory>

#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/IO/KBInfo.h"
#include "RanakEngine/IO/MouseInfo.h"

#include "SDL3/SDL.h"

namespace RanakEngine::Core
{
    class Manager;
    class Scene;
}

namespace RanakEngine::IO
{
    class Window;
    class Audio;
    class Manager
    {
        friend Core::Manager;
        private:
        static inline std::weak_ptr<IO::Manager> m_self;
        std::weak_ptr<Core::Manager> m_core;
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Audio> m_audio;
        
        KBInfo m_kbInfo, m_lastFrameKBInfo;
        
        MouseInfo m_mouseInfo, m_lastFrameMouseInfo;
        
        bool m_quitSignalCalled;

        static inline bool m_fileDialogEnded = true;
        static inline std::string m_defaultFilePath = "./";

        static inline const SDL_DialogFileFilter m_FileDialogFilters[] = {
            { "Lua files", "lua"},
            { "Scene files",  "scene" },
            { "All files",   "*" }
        };

        static void SDLCALL FileDialogSelected(void* userdata, const char* const* filelist, int filter);

        void SetCore(std::weak_ptr<Core::Manager> _core);
        Manager(Vector2 _screenSize);
        public:
        ~Manager();

        static std::shared_ptr<IO::Manager> Init(Vector2 _screenSize);
        static std::weak_ptr<IO::Manager> Instance();

        std::string OpenFileDialog();
        std::string SaveFileDialog();

        std::vector<SDL_Event> UpdateInputs();

        // Returns true if key was pressed down just this frame (and not previous frame)
        bool GetKeyDownThisFrame(char _key);

        // Returns true if key is pressed down
        bool GetKeyDown(char _key);
        bool WindowFocused();

        void SetScreenSize(Vector2 _size);
        Vector2 GetScreenSize();

        void SetClearColour(Vector4 _colour);
        Vector4 GetClearColour();

        void Draw(std::shared_ptr<Core::Scene> _sceneToDraw);

        std::weak_ptr<Window> GetWindow();
        std::weak_ptr<Audio> GetAudio();

        bool GetQuitSignal() { return m_quitSignalCalled; };

        MouseInfo& GetMouseInfo();
        KBInfo& GetKBInfo();
        MouseInfo& GetLastFrameMouseInfo() { return m_lastFrameMouseInfo; };
        KBInfo& GetLastFrameKBInfo() { return m_lastFrameKBInfo;  };
    };
}

#endif