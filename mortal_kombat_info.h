#pragma once

namespace mortal_kombat
{
    class CharacterData {
    public:
        struct CharacterSpriteInfo {
            int frameCount = 0;
            int x = 0, y = 0;
            int w = 230, h = 220;
        };

        static constexpr CharacterSpriteInfo SUBZERO_SPRITE[]{
                {12, 32,   58, 230, 220}, // Stance
                {9,  3074, 58, 230, 220},  // Walk Forwards
                {9,  3074, 58, 230, 220},  // Walk Backwards
                {5,  32,   580, 230, 220},  // Low Punch
                {10, 1436, 580, 230, 220}, // Low Punch Spree
                {5,  4010, 580, 230, 220},  // Body to Body Punch
                {5,  32,   1052, 230, 220},  // High Punch
                {10, 1436, 1052, 230, 220}, // High Punch Spree
                {5,  4010, 1052, 230, 220},  // Body to Body Kick
                {7,  32,   1574, 230, 220},  // Low Kick
                {8,  1904, 1574, 230, 220},  // Lowkick Sweep
                {10, 32,   2046, 230, 220}, // High Kick
                {8,  2606, 2046, 230, 220},  // Highkick Sweep
                {5,  32,   2568, 230, 220},  // Crouch
                {5,  1436, 2568, 230, 220},  // Uppercut
                {5,  2840, 2568, 230, 220},  // Crouch Kick
                {1,  32,   3090, 230, 220},  // Jump
                {5,  500,  3090, 230, 220},  // Jump Punch
                {3,  1904, 3090, 230, 220},  // Jump Highkick
                {3,  2840, 3090, 230, 220},  // Landing
                {1,  32,   3562, 230, 220},  // Jump Back
                {7,  500,  3562, 230, 220},  // Roll
                {5,  2372, 3562, 230, 220},  // Forward Jump Punch
                {5,  3776, 3562, 230, 220},  // Jump Lowkick
                {4,  32,   4084, 230, 220},  // Torso Hit
                {4,  1202, 4084, 230, 220},  // Head Hit
                {6,  2372, 4084, 230, 220},  // Kickback Torso Hit
                {3,  4010, 4084, 230, 220},  // Crouch Hit
                {5,  32,   4606, 230, 220},  // Fall
                {6,  1436, 4606, 230, 220},  // Uppercut Hit
                {6,  3074, 4606, 230, 220},  // Nuts Hit
                {6,  32,   5078, 230, 220},  // Fall Inplace
                {5,  1670, 5078, 230, 220},  // Getup
                {6,  32,   5600, 230, 220},  // Caught
                {7,  1670, 5600, 230, 220},  // Thrown
                {5,  32,   6122, 230, 220},  // Block
                {5,  1436, 6122, 230, 220},  // Crouch Block
                {4,  2372, 6122, 230, 220},  // Turn Right to Left
                {4,  3542, 6122, 230, 220},  // Turn Left to Right
                {10, 32,   6644, 230, 220}, // Special 1
                {3,  3542, 6644, 230, 220},  // Special 2
                {-1, -1,   -1, 230, 220},  // Special 3
                {7,  32,   7166, 230, 220},  // Giddy
                {0,  1904, 7166, 230, 220},  // Finish Him
                {7,  2606, 7166, 230, 220},  // Giddy Fall
                {4,  32,   7688, 230, 220},  // Win
        };

    };
}
