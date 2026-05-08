#include "RanakEngine/IO/IOManager.h"
#include "RanakEngine/IO/Audio.h"
#include "RanakEngine/IO/Window.h"

#include "RanakEngine/Log.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/Scene.h"

namespace RanakEngine::IO
{
    void IO::Manager::SetCore(std::weak_ptr<Core::Manager> _core)
    {
        m_core = _core;
    }

    IO::Manager::Manager(Vector2 _screenSize)
    : m_kbInfo()
    , m_mouseInfo()
    , m_lastFrameKBInfo()
    , m_lastFrameMouseInfo()
    , m_quitSignalCalled(false)
    {
        m_window = std::make_shared<Window>(_screenSize);
        m_audio = std::make_shared<Audio>();

        m_kbInfo.kbSize = 322;
    }

    IO::Manager::~Manager()
    {
        
    }

    std::shared_ptr<IO::Manager> IO::Manager::Init(Vector2 _screenSize)
    {
        if(m_self.lock() == nullptr)
        {
            std::shared_ptr<IO::Manager> l_toReturn;
            IO::Manager* l_manager = new IO::Manager(_screenSize);
            l_toReturn.reset(l_manager);
            
            l_toReturn->m_self = l_toReturn;

            glDebugMessageCallback(Log::Manager::GLDebugMessageCallback, 0);

            return l_toReturn;
        }

        return m_self.lock();
    };

    std::weak_ptr<IO::Manager> IO::Manager::Instance()
    {
        return m_self;
    }

    void SDLCALL IO::Manager::FileDialogSelected(void* userdata, const char* const* filelist, int filter)
    {
        if (!filelist) 
        {
            std::string l_error = SDL_GetError();
            Log::Error("An error occured: " + l_error);
        
            m_fileDialogEnded = true;
            return;
        }
        else if (!*filelist)
        {
            Log::Message( "The user did not select any file.\nMost likely the dialog was cancelled.");
            m_fileDialogEnded = true;
            return;
        }

        std::string& outFile = *(std::string*)userdata;
        outFile = *filelist;

        while (*filelist)
        {
            std::string l_fileList = *filelist;
            Log::Message( "Full path to selected file: " + l_fileList);
            filelist++;
        }

        if (filter < 0)
        {
            Log::Message( "The current platform does not support fetching\nthe selected filter, or the user did not select any filter.");
        }
        else if (filter < SDL_arraysize(m_FileDialogFilters)) 
        {
            std::string l_filterPattern = std::string(m_FileDialogFilters[filter].pattern);

            std::string l_info = "The filter selected by the user is " + l_filterPattern + 
                " (" + m_FileDialogFilters[filter].name + ").";
            Log::Message( l_info);
        }

        m_fileDialogEnded = true;
    }

    std::string IO::Manager::OpenFileDialog()
    {
        std::string l_filePath = "";
        SDL_ShowOpenFileDialog(FileDialogSelected, &l_filePath, m_window->GetSDLWindow(), m_FileDialogFilters, 3, m_defaultFilePath.c_str(), false);

        m_fileDialogEnded = false;

        // Force program to wait until file is selected,
        // because SDL file dialog is async
        while(!m_fileDialogEnded)
        {
            SDL_Delay(100);
            SDL_PumpEvents();
        }

        m_fileDialogEnded = false;
        
        return l_filePath;
    }

    std::string IO::Manager::SaveFileDialog()
    {
        std::string l_filePath = "";
        SDL_ShowSaveFileDialog(FileDialogSelected, &l_filePath, m_window->GetSDLWindow(), m_FileDialogFilters, 3, m_defaultFilePath.c_str());

        m_fileDialogEnded = false;
        // Force program to wait until file is selected,
        // because SDL file dialog is async
        while(!m_fileDialogEnded)
        {
            SDL_Delay(100);
            SDL_PumpEvents();
        }

        m_fileDialogEnded = false;
        
        return l_filePath;
    }

    std::vector<SDL_Event> IO::Manager::UpdateInputs()
    {
        m_lastFrameMouseInfo = m_mouseInfo;
        m_lastFrameKBInfo = m_kbInfo;

        static bool l_previousESCValue = false;
        SDL_Event l_event;

        bool l_resized = false;

        m_mouseInfo.deltaPosition.x = 0.0f;
        m_mouseInfo.deltaPosition.y = 0.0f;
        m_mouseInfo.deltaScroll = 0.0f;

        std::vector<SDL_Event> l_polledEvents;

        while (SDL_PollEvent(&l_event))
        {
            l_polledEvents.push_back(SDL_Event(l_event));
            switch (l_event.type)
            {
            case SDL_EVENT_QUIT:
                m_core.lock()->Stop();
                m_quitSignalCalled = true;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (l_event.button.button == SDL_BUTTON_LEFT)
                    m_mouseInfo.LMBDown = true;
                else if (l_event.button.button == SDL_BUTTON_MIDDLE)
                    m_mouseInfo.MMBDown = true;
                else
                    m_mouseInfo.RMBDown = true;
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (l_event.button.button == SDL_BUTTON_LEFT)
                    m_mouseInfo.LMBDown = false;
                else if (l_event.button.button == SDL_BUTTON_MIDDLE)
                    m_mouseInfo.MMBDown = false;
                else
                    m_mouseInfo.RMBDown = false;
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                m_mouseInfo.deltaScroll = -l_event.wheel.y;
                break;

            case SDL_EVENT_MOUSE_MOTION:
                m_mouseInfo.deltaPosition.x = l_event.motion.xrel;
                m_mouseInfo.deltaPosition.y = l_event.motion.yrel;
                break;

            case SDL_EVENT_KEY_DOWN:
                m_kbInfo.inputMap[l_event.key.scancode] = true;
                break;

            case SDL_EVENT_KEY_UP:
                m_kbInfo.inputMap[l_event.key.scancode] = false;
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                l_resized = true;
            }
        }

        if (l_resized)
        {
            int l_w, l_h;
            SDL_GetWindowSizeInPixels(m_window->GetSDLWindow(), &l_w, &l_h);
            m_window->SetScreenSize(Vector2(l_w, l_h));
        }

        SDL_GetMouseState(&m_mouseInfo.position.x, &m_mouseInfo.position.y);

        m_mouseInfo.position.y = m_window->GetScreenSize().y - m_mouseInfo.position.y;

        // Take mouse out of window focus on ESC+LCTRL
        if (m_kbInfo.inputMap[SDL_SCANCODE_ESCAPE] && !l_previousESCValue && m_kbInfo.inputMap[SDL_SCANCODE_LCTRL])
        {
            m_window->SetRelativeWindow(!m_window->GetRelativeWindow());
        }

        l_previousESCValue = m_kbInfo.inputMap[SDL_SCANCODE_ESCAPE];

        return l_polledEvents;
    }
    
    bool IO::Manager::GetKeyDownThisFrame(char _key)
    {
        int l_sdlCode = SDL_GetScancodeFromKey(_key, NULL);
        return m_kbInfo.inputMap[l_sdlCode] && !m_lastFrameKBInfo.inputMap[l_sdlCode];
    }

    bool IO::Manager::GetKeyDown(char _key)
    {
        int l_sdlCode = SDL_GetScancodeFromKey(_key, NULL);
        return m_kbInfo.inputMap[l_sdlCode];
    }

    bool IO::Manager::WindowFocused()
    {
        return m_window->IsMouseInside();
    }

    void IO::Manager::Draw(std::shared_ptr<Core::Scene> _sceneToDraw)
    {
        Vector4 l_clearColour = m_window->GetClearColour();
        
        glClearColor(l_clearColour.x, l_clearColour.y, l_clearColour.z, l_clearColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        _sceneToDraw->Draw();

        m_window->Swap();
    }

    void IO::Manager::SetCurrentWindowSize(Vector2 _size)
    {
        m_window->SetScreenSize(_size);
    }

    Vector2 IO::Manager::GetCurrentWindowSize()
    {
        return m_window->GetScreenSize();
    }

    void IO::Manager::SetCurrentWindowPosition(Vector2 _position)
    {
        m_window->SetScreenPosition(_position);
    }

    Vector2 IO::Manager::GetCurrentWindowPosition()
    {
        return m_window->GetScreenPosition();
    }

    void IO::Manager::SetCurrentClearColour(Vector4 _colour)
    {
        m_window->SetClearColour(_colour);
    }

    Vector4 IO::Manager::GetCurrentClearColour()
    {
        return m_window->GetClearColour();
    }

    void IO::Manager::MaximiseCurrentWindow()
    {
        m_window->MaximiseWindow();
    }

    bool IO::Manager::IsCurrentWindowMaximised()
    {
        return m_window->IsWindowMaximised();
    }

    std::weak_ptr<Window> IO::Manager::GetWindow()
    {
        return m_window;
    }

    std::weak_ptr<Audio> IO::Manager::GetAudio()
    {
        return m_audio;
    }

    MouseInfo& IO::Manager::GetMouseInfo()
    {
        return m_mouseInfo;
    }

    KBInfo& IO::Manager::GetKBInfo()
    {
        return m_kbInfo;
    }
}