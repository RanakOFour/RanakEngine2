#ifndef MOUSEINFO_H
#define MOUSEINFO_H

#include "RanakEngine/Math.h"

namespace RanakEngine::IO
{
    /**
     * @struct MouseInfo
     * @brief Contains mouse input state and position information.
     */
    struct MouseInfo
    {
        Vector2 deltaPosition = Vector2(0.0f); ///< Mouse movement deltas since last frame
        
        float deltaScroll = 0.0f;

        Vector2 position = Vector2(0.0f); ///< Current mouse position in screen coordinates
        bool RMBDown = false; ///< Whether the right mouse button is currently pressed
        bool LMBDown = false; ///< Whether the left mouse button is currently pressed
        bool MMBDown = false; ///< Whether the middle mouse button is currently pressed
    };
}

#endif