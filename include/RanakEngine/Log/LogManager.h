#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <vector>

#include "GL/glew.h"

namespace RanakEngine::IO
{
    class Manager;
}

namespace RanakEngine::Log
{
    struct MessageContent
    {
        enum Severity
        {
            DEBUG,
            NORMAL,
            WARNING,
            ERRORLOG
        };
        
        Severity severity = Severity::NORMAL;
        std::string contents = "";
    };

    class Manager
    {
        friend IO::Manager;
        private:
        /// Callback signature: severity, contents
        using LogCallback = std::function<void(MessageContent::Severity, const std::string&)>;

        /// Static weak pointer to the Logger singleton instance
        static inline std::weak_ptr<Log::Manager> m_self;
        
        std::queue<MessageContent> m_messageQueue; ///< Thread-safe queue of pending log messages
        std::thread m_watchThread; ///< Background thread for monitoring/processing logs
        std::mutex m_threadMutex; ///< Mutex protecting the message queue

        std::vector<LogCallback> m_listeners; ///< External listeners called on each log message
        std::mutex m_listenerMutex; ///< Protects m_listeners

        std::atomic<bool> m_running = false; ///< Whether the logging system is currently running

        /**
         * @brief Monitoring thread function.
         * 
         * Static function run in the watchThread that continuously monitors for
         * new messages and processes them.
         */
        static void Monitor();

        /**
         * @brief OpenGL debug message callback handler.
         * 
         * Registered with OpenGL to capture debug messages from the graphics system.
         * These messages are logged as debug-level messages.
         * 
         * @param _source The source of the debug message (API, window system, shader, etc.)
         * @param _type The type of debug message (error, deprecated behavior, etc.)
         * @param _id The ID number of the debug message
         * @param _severity The severity of the debug message (notification, low, medium, high)
         * @param _length The length of the debug message string
         * @param _message The debug message text
         * @param _userParam User-defined parameter (unused)
         */
        static void GLDebugMessageCallback(GLenum _source, GLenum _type, GLuint _id,
                             GLenum _severity, GLsizei _length, const GLchar* _message,
                             const void* _userParam);

        /**
         * @brief Constructs the Logger.
         * 
         * Private constructor for singleton pattern. Initializes the message queue
         * and creates the monitoring thread.
         */
        Manager();

        public:
        /**
         * @brief Destructs the Logger.
         * 
         * Stops the monitoring thread and cleans up resources.
         */
        ~Manager();

        /**
         * @brief Initializes the Logger singleton instance.
         * 
         * Creates and initializes the Logger. Should be called once at engine startup.
         * 
         * @return std::shared_ptr<Logger> Shared pointer to the newly created Logger instance.
         */
        static std::shared_ptr<Log::Manager> Init();

        /**
         * @brief Gets a weak pointer to the Logger singleton instance.
         * 
         * @return std::weak_ptr<Logger> Weak pointer to the Logger instance.
         */
        static std::weak_ptr<Log::Manager> Instance();

        /**
         * @brief Stops the logging system.
         * 
         * Signals the monitoring thread to stop and waits for it to finish.
         * Should be called during engine shutdown.
         */
        void Stop();

        /**
         * @brief Logs a message.
         * 
         * Thread-safe method to add a message to the logging queue.
         * Can be called from any thread.
         * 
         * @param _type The severity level of the message (see Message::Type).
         * @param _contents The message text.
         * 
         * @see Message::Type
         */
        static void LogMessage(int _type, std::string _contents);

        /**
         * @brief Checks if the message queue is empty.
         * 
         * @return bool True if no messages are pending, false otherwise.
         */
        bool Empty() { return m_messageQueue.empty(); };

        /**
         * @brief Registers an external listener that receives copies of all log messages.
         * @param _callback Function called with (severity, message) from the logging thread.
         */
        static void AddListener(LogCallback _callback);

        /**
         * @brief Checks if the logging system is running.
         * 
         * @return bool True if the logger is actively running, false otherwise.
         */
        bool Running() { return m_running; };
    };
}

#endif