#pragma once
#include "bagel.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace mortal_kombat
{
    class Mortal_kombat {
    public:
        Mortal_kombat();

        ~Mortal_kombat();

        void run();

    private:
        static constexpr int FPS = 60;
        static constexpr int WINDOW_WIDTH = 800;
        static constexpr int WINDOW_HEIGHT = 600;
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

    /* =============== components =============== */
    /// @brief Components are the data structures that hold the state of the game objects.

    /// @brief Position component holds the x and y coordinates of an object.
    struct Position {
        float x, y;
    };

    /// @brief Velocity component holds the x and y velocity of an object.
    struct Velocity {
        float vx = 0, vy = 0;
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

    /// @brief Hitbox component holds the body and fixture IDs for Hitbox detection.
    struct Hitbox {
        b2BodyId body;
        b2FixtureId fixture;
    };

    /// @brief Enum State holds the different states of the player.
    enum class State
    {
            IDLE, WALK_LEFT, WALK_RIGHT, CROUCH, WALK, JUMP,
            LOW_PUNCH, HIGH_PUNCH, LOW_KICK, HIGH_KICK,
            LOW_JUMP_KICK, HIGH_JUMP_KICK, JUMP_PUNCH,
            UPPERCUT, CROUCH_KICK, LOW_SWEEP_KICK,
            HIGH_SWEEP_KICK BLOCK, SPECIAL_MOVE, CHEER, WON,
            KNOCKED_BACK, KNOCKED_UP, HIT,
    };

    /// @brief Player_state component holds the state for the player.
    struct Player_state {
        State state;
        int busy_frames;
    };

    /// @brief Input component holds the input state for the player.
    enum class Input
    {
        UP, DOWN, LEFT, RIGHT,
        LOW_PUNCH, HIGH_PUNCH, LOW_KICK, HIGH_KICK, BLOCK,
    };

    /// @brief Input component holds the input, and input history for the player.
    struct Inputs {
        input[20] history = {0};
        int[20] frame_number = {0};
        int index = 0;
    };

    /// @brief Attack component holds the attack type, damage, hitbox, and hitbox type.
    struct Attack {
        int type;
        int damage;
        int hitbox;
        int hitbox_type;
        int hitbox_size;
        int hitbox_duration;
    };

    /// @brief Special_move component holds the special move type, damage, hitbox, and hitbox type.
    struct Special_attack {
        int type;
        input[5] input;
        int damage;
        int hitbox;
        int hitbox_type;
        int hitbox_size;
        int hitbox_duration;
        bool bullet;
    };

    /// @brief Character component holds the character information of the player.
    struct Character {
        char[10] name;
        Special_move[3] special_moves;
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
        int round = 0;
        int player1_score = 0;
        int player2_score = 0;
    };

    /* =============== systems =============== */

    /// @brief Destroys entities with the Lifetime component when their lifetime expires.
    class LifetimeSystem final: bagel::NoInstance
    {
    public:
        static void run() {
            for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
                bagel::Entity entity{e};
                if (entity.test(mask)) {
                    // Process the entity
                }
            }
        }
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Lifetime>()
                .build();
    };

    /* =============== entities =============== */
    /// @brief Entity is a unique identifier for each game object.

    /// @brief Creates a player1's character (like Scorpion, Sub-Zero, etc.)
    bagel::Entity createPlayer1(float x, float y, Character &character) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Velocity{0, 0},
                      Hitbox{nullptr, nullptr},
                      Texture{"res/" + character.name + ".png", SDL_FRect{0, 0, 100, 100}},
                      Player_state{0, 0},
                      Inputs{},
                      character,
                      Health{100, 100});

        return entity;
    }

    /// @brief Creates a player2's character (like Scorpion, Sub-Zero, etc.)
    bagel::Entity createPlayer2(float x, float y, Character &character) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Velocity{0, 0},
                      Hitbox{nullptr, nullptr},
                      Texture{"res/" + character.name + ".png", SDL_FRect{0, 0, 100, 100}},
                      Player_state{0, 0},
                      Inputs{},
                      character,
                      Health{100, 100});

        return entity;
    }

    /// @brief Creates a Attack entity (like a punch or kick)
    bagel::Entity createAttack(float x, float y, Attack attack) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Hitbox{nullptr, nullptr},
                      attack);

        return entity;
    }

    /// @brief Creates a Special Attack entity (like a punch or kick)
    bagel::Entity createSpecialAttack(float x, float y, Special_attack special_attack) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Hitbox{nullptr, nullptr},
                      special_attack);

        return entity;
    }

    /// @brief Creates a static platform/boundary
    bagel::Entity createBoundary(float x, float y, float width, float height) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Hitbox{nullptr, nullptr});

        return entity;
    }

    /// @brief Creates a game info entity
    bagel::Entity gameInfo(int initialTime) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Time{initialTime},
                      Score{0, 0, 0},
                      Position{0, 0},
                      Texture{nullptr, SDL_FRect{0, 0, 100, 50}});

        return entity;
    }

    /// @brief Creates a background entity
    bagel::Entity createBackground(SDL_Texture* texture) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{0, 0},
                      Texture{texture, SDL_FRect{0, 0, Mortal_kombat::WINDOW_WIDTH,
                                                 Mortal_kombat::WINDOW_HEIGHT}});

        return entity;
    }
}


