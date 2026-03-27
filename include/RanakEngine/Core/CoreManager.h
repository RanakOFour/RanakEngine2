#ifndef COREMANAGER_H
#define COREMANAGER_H

#include "RanakEngine/Math/Vector3.h"

#include <memory>

namespace RanakEngine
{
    namespace IO
    {
        class Manager;
    };

    namespace Physics
    {
        class Manager;
    }

    namespace Asset
    {
        class LuaFile;
    }
}

namespace RanakEngine::Core
{
    class LuaContext;
    class Category;
    class CategoryFactory;
    class Scene;
    class Camera;

    /**
     * @class Manager
     * @brief Singleton that owns the core runtime subsystems and drives the game loop.
     *
     * CoreManager (aliased as Core::Manager) is the top-level orchestrator.  It
     * creates and holds the LuaContext, main Camera, and current Scene, and
     * provides the DeltaTime value computed each frame.
     *
     * Typical usage:
     * @code{.cpp}
     *   auto core = Core::Manager::Init(false);
     *   core->SetScene(myScene);
     *   core->Start();
     * @endcode
     *
     * The IOManager and PhysicsManager are siblings (weakly referenced) that are
     * set up by the top-level Initialise() helper in RanakEngine.h.
     */
    class Manager
    {
    private:
        static inline std::weak_ptr<Core::Manager> m_self; ///< Singleton weak self-reference.
        std::weak_ptr<IO::Manager>      m_ioManager;       ///< IO subsystem (input, window).
        std::weak_ptr<Physics::Manager> m_physicsManager;  ///< Physics subsystem.

        std::shared_ptr<LuaContext> m_luaContext;   ///< Owned Lua scripting runtime.
        std::shared_ptr<Camera>     m_mainCamera;   ///< Primary scene camera.

        std::shared_ptr<Scene> m_currentScene; ///< The scene currently being updated/rendered.
        bool  m_running   = false;  ///< True while the main loop is executing.
        bool  m_debug     = false;  ///< When true, enables additional debug output.
        float m_deltaTime = 0.0f;   ///< Time elapsed since the previous frame (seconds).
        float m_targetFPS = 60.0f;  ///< Desired frames-per-second used for frame capping.

        Manager(bool _debug);

    public:
        ~Manager();

        /**
         * @brief Creates the singleton Core::Manager and all dependent subsystems.
         * @param _debug When true enables verbose engine logging.
         * @return Shared pointer to the newly created manager.
         */
        static std::shared_ptr<Core::Manager> Init(bool _debug);
        /** @brief Returns a weak pointer to the singleton instance. */
        static std::weak_ptr<Core::Manager> Instance();

        /** @brief Starts the game/editor loop; blocks until Stop() is called. */
        void Start();
        /** @brief Signals the loop to exit after the current frame. */
        void Stop();

        /**
         * @brief Makes _scene the active scene; updates the Lua context reference.
         * @param _scene Shared pointer to the scene to activate.
         */
        void SetScene(std::shared_ptr<Scene> _scene);
        /** @brief Returns a weak pointer to the currently active scene. */
        std::weak_ptr<Scene> GetScene();

        /** @brief Returns the time between the previous two frames in seconds. */
        float DeltaTime();

        /** @brief Returns true when the engine was initialised with debug=true. */
        bool IsDebug();

        /**
         * @brief Converts a screen-space pixel position to a world-space 3-D point.
         * @param _screenPos Pixel coordinate.
         * @return World-space position at the near plane corresponding to _screenPos.
         */
        Vector3 ScreenToWorldPoint(Vector2 _screenPos);
        /**
         * @brief Converts a world-space 2-D position to a screen-space pixel coordinate.
         * @param _worldPoint World-space XY position.
         * @return Screen-space pixel coordinate.
         */
        Vector2 WorldPointToScreen(Vector2 _worldPoint);

        /** @brief Returns a weak pointer to the main Camera. */
        std::weak_ptr<Camera> GetCamera();
        /** @brief Returns the shared LuaContext owned by this manager. */
        std::shared_ptr<LuaContext> GetLuaContext();
    };
}

#endif