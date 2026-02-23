// Input event enums shared between termin libraries.

#pragma once

namespace tcbase {

// Mouse button enum.
enum class MouseButton : int {
    LEFT = 0,
    RIGHT = 1,
    MIDDLE = 2
};

// Action enum.
enum class Action : int {
    RELEASE = 0,
    PRESS = 1,
    REPEAT = 2
};

// Modifier key flags enum.
enum class Mods : int {
    SHIFT = 1,
    CTRL = 2,
    ALT = 4,
    SUPER = 8
};

} // namespace tcbase
