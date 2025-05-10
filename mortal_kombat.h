#pragma once
#include "SDL3/SDL.h"
#include "box2d/box2d.h"
#include "bagel.h"
#include "lib/box2d/src/body.h"
#include <string>

#include "SDL3_image/SDL_image.h"

namespace mortal_kombat
{
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

    /// @brief Player_state component holds the state for the player.
    struct PlayerState {
        State state = State::STANCE;
        bool direction = RIGHT;
        bool isJumping = false; // Whether the player is jumping
        int busy_frames = 0; // Number of frames the player is busy

        static constexpr bool LEFT = true;
        static constexpr bool RIGHT = false;
    };

    /// @brief Input component holds the input state for the player.
//    struct Input
//    {
//        bool up = false;
//        bool down = false;
//        bool left = false;
//        bool right = false;
//        bool low_punch = false;
//        bool high_punch = false;
//        bool low_kick = false;
//        bool high_kick = false;
//        bool block = false;
//    };

    using Input = Uint8;


        /// @brief Inputs component holds the input, and input history for the player.
    struct Inputs {
        static constexpr int MAX_HISTORY = 20;
        Input history[MAX_HISTORY] = {};
        Uint64 historyTime[MAX_HISTORY] = {};
        int index = 0;

        static constexpr Input UP = 1;
        static constexpr Input DOWN = 1 << 1;
        static constexpr Input LEFT = 1 << 2;
        static constexpr Input RIGHT = 1 << 3;
        static constexpr Input LOW_PUNCH = 1 << 4;
        static constexpr Input HIGH_PUNCH = 1 << 5;
        static constexpr Input LOW_KICK = 1 << 6;
        static constexpr Input HIGH_KICK = 1 << 7;
        static constexpr Input BLOCK = 0xD; // Use all bits expect Down
        static constexpr Input RESET = 0;

        static constexpr Input UPPERCUT = Inputs::DOWN | Inputs::HIGH_PUNCH;
        static constexpr Input CROUCH_KICK = Inputs::DOWN | Inputs::LOW_KICK;
        static constexpr Input LOW_SWEEP_KICK_RIGHT = Inputs::LEFT | Inputs::LOW_KICK;
        static constexpr Input LOW_SWEEP_KICK_LEFT = Inputs::RIGHT | Inputs::LOW_KICK;
        static constexpr Input HIGH_SWEEP_KICK_RIGHT = Inputs::LEFT | Inputs::HIGH_KICK;
        static constexpr Input HIGH_SWEEP_KICK_LEFT = Inputs::RIGHT | Inputs::HIGH_KICK;
        static constexpr Input CROUCH_BLOCK = Inputs::DOWN | Inputs::BLOCK;
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
        int playerNumber = 1;
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

    /* =============== Systems =============== */

    /// @brief Processes entities with the specified mask and applies a function to each entity.
    void processEntities(bagel::Mask mask, std::function<void(bagel::Entity&)> process);

    /// @brief Handles the movement of entities with Position and Movement components.

    /// @brief Updates the position of entities based on their movement components.
    class MovementSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Position>()
                .set<Movement>()
                .build();
    };

    /// @brief Renders entities with position and texture components to the screen.
    class RenderSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Position>()
                .set<Texture>()
                .build();
    };

    /// @brief Plays sounds for entities with sound components.
    class SoundSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Sound>()
                .build();
    };

    /// @brief Manages player-specific logic, such as state and character updates.
    class PlayerSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static constexpr Uint64 MAX_COMBO_TIME_MS = 500; // Maximum time between inputs to be considered a combo
        static constexpr Uint64 MAX_ACTION_TIME_MS = 500;

        static State CheckCombo(const mortal_kombat::Inputs& inputs, int currentIndex,
                                  Character character, Uint64 maxTimeMs);

        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Input>()
                .set<PlayerState>()
                .set<Character>()
                .build();
    };

    /// @brief Handles collision detection and response for entities with colliders.
    class CollisionSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Collider>()
                .set<Position>()
                .build();
    };

    /// @brief Manages match-related logic, such as health updates and round progression.
    class MatchSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Health>()
                .build();
    };

    /// @brief Determines the winner of the match based on scores.
    class WinSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Score>()
                .build();
    };

    /// @brief Updates the game clock and manages time-related logic.
    class ClockSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Time>()
                .build();
    };

    /// @brief Processes player inputs and updates input history.
    class InputSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Inputs>()
                .build();
    };

    /// @brief Handles attack logic, such as applying damage and managing hitboxes.
    class AttackSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Attack>()
                .set<Health>()
                .build();
    };

    /// @brief Manages special attack logic, including damage and hitbox effects.
    class SpecialAttackSystem final: bagel::NoInstance
    {
    public:
        static void run();
    private:
        static inline bagel::Mask mask = bagel::MaskBuilder()
                .set<Attack>()
                .set<Health>()
                .build();
    };

    /* =============== Entities =============== */
    /// @brief Entity is a unique identifier for each game object.

    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;

    /// @brief Creates a player's character (like Scorpion, Sub-Zero, etc.)
    /// @param x,y Position of the entity in the game world.
    /// @param character Character data for the player.
    /// @param renderer SDL renderer used to create the texture.
    /// @return A `bagel::ent_type` representing the player entity.
    inline bagel::ent_type createPlayer(float x, float y, Character &character, SDL_Renderer* renderer) {
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
                      PlayerState{State::STANCE, 0},
                      Inputs{},
                      character,
                      Health{100, 100});

        return entity.entity();
    }

    /// @brief Creates an Attack entity (like a punch or kick).
    /// @param x,y Position of the entity in the game world.
    /// @param type Type of the attack.
    /// @return A `bagel::ent_type` representing the attack entity.
    inline bagel::ent_type createAttack(float x, float y, AttackType type) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Collider{nullptr, nullptr},
                      Attack{type, 0.0f, 0.0f, 0, 0.0f, 0.0f});

        return entity.entity();
    }

    /// @brief Creates a special attack entity.
    /// @param x,y Position of the entity in the game world.
    /// @param type Type of the special attack.
    /// @return A `bagel::ent_type` representing the special attack entity.
    bagel::ent_type createSpecialAttack(float x, float y, SpecialAttackType type) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Collider{nullptr, nullptr},
                      SpecialAttack{type, 0.0f, 0.0f, 0, 0.0f, 0.0f});

        return entity.entity();
    }

    /// @brief Creates a static platform/boundary.
    /// @param x,y Position of the boundary in the game world.
    /// @param width,height Dimensions of the boundary.
    /// @return A `bagel::ent_type` representing the boundary entity.
    bagel::ent_type createBoundary(float x, float y, float width, float height) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{x, y},
                      Collider{nullptr, nullptr});

        return entity.entity();
    }

    /// @brief Creates a game info entity.
    /// @param initialTime Initial time for the game.
    /// @return A `bagel::ent_type` representing the game info entity.
    bagel::ent_type createGameInfo(float initialTime) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Time{initialTime},
                      Score{0, 0, 0},
                      Position{0, 0},
                      Texture{nullptr, SDL_FRect{0, 0, 100, 50}});

        return entity.entity();
    }

    /// @brief Creates a background entity.
    /// @param texture SDL texture for the background.
    /// @return A `bagel::ent_type` representing the background entity.
    bagel::ent_type createBackground(SDL_Texture* texture) {
        bagel::Entity entity = bagel::Entity::create();

        entity.addAll(Position{0, 0},
                      Texture{texture, SDL_FRect{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}});

        return entity.entity();
    }
}


