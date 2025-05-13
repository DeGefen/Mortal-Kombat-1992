#pragma once
#include "mortal_kombat_info.h"
#include <functional>
#include <string>

#include "SDL3/SDL.h"
#include "box2d/box2d.h"
#include "bagel.h"
#include "lib/box2d/src/body.h"
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
        static constexpr float SCALE_CHARACTER = 0.9f;
        static constexpr int WINDOW_SCALE = 100;

        static constexpr int PLAYER_1_BASE_X = WINDOW_WIDTH / 4 - (CHAR_SQUARE_WIDTH / 2);
        static constexpr int PLAYER_2_BASE_X = (WINDOW_WIDTH / 4) * 3 - (CHAR_SQUARE_WIDTH / 2);
        static constexpr int PLAYER_BASE_Y = WINDOW_HEIGHT / 2;

        static constexpr int FPS = 60;
        static constexpr float	BOX2D_STEP = 1.f/FPS;

        static constexpr float CHARACTER_WIDTH = 50;
        static constexpr float CHARACTER_HEIGHT = 135;

        static constexpr int NONE = -1;

        SDL_Renderer* ren;
        SDL_Window* win;
        b2WorldId boxWorld;

        void prepareBoxWorld();

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
            void reset() {vx = vy = 0;}
        };

        /// @brief Texture component holds the SDL texture and its rectangle for rendering.
        struct Texture {
            SDL_Texture *tex = nullptr;
            SDL_FRect srcRect = {0, 0, 0, 0}; // Source rectangle for texture
            SDL_FRect rect = {0, 0, 0, 0}; // Destination rectangle for rendering
        };

        /// @brief Collider component holds the physics body and shape.
        struct Collider {
            b2BodyId body = b2_nullBodyId; // Default invalid body ID
            b2ShapeId shape = b2_nullShapeId; // Default invalid shape ID
            bool isSensor = false; // Whether the collider is a sensor
        };

        /// @brief Player_state component holds the state for the player.
        struct PlayerState {
            State state = State::STANCE;
            bool direction = RIGHT;
            bool isJumping = false; // Whether the player is jumping
            bool isCrouching = false; // Whether the player is crouching
            bool isAttacking = false; // Whether the player is attacking
            bool isLaying = false; // Whether the player is laying down
            bool busy = false; // Whether the player is busy
            int playerNumber = 1; // Player number (1 or 2)
            int busyFrames = 0; // Total frames spent in the current state
            int currFrame = 0; //  Frames spent in the current state
            int freezeFrame = NONE; // Frame to freeze the player
            int freezeFrameDuration = 0; // Duration of the freeze frame

            static constexpr bool LEFT = true;
            static constexpr bool RIGHT = false;

            void reset()
            {
                state = State::STANCE;
                isJumping = false;
                isCrouching = false;
                isAttacking = false;
                isLaying = false;
                busy = false;
                busyFrames = 0;
                currFrame = 0;
                freezeFrame = NONE;
            }
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
            State type;
            int attacker;
            float damage = 0.0f;
            float hitbox = 0.0f;
            int hitbox_type = 0;
            float hitbox_size = 0.0f;
            float hitbox_duration = 0.0f;

            static constexpr int ATTACK_LIFE_TIME = 1;
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
            SpriteData sprite;
            Input special_moves_input[3] = {};
        };

        /// @brief Health component holds the maximum and current health of the player.
        struct Health {
            float max_health = 100.0f;
            float health = 100.0f;
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

        /// @brief Updates the position of entities based on their movement components.
        void MovementSystem();

        static b2Vec2 getPosition(const Position& position)
        {
            return {position.x / WINDOW_SCALE, position.y / WINDOW_SCALE};
        }

        static b2Vec2 getPosition(const float x, const float y)
        {
            return {x / WINDOW_SCALE, y / WINDOW_SCALE};
        }

        /// @brief Renders entities with position and texture components to the screen.
        void RenderSystem();

        /// @brief Returns the sprite rectangle for a given action and frame.
        /// @param character Character data for the player.
        /// @param action Action state of the character.
        /// @param frame Frame number of the action.
        /// @param shadow Whether to return shadow.
        /// @return SDL_FRect representing the sprite rectangle.
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

        /// @brief Handles attack creation.
        void AttackSystem(bagel::Entity &eAttack);

        /// @brief Handles combat logic, such has damage application, and player hit state.
        /// @param eAttack Entity representing the attack.
        /// @param ePlayer Entity representing the attacked player.
        static void CombatSystem(bagel::Entity &eAttack, bagel::Entity &ePlayer);

        /// @brief Handles attack's entity destruction and decay logic.
        void AttackDecaySystem();

        /// @brief Manages special attack logic, including damage and hitbox effects.
        void SpecialAttackSystem();

        /* =============== Entities =============== */
        /// @brief Entity is a unique identifier for each game object.

        /// @brief Creates a player's character (like Scorpion, Sub-Zero, etc.)
        /// @param x,y Position of the entity in the game world.
        /// @param character Character data for the player.
        /// @param playerNumber Player number (1 or 2).
        void createPlayer(float x, float y, Character character, int playerNumber);

        /// @brief Creates an Attack entity (like a punch or kick).
        /// @param x,y Position of the entity in the game world.
        /// @param type Type of the attack.
        /// @param playerNumber Player number (1 or 2).
        void createAttack(float x, float y, State type, int playerNumber);

        /// @brief Creates a special attack entity.
        /// @param x,y Position of the entity in the game world.
        /// @param type Type of the special attack.
        static inline void createSpecialAttack(float x, float y, SpecialAttackType type);

        /// @brief Creates a static platform/boundary.
        /// @param x,y Position of the boundary in the game world.
        /// @param width, height Dimensions of the boundary.
        static inline void createBoundary(float x, float y, float width, float height);

        /// @brief Creates a game info entity.
        /// @param initialTime Initial time for the game.
        static inline void createGameInfo(float initialTime);

        /// @brief Creates a background entity.
        /// @param backgroundName SDL texture for the background.
        inline void createBackground(std::string backgroundName);

        struct Characters
        {
            constexpr static Character SUBZERO = {
                "Sub-Zero",
                SUBZERO_SPRITE,
            {}};

            constexpr static Character LIU_KANG = {
                "Liu Kang",
                LIU_KANG_SPRITE,
            {}};
        };


    };
}


