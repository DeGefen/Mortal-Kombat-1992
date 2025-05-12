#pragma once
#include "mortal_kombat_info.h"
#include <functional>
#include "SDL3/SDL.h"
#include "box2d/box2d.h"
#include "bagel.h"
#include "lib/box2d/src/body.h"
#include <string>


#include "SDL3_image/SDL_image.h"

namespace mortal_kombat
{
    class MK
    {
    public:
        MK();
        ~MK();
        void run();

    private:
        static constexpr Uint8 COLOR_IGNORE_RED = 165;
        static constexpr Uint8 COLOR_IGNORE_GREEN = 231;
        static constexpr Uint8 COLOR_IGNORE_BLUE = 255;
        static constexpr int WINDOW_WIDTH = 800;
        static constexpr int WINDOW_HEIGHT = 600;
        static constexpr int CHAR_SQUARE_WIDTH = 230;
        static constexpr int CHAR_SQUARE_HEIGHT = 220;
        static constexpr int NEXT_FRAME_OFFSET = 4;
        static constexpr int SHADOW_OFFSET = 8;
        static constexpr float SCALE_CHARACTER = 0.8f;

        SDL_Renderer* ren{};
        SDL_Window* win{};

        /// @brief Enum State holds the different states of the player.
        enum class State {
            STANCE = 0,
            WALK_FORWARDS,
            WALK_BACKWARDS,
            LOW_PUNCH,
            LOW_PUNCH_SPREE,
            BODY_TO_BODY_PUNCH,
            HIGH_PUNCH,
            HIGH_PUNCH_SPREE,
            BODY_TO_BODY_KICK,
            LOW_KICK,
            LOW_SWEEP_KICK,
            HIGH_KICK,
            HIGH_SWEEP_KICK,
            CROUCH,
            UPPERCUT,
            CROUCH_KICK,
            JUMP,
            JUMP_PUNCH,
            JUMP_HIGH_KICK,
            LANDING,
            JUMP_BACK,
            ROLL,
            FORWARD_JUMP_PUNCH,
            JUMP_LOW_KICK,
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

        /// @brief Enum AttackType hold the different attacks types.
        enum class AttackType
        {
            LOW_PUNCH, HIGH_PUNCH, LOW_KICK, HIGH_KICK,
            LOW_JUMP_KICK, HIGH_JUMP_KICK, JUMP_PUNCH,
            UPPERCUT, CROUCH_KICK, LOW_SWEEP_KICK,
            HIGH_SWEEP_KICK, BLOCK
        };

        /// @brief Enum AttackType hold the different special attacks types.
        enum class SpecialAttackType
        {
            FIREBALL, TELEPORT, FLYING_KICK,
            SPINNING_BIRD_KICK, SCORPION_PUNCH,
            SUBZERO_FREEZE, SCORPION_TELEPORT,
            SUBZERO_SLIDE, SCORPION_CHAIN
        };

        /// @brief Input variable for player's key inputs
        using Input = Uint16;

        /* =============== components =============== */
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
            SDL_FRect srcRect = {0, 0, 0, 0}; // Source rectangle for texture
            SDL_FRect rect = {0, 0, 0, 0}; // Destination rectangle for rendering
        };

        /// @brief Collider component holds the physics body and shape.
        struct Collider {
            b2Body* body = nullptr; // Box2D body for collision
            b2Shape* shape = nullptr; // Shape used for collision detection
            bool isTrigger = false; // Whether the collider is a trigger
        };

        /// @brief Player_state component holds the state for the player.
        struct PlayerState {
            State state = State::STANCE;
            State prevState = State::STANCE;
            bool direction = RIGHT;
            bool isJumping = false; // Whether the player is jumping
            int busyFrames = 0; // Total frames spent in the current state
            int currFrame = 0; //  Frames spent in the current state
            bool busy = false; // Whether the player is busy

            static constexpr bool LEFT = true;
            static constexpr bool RIGHT = false;
        };

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
            static constexpr Input BLOCK = 1 << 8;
            static constexpr Input DIRECTION_RIGHT = 1 << 9;
            static constexpr Input DIRECTION_LEFT = 1 << 10;
            static constexpr Input JUMPING = 1 << 11;
            static constexpr Input RESET = 0;

            static constexpr Input UPPERCUT = Inputs::DOWN | Inputs::HIGH_PUNCH;
            static constexpr Input CROUCH_KICK = Inputs::DOWN | Inputs::LOW_KICK;
            static constexpr Input LOW_SWEEP_KICK_RIGHT = Inputs::LEFT | Inputs::LOW_KICK | Inputs::DIRECTION_RIGHT;
            static constexpr Input LOW_SWEEP_KICK_LEFT = Inputs::RIGHT | Inputs::LOW_KICK | Inputs::DIRECTION_LEFT;
            static constexpr Input HIGH_SWEEP_KICK_RIGHT = Inputs::LEFT | Inputs::HIGH_KICK | Inputs::DIRECTION_RIGHT;
            static constexpr Input HIGH_SWEEP_KICK_LEFT = Inputs::RIGHT | Inputs::HIGH_KICK | Inputs::DIRECTION_LEFT;
            static constexpr Input CROUCH_BLOCK = Inputs::DOWN | Inputs::BLOCK;
            static constexpr Input WALK_FORWARDS_RIGHT = Inputs::RIGHT | Inputs::DIRECTION_RIGHT;
            static constexpr Input WALK_FORWARDS_LEFT = Inputs::LEFT | Inputs::DIRECTION_LEFT;
            static constexpr Input WALK_BACKWARDS_RIGHT = Inputs::LEFT | Inputs::DIRECTION_RIGHT;
            static constexpr Input WALK_BACKWARDS_LEFT = Inputs::RIGHT | Inputs::DIRECTION_LEFT;
            static constexpr Input JUMP_PUNCH = Inputs::LOW_PUNCH | Inputs::JUMPING;
            static constexpr Input JUMP_HIGH_KICK = Inputs::HIGH_KICK | Inputs::JUMPING;
            static constexpr Input JUMP_LOW_KICK = Inputs::LOW_KICK | Inputs::JUMPING;

            bool test(Input input) const
            {
                return (history[index] & input) == input;
            }
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
            CharacterData::CharacterSpriteInfo *sprite{};
            Input special_moves_input[3] = {};
            int playerNumber = 1;

            static constexpr float WALK_SPEED_BACKWARDS = 4;
            static constexpr float WALK_SPEED_FORWARDS = 5;
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
        static void processEntities(bagel::Mask mask, std::function<void(bagel::Entity&)> process);

        /// @brief Updates the position of entities based on their movement components.
        void MovementSystem();

        /// @brief Renders entities with position and texture components to the screen.
        void RenderSystem();

        static SDL_FRect getCharacterFrame(const Character& character, State action,
                                            int frame, bool shadow = false);

        /// @brief Manages player-specific logic, such as state and character updates.
        void PlayerSystem();

        /// @brief Handles collision detection and response for entities with colliders.
        void CollisionSystem();

        /// @brief Manages match-related logic, such as health updates and round progression.
        void MatchSystem();

        /// @brief Determines the winner of the match based on scores.
        void WinSystem();

        /// @brief Updates the game clock and manages time-related logic.
        void ClockSystem();

        /// @brief Processes player inputs and updates input history.
        void InputSystem();

        /// @brief Handles attack logic, such as applying damage and managing hitboxes.
        void AttackSystem();

        /// @brief Manages special attack logic, including damage and hitbox effects.
        void SpecialAttackSystem();

        // /// @brief Updates the position of entities based on their movement components.
        // class MovementSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Position>()
        //             .set<Movement>()
        //             .build();
        // };
        //
        // /// @brief Renders entities with position and texture components to the screen.
        // class RenderSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Position>()
        //             .set<Texture>()
        //             .build();
        //
        //     static inline bagel::Mask maskPlayer = bagel::MaskBuilder()
        //             .set<PlayerState>()
        //             .set<Character>()
        //             .build();
        //
        //     // Returns the sprite rectangle for a given action and frame
        //     static SDL_FRect getCharacterFrame(const Character& character, State action, int frame, bool shadow = false);
        // };
        //
        // /// @brief Manages player-specific logic, such as state and character updates.
        // class PlayerSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static constexpr Uint64 MAX_COMBO_TIME_MS = 500; // Maximum time between inputs to be considered a combo
        //     static constexpr Uint64 MAX_ACTION_TIME_MS = 500;
        //
        //     static State CheckCombo(const Inputs& inputs, int currentIndex,
        //                               Character character, Uint64 maxTimeMs);
        //
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Movement>()
        //             .set<Input>()
        //             .set<PlayerState>()
        //             .set<Character>()
        //             .build();
        // };
        //
        // /// @brief Handles collision detection and response for entities with colliders.
        // class CollisionSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Collider>()
        //             .set<Position>()
        //             .build();
        // };
        //
        // /// @brief Manages match-related logic, such as health updates and round progression.
        // class MatchSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Health>()
        //             .build();
        // };
        //
        // /// @brief Determines the winner of the match based on scores.
        // class WinSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Score>()
        //             .build();
        // };
        //
        // /// @brief Updates the game clock and manages time-related logic.
        // class ClockSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Time>()
        //             .build();
        // };
        //
        // /// @brief Processes player inputs and updates input history.
        // class InputSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Inputs>()
        //             .build();
        // };
        //
        // /// @brief Handles attack logic, such as applying damage and managing hitboxes.
        // class AttackSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Attack>()
        //             .set<Health>()
        //             .build();
        // };
        //
        // /// @brief Manages special attack logic, including damage and hitbox effects.
        // class SpecialAttackSystem final: bagel::NoInstance
        // {
        // public:
        //     static void run();
        // private:
        //     static inline bagel::Mask mask = bagel::MaskBuilder()
        //             .set<Attack>()
        //             .set<Health>()
        //             .build();
        // };

        /* =============== Entities =============== */
        /// @brief Entity is a unique identifier for each game object.

        /// @brief Creates a player's character (like Scorpion, Sub-Zero, etc.)
        /// @param x,y Position of the entity in the game world.
        /// @param character Character data for the player.
        /// @param renderer SDL renderer for rendering the texture.
        /// @return A `bagel::ent_type` representing the player entity.
        static inline bagel::ent_type createPlayer(float x, float y, Character character, SDL_Renderer* renderer) {
            bagel::Entity entity = bagel::Entity::create();

            // Construct the texture path
            std::string texturePath = "res/" + std::string(character.name) + ".png";

            // Load the image as a surface
            SDL_Surface* surface = IMG_Load(texturePath.c_str());
            if (!surface) {
                SDL_Log("Failed to load image: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
                return {};
            }

            const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);

            SDL_SetSurfaceColorKey(surface, true, SDL_MapRGB(fmt, nullptr,
                                                          COLOR_IGNORE_RED,
                                                          COLOR_IGNORE_GREEN,
                                                          COLOR_IGNORE_BLUE));

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
                          Texture{texture},
                          PlayerState{},
                          Inputs{},
                          character,
                          Health{100, 100});

            return entity.entity();
        }

        /// @brief Creates an Attack entity (like a punch or kick).
        /// @param x,y Position of the entity in the game world.
        /// @param type Type of the attack.
        /// @return A `bagel::ent_type` representing the attack entity.
        static inline bagel::ent_type createAttack(float x, float y, AttackType type) {
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
        static inline bagel::ent_type createSpecialAttack(float x, float y, SpecialAttackType type) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Position{x, y},
                          Collider{nullptr, nullptr},
                          SpecialAttack{type, 0.0f, 0.0f, 0, 0.0f, 0.0f});

            return entity.entity();
        }

        /// @brief Creates a static platform/boundary.
        /// @param x,y Position of the boundary in the game world.
        /// @param width, height Dimensions of the boundary.
        /// @return A `bagel::ent_type` representing the boundary entity.
        static inline bagel::ent_type createBoundary(float x, float y, float width, float height) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Position{x, y},
                          Collider{nullptr, nullptr});

            return entity.entity();
        }

        /// @brief Creates a game info entity.
        /// @param initialTime Initial time for the game.
        /// @return A `bagel::ent_type` representing the game info entity.
        static inline bagel::ent_type createGameInfo(float initialTime) {
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
        static inline bagel::ent_type createBackground(SDL_Texture* texture) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Position{0, 0},
                          Texture{texture, SDL_FRect{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}});

            return entity.entity();
        }

        struct Characters
        {
            constexpr static Character SUBZERO = {
                "Sub-Zero",
                const_cast<CharacterData::CharacterSpriteInfo *>(CharacterData::SUBZERO_SPRITE),
            {},
                1
            };
        };
    };
}


