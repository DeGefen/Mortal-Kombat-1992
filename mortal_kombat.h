#pragma once
#include "SDL3/SDL.h"
#include "box2d/box2d.h"
#include "bagel.h"
#include "lib/box2d/src/body.h"
#include <string>

#include "SDL3_image/SDL_image.h"

namespace mortal_kombat
{
    class Mortal_kombat {
    public:
        Mortal_kombat();

        ~Mortal_kombat();

        void run();

    private:
        static constexpr int FPS = 60;
        static constexpr float BOX_SCALE = 10.0f;
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
        float x = 0.0f, y = 0.0f;
    };

    /// @brief Movement component holds the velocity of the entity.
    struct Movement {
        float vx = 0, vy = 0; // Velocity in x and y directions
    };

    /// @brief Texture component holds the SDL texture and its rectangle for rendering.
    struct Texture {
        SDL_Texture *tex = nullptr;
        SDL_FRect rect = {0, 0, 0, 0}; // Source rectangle for texture
    };

    /// @brief Texture component holds the SDL texture and its rectangle for rendering.
    struct Sound {
        SDL_AudioSpec spec = {};
        SDL_AudioDeviceID dev = 0;
        Uint8 *buf = nullptr;
        Uint32 len = 0;
    };

    /// @brief Collider component holds the physics body and shape.
    struct Collider {
        b2Body* body = nullptr; // Box2D body for collision
        b2Shape* shape = nullptr; // Shape used for collision detection
        bool isTrigger = false; // Whether the collider is a trigger
    };

    /// @brief Enum State holds the different states of the player.
    enum class State {
        IDLE, WALK_LEFT, WALK_RIGHT, CROUCH, WALK, JUMP,
        LOW_PUNCH, HIGH_PUNCH, LOW_KICK, HIGH_KICK,
        LOW_JUMP_KICK, HIGH_JUMP_KICK, JUMP_PUNCH,
        UPPERCUT, CROUCH_KICK, LOW_SWEEP_KICK,
        HIGH_SWEEP_KICK, BLOCK, SPECIAL_MOVE, CHEER, WON,
        KNOCKED_BACK, KNOCKED_UP, HIT
    };

    /// @brief Player_state component holds the state for the player.
    struct PlayerState {
        State state = State::IDLE;
        int busy_frames = 0; // Number of frames the player is busy
    };

    /// @brief Input component holds the input state for the player.
    enum class Input
    {
        UP, DOWN, LEFT, RIGHT,
        LOW_PUNCH, HIGH_PUNCH, LOW_KICK, HIGH_KICK, BLOCK,
    };

    /// @brief Inputs component holds the input, and input history for the player.
    struct Inputs {
        Input history[20] = {};
        int frame_number[20] = {};
        int index = 0;
    };

    enum class AttackType
    {
        LOW_PUNCH, HIGH_PUNCH, LOW_KICK, HIGH_KICK,
        LOW_JUMP_KICK, HIGH_JUMP_KICK, JUMP_PUNCH,
        UPPERCUT, CROUCH_KICK, LOW_SWEEP_KICK,
        HIGH_SWEEP_KICK, BLOCK
    };

    enum class SpecialAttackType
    {
        FIREBALL, TELEPORT, FLYING_KICK,
        SPINNING_BIRD_KICK, SCORPION_PUNCH,
        SUBZERO_FREEZE, SCORPION_TELEPORT,
        SUBZERO_SLIDE, SCORPION_CHAIN
    };

    /// @brief Attack component holds the attack type, damage, hitbox, and hitbox type.
    struct Attack {
        AttackType type;
        float damage = 0.0f;
        float hitbox = 0.0f;
        int hitbox_type = 0;
        float hitbox_size = 0.0f;
        float hitbox_duration = 0.0f;
    };

    /// @brief SpecialAttack component holds the special move type and inputs for the attack.
    struct SpecialAttack {
        SpecialAttackType type;
        float damage = 0.0f;
        float hitbox = 0.0f;
        int hitbox_type = 0;
        float hitbox_size = 0.0f;
        float hitbox_duration = 0.0f;
    };

    /// @brief Character component holds the character information of the player.
    struct Character {
        char name[10] = {};
        Input special_moves_input[3] = {};
    };

    /// @brief Health component holds the maximum and current health of the player.
    struct Health {
        float max_health = 100.0f;
        float current_health = 100.0f;
    };

    /// @brief Time component holds the time remaining in the match.
    struct Time {
        float time = 0.0f;
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

    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;

    /// @brief Creates Mario entity.
    /// @param x,y Position of the entity in the game world.
    /// @param renderer SDL renderer used to create the texture.
    /// @return A `bagel::ent_type` representing the Mario entity.
    inline bagel::ent_type createMario(float x, float y, SDL_Renderer* renderer) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(
                    Position{x, y},
                    MarioState{},
                    Movement{},
                    Physics{},
                    Texture{},
                    AnimatedImage{},
                    Collider{},
                    Input{},
                    State{}
                    );

        return entity.entity();
    }

    /// @brief Creates a player1's character (like Scorpion, Sub-Zero, etc.)
    inline bagel::ent_type createPlayer1(float x, float y, Character &character) {
        bagel::Entity entity = bagel::Entity::create();

        // Construct the texture path
        std::string texturePath = "res/" + std::string(character.name) + ".png";

        // Load the image as a surface
        SDL_Surface* surface = IMG_Load(texturePath.c_str());
        if (!surface) {
            SDL_Log("Failed to load image: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
            return {};
        }

        // Create a texture from the surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface); // Free the surface after creating the texture
        if (!texture) {
            SDL_Log("Failed to create texture: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
            return {};
        }

        // Add components to the entity
        entity.addAll(Position{x, y},
                      Movement{0, 0},
                      Collider{nullptr, nullptr},
                      Texture{texture, SDL_FRect{0, 0, 100, 100}},
                      PlayerState{State::IDLE, 0},
                      Inputs{},
                      character,
                      Health{100, 100});

        return entity.entity();
    }

    /// @brief Creates a player2's character (like Scorpion, Sub-Zero, etc.)
    inline bagel::ent_type createPlayer2(float x, float y, Character &character, SDL_Renderer* renderer) {
        bagel::Entity entity = bagel::Entity::create();

        // Construct the texture path
        std::string texturePath = "res/" + std::string(character.name) + ".png";

        // Load the image as a surface
        SDL_Surface* surface = IMG_Load(texturePath.c_str());
        if (!surface) {
            SDL_Log("Failed to load image: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
            return {};
        }

        // Create a texture from the surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface); // Free the surface after creating the texture
        if (!texture) {
            SDL_Log("Failed to create texture: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
            return {};
        }

        // Add components to the entity
        entity.addAll(Position{x, y},
                      Movement{0, 0},
                      Collider{nullptr, nullptr},
                      Texture{texture, SDL_FRect{0, 0, 100, 100}},
                      PlayerState{State::IDLE, 0},
                      Inputs{},
                      character,
                      Health{100, 100});

        return entity.entity();
    }

    /// @brief Creates a Attack entity (like a punch or kick)
    inline bagel::ent_type createAttack(float x, float y, AttackType type) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Collider{nullptr, nullptr},
                      Attack{type, 0.0f, 0.0f, 0, 0.0f, 0.0f});

        return entity.entity();
    }

    /// @brief Creates a Special Attack entity (like a punch or kick)
    bagel::ent_type createSpecialAttack(float x, float y, SpecialAttackType type) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Collider{nullptr, nullptr},
                      SpecialAttack{type, 0.0f, 0.0f, 0, 0.0f, 0.0f});

        return entity.entity();
    }

    /// @brief Creates a static platform/boundary
    bagel::ent_type createBoundary(float x, float y, float width, float height) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Collider{nullptr, nullptr});

        return entity.entity();
    }

    /// @brief Creates a game info entity
    bagel::ent_type createGameInfo(float initialTime) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Time{initialTime},
                      Score{0, 0, 0},
                      Position{0, 0},
                      Texture{nullptr, SDL_FRect{0, 0, 100, 50}});

        return entity.entity();
    }

    /// @brief Creates a background entity
    bagel::ent_type createBackground(SDL_Texture* texture) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{0, 0},
                      Texture{texture, SDL_FRect{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}});

        return entity.entity();
    }
}


