#pragma once
#include "bagel.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace 
{
    /* =============== components =============== */
    /// @brief Components are the data structures that hold the state of the game objects.

    /// @brief Position component holds the x and y coordinates of an object.
    struct Position {
        float x, y;
    };

    /// @brief Velocity component holds the x and y velocity of an object.
    struct Velocity {
        float vx, vy;
    };

    /// @brief Texture component holds the SDL texture and its rectangle for rendering.
    struct Texture {
        SDL_Texture *tex;
        SDL_FRect rect;
    };

    /// @brief Texture component holds the SDL texture and its rectangle for rendering.
    struct Sound {
        SDL_AudioSpec spec;
        SDL_AudioDeviceID dev;
        Uint8 *buf;
        Uint32 len;
    };

    /// @brief Collision component holds the body and fixture IDs for collision detection.
    struct Collision {
        b2BodyId body;
        b2FixtureId fixture;
    };

    /// @brief Character_info component holds the character ID for the player.
    /// @brief This is used to identify which character the player is using.
    struct Character_info {
        int character_id;
    };

    enum class Action
    {
            IDLE, LEFT, RIGHT, UP, DOWN, WALK,
            JUMP, PUNCH, KICK, BLOCK, SPECIAL_MOVE
    };

    /// @brief State component holds the state for the player.
    /// @brief This is used to determine which actions the player is taking.
    /// @brief additionally, holds the previous actions of the player for combos and special moves.
    struct State {
        Action[10] actions;
        int curr_action;
        int delay;
        bool busy;
    };

    /// @brief Bullet component holds the damage and speed of the bullet.
    struct Bullet {
        int damage;
        int speed;
    };

    /// @brief Health component holds the maximum and current health of the player.
    struct Health {
        int max_health;
        int current_health;
    };

    /// @brief Time component holds the time remaining in the match.
    struct Time {
        int time;
    };

    /// @brief Score component holds the score of the player.
    struct Score {
        int round;
        int player1_score;
        int player2_score;
    };

    /* =============== systems =============== */
    /// @brief Systems are the logic that updates the game objects.

    /// @brief Movement system updates the position of the player based on the velocity.

    /* =============== entity =============== */
    /// @brief Entity is a unique identifier for each game object.



    /* =============== game =============== */

    class Mortal_kombat {
    public:
        Mortal_kombat();

        ~Mortal_kombat();

        void run();

    private:
        static constexpr int FPS = 60;
        static constexpr float BOX_SCALE = 10;
        static constexpr float TEX_SCALE = 0.5f;
        static constexpr SDL_FRect
        BALL_TEX = {404, 580, 76, 76};

        SDL_Texture *tex;
        SDL_Renderer *ren;
        SDL_Window *win;

        b2WorldId world;
        b2BodyId ballBody;
    };
}


