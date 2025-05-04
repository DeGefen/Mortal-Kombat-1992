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

    /// @brief Input system - processes player inputs and updates input history
    void inputSystem() {
        // Mask for entities with input components
        auto inputMask = bagel::MaskBuilder().set<Inputs>().set<Player_state>().set<Character>().build();

        for (auto entity_id = bagel::Entity({0}); entity_id.entity().id <= bagel::World::maxId().id; ++entity_id.entity().id) {
            if (entity_id.test(inputMask)) {
                // Process keyboard/gamepad inputs
                // Update input history
                // Check for special move patterns
            }
        }
    }

    // Movement system - handles character movement based on input and state
    void movementSystem(bagel::Scene& scene) {
        auto movementMask = bagel::Mask::from<Position, Velocity, Player_state, Special_attack>();

        for (auto entity : scene.view(movementMask)) {
            // Update position based on velocity
            // Apply physics (gravity, momentum)
        }
    }

    // Combat system - handles attacks, collisions, and damage
    void combatSystem(bagel::Scene& scene) {
        auto attackerMask = bagel::Mask::from<Position, Player_state, Attack>();
        auto targetMask = bagel::Mask::from<Position, Hitbox, Health>();

        // Process basic attacks
        for (auto attacker : scene.view(attackerMask)) {
            for (auto target : scene.view(targetMask)) {
                // Check if attack hitbox collides with target
                // Apply damage calculations
            }
        }

        // Process special attacks
        auto specialAttackMask = bagel::Mask::from<Position, Special_attack>();
        for (auto attack : scene.view(specialAttackMask)) {
            for (auto target : scene.view(targetMask)) {
                // Check special attack collision
                // Apply special effects and damage
            }
        }
    }

    // Animation system - updates character animations based on state
    void animationSystem(bagel::Scene& scene) {
        auto animationMask = bagel::Mask::from<Texture, Player_state>();

        for (auto entity : scene.view(animationMask)) {
            // Update animation frames based on state
            // Handle sprite transitions
        }
    }

    // Physics system - handles physical interactions using Box2D
    void physicsSystem(bagel::Scene& scene, b2WorldId world) {
        auto physicsMask = bagel::Mask::from<Position, Hitbox>();

        for (auto entity : scene.view(physicsMask)) {
            // Sync Box2D positions with ECS positions
            // Handle collision responses
        }
    }

    // Health system - handles damage, knockbacks, and death
    void healthSystem(bagel::Scene& scene) {
        auto healthMask = bagel::Mask::from<Health, Player_state>();

        for (auto entity : scene.view(healthMask)) {
            // Check health status
            // Handle death animations
            // Trigger round end if needed
        }
    }

    // Game state system - handles round timing, victory conditions
    void gameStateSystem(bagel::Scene& scene) {
        auto timeMask = bagel::Mask::from<Time>();
        auto scoreMask = bagel::Mask::from<Score>();

        for (auto timeEntity : scene.view(timeMask)) {
            // Update match timer
            // Check for time-out condition
        }

        for (auto scoreEntity : scene.view(scoreMask)) {
            // Track round victories
            // Check for match end conditions
        }
    }

    // Render system - handles drawing all visible entities
    void renderSystem(bagel::Scene& scene, SDL_Renderer* renderer) {
        // Background rendering
        auto backgroundMask = bagel::Mask::from<Position, Texture>();
        auto backgroundOnly = bagel::Mask::from<Position, Texture>().exclude<Player_state, Health>();

        for (auto entity : scene.view(backgroundOnly)) {
            // Render background and static elements
        }

        // Character/entity rendering
        auto renderMask = bagel::Mask::from<Position, Texture>();
        auto characterMask = bagel::Mask::from<Position, Texture, Player_state>();

        for (auto entity : scene.view(characterMask)) {
            // Render characters with appropriate animations
        }

        // UI rendering
        auto uiMask = bagel::Mask::from<Score, Time>();

        for (auto entity : scene.view(uiMask)) {
            // Render health bars, timer, score
        }
    }

    // Sound system - handles game audio
    void soundSystem(bagel::Scene& scene) {
        auto soundMask = bagel::Mask::from<Sound>();

        for (auto entity : scene.view(soundMask)) {
            // Play appropriate sounds based on game events
        }
    }

    // Special move system - handles special attacks and their effects
    void specialMoveSystem(bagel::Scene& scene) {
        auto specialMoveMask = bagel::Mask::from<Inputs, Character, Player_state>();

        for (auto entity : scene.view(specialMoveMask)) {
            // Check for special move input patterns
            // Trigger special moves when detected
        }
    }


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


