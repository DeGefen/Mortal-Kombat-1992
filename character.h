#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
#include "character_data.h"

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


    class CharacterAnimations {
    public:
        static constexpr int CHAR_SQUARE_WIDTH = 230;
        static constexpr int CHAR_SQUARE_HEIGHT = 220;
        static constexpr int NEXT_FRAME_OFFSET = 4;
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