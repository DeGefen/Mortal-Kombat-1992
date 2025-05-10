#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

namespace character {
    class MK
    {
    public:
        MK();
        ~MK();

        void run();
    private:
        static constexpr int FPS = 60;
        static constexpr float BOX_SCALE = 10;
        static constexpr float TEX_SCALE = 0.5f;
        static constexpr SDL_FRect BALL_TEX = {404, 580, 76, 76};

        SDL_Texture* tex;
        SDL_Renderer* ren;
        SDL_Window* win;
    };

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

    class CharacterAnimations {
    public:
        static constexpr int CHAR_SQUARE_WIDTH = 230;
        static constexpr int CHAR_SQUARE_HEIGHT = 220;
        static constexpr int NEXT_FRAME_OFFSET = 4;
        static constexpr Uint8 COLOR_IGNORE_RED = 165;
        static constexpr Uint8 COLOR_IGNORE_GREEN = 231;
        static constexpr Uint8 COLOR_IGNORE_BLUE = 255;
        static constexpr int SHADOW_OFFSET = 8;
        static constexpr float SCALE_CHARACTER = 0.8f;

        enum class Action {
            STANCE = 0,
            WALK,
            LOW_PUNCH,
            LOW_PUNCH_SPREE,
            BODY_TO_BODY_PUNCH,
            HIGH_PUNCH,
            HIGH_PUNCH_SPREE,
            BODY_TO_BODY_KICK,
            LOW_KICK,
            LOWKICK_SWEEP,
            HIGH_KICK,
            HIGHKICK_SWEEP,
            CROUCH,
            UPPERCUT,
            CROUCH_KICK,
            JUMP,
            JUMP_PUNCH,
            JUMP_HIGHKICK,
            LANDING,
            JUMP_BACK,
            ROLL,
            FORWARD_JUMP_PUNCH,
            JUMP_LOWKICK,
            TORSO_HIT,
            HEAD_HIT,
            KICKBACK_TORSO_HIT,
            CROUCH_HIT,
            FALL,
            UPPERCUT_HIT,
            NUTS_HIT,
            FALL_INPLACE,
            GETUP,
            CAUGHT,
            THROWN,
            BLOCK,
            CROUCH_BLOCK,
            TURN_RIGHT_TO_LEFT,
            TURN_LEFT_TO_RIGHT,
            SPECIAL_1,
            SPECIAL_2,
            SPECIAL_3,
            GIDDY,
            FINISH_HIM,
            GIDDY_FALL,
            WIN
        };

        struct Character {
            CharacterData::CharacterSpriteInfo *sprites;
        };

        static constexpr Character SUBZERO = {
                const_cast<CharacterData::CharacterSpriteInfo *>(CharacterData::SUBZERO_SPRITE)
        };

        // Returns the sprite rectangle for a given action and frame
        static SDL_FRect getFrame(Character cha, Action action, int frame, bool shadow = false) {

            return {static_cast<float>(cha.sprites[static_cast<int>(action)].x + ((frame % cha.sprites[static_cast<int>(action)].frameCount) * (NEXT_FRAME_OFFSET + CHAR_SQUARE_WIDTH))),
                    static_cast<float>(cha.sprites[static_cast<int>(action)].y + (shadow ? (SHADOW_OFFSET + CHAR_SQUARE_HEIGHT) : 0)),
                    CHAR_SQUARE_WIDTH,
                    CHAR_SQUARE_HEIGHT};
        }

        struct Squence {
            Action action;
            int frame;
            bool right = false; // true = right, false = left
            bool walkBack = false; // true = walk back, false = walk forward
        };
    };

}