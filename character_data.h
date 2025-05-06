#pragma once

namespace character {
    class CharacterData {
    public:
        struct CharacterSpriteInfo {
            int frameCount;
            int x;
            int y;
        };

        static constexpr CharacterSpriteInfo SUBZERO_SPRITE[]{
                {12, 32,   58}, // Stance
                {9,  3074, 58},  // Walk
                {5,  32,   580},  // Low Punch
                {10, 1436, 580}, // Low Punch Spree
                {5,  4010, 580},  // Body to Body Punch
                {5,  32,   1052},  // High Punch
                {10, 1436, 1052}, // High Punch Spree
                {5,  4010, 1052},  // Body to Body Kick
                {7,  32,   1574},  // Low Kick
                {8,  1904, 1574},  // Lowkick Sweep
                {10, 32,   2046}, // High Kick
                {8,  2606, 2046},  // Highkick Sweep
                {5,  32,   2568},  // Crouch
                {5,  1436, 2568},  // Uppercut
                {5,  2840, 2568},  // Crouch Kick
                {1,  32,   3090},  // Jump
                {5,  500,  3090},  // Jump Punch
                {3,  1904, 3090},  // Jump Highkick
                {3,  2840, 3090},  // Landing
                {1,  32,   3562},  // Jump Back
                {7,  500,  3562},  // Roll
                {5,  2372, 3562},  // Forward Jump Punch
                {5,  3776, 3562},  // Jump Lowkick
                {4,  32,   4084},  // Torso Hit
                {4,  1202, 4084},  // Head Hit
                {6,  2372, 4084},  // Kickback Torso Hit
                {3,  4010, 4084},  // Crouch Hit
                {5,  32,   4606},  // Fall
                {6,  1436, 4606},  // Uppercut Hit
                {6,  3074, 4606},  // Nuts Hit
                {6,  32,   5078},  // Fall Inplace
                {5,  1670, 5078},  // Getup
                {6,  32,   5600},  // Caught
                {7,  1670, 5600},  // Thrown
                {5,  32,   6122},  // Block
                {5,  1436, 6122},  // Crouch Block
                {4,  2372, 6122},  // Turn Right to Left
                {4,  3542, 6122},  // Turn Left to Right
                {10, 32,   6644}, // Special 1
                {3,  3542, 6644},  // Special 2
                {-1, -1,   -1},  // Special 3
                {7,  32,   7166},  // Giddy
                {0,  1904, 7166},  // Finish Him
                {7,  2606, 7166},  // Giddy Fall
                {4,  32,   7688},  // Win
        };

    };
}
//struct ActionPosition {
//    int row;
//    int col;
//};


//static constexpr ActionPosition ACTION_POSITIONS[] = {
//        {0,  0}, //Stance
//        {0,  1}, // Walk
//        {1,  0}, //High Punch
//        {1,  1}, //High Punch Spree
//        {1,  2}, //Body to Body Punch
//        {2,  0}, //Low Punch
//        {2,  1}, //Low Punch Spree
//        {2,  2}, //Body to Body Kick
//        {3,  0}, //Low Kick
//        {3,  1}, //Lowkick Sweep
//        {4,  0}, //High Kick
//        {4,  1}, //Highkick Sweep
//        {5,  0}, //Crouch
//        {5,  1}, //Uppercut
//        {5,  2}, //Crouch Kick
//        {6,  0}, //Jump
//        {6,  1}, //Jump Punch
//        {6,  2}, //Jump Highkick
//        {6,  3}, //Landing
//        {7,  0}, //Jump Back
//        {7,  1}, //Roll
//        {7,  2}, //Forward Jump Punch
//        {7,  3}, //Jump Lowkick
//        {8,  0}, //Torso Hit
//        {8,  1}, //Head Hit
//        {8,  2}, //Kickback Torso Hit
//        {8,  3}, //Crouch Hit
//        {9, 0}, //Fall
//        {9,  1}, //Uppercut Hit
//        {9,  2}, //Nuts Hit
//        {10, 0}, //Fall Inplace
//        {10, 1}, //Getup
//        {11, 0}, //Caught
//        {11, 1}, //Thrown
//        {12, 0}, //Block
//        {12, 1}, //Crouch Block
//        {12, 2}, //Turn Right to Left
//        {12, 3}, //Turn Left to Right
//        {13, 0}, //Special 1
//        {13, 1}, //Special 2
//        {13, 2}, //Special 3
//        {14, 0}, //Giddy
//        {14, 1}, //Finish Him
//        {14, 2}, //Giddy Fall
//        {15, 0}  //Win
//};