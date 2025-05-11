#include "mortal_kombat_info.h"
#include "mortal_kombat.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

namespace mortal_kombat
{
    void processEntities(bagel::Mask mask, std::function<void(bagel::Entity&)> process) {
        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            bagel::Entity entity{e};
            if (entity.test(mask)) {
                process(entity);
            }
        }
    }

    void MovementSystem::run()
    {
        processEntities(mask, [](bagel::Entity& entity) {
            auto& position = entity.get<Position>();
            auto& movement = entity.get<Movement>();


            position.x += movement.vx;
            position.y += movement.vy;
        });
    }

    SDL_Renderer* RenderSystem::ren = nullptr;
    SDL_Window* RenderSystem::win = nullptr;

    RenderSystem::RenderSystem()
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cout << SDL_GetError() << std::endl;
            return;
        }

        if (!SDL_CreateWindowAndRenderer(
                "MK1992", WINDOW_WIDTH,WINDOW_HEIGHT, 0, &win, &ren)) {
            std::cout << SDL_GetError() << std::endl;
            return;
                }

        SDL_SetRenderDrawColor(ren, 30,30,30,255);
    }

    RenderSystem::~RenderSystem()
    {
        if (ren != nullptr)
            SDL_DestroyRenderer(ren);
        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }

    void RenderSystem::run()
    {

        processEntities(mask, [](bagel::Entity &entity) {
            auto& position = entity.get<Position>();
            auto& texture = entity.get<Texture>();

            if (entity.test(maskPlayer))
            {
                auto& playerState = entity.get<PlayerState>();
                auto& character = entity.get<Character>();

                texture.srcRect = getCharacterFrame(character, playerState.state, playerState.currFrame);
                texture.rect.w = static_cast<float>(character.sprite[static_cast<int>(playerState.state)].w)
                                                    * SCALE_CHARACTER;
                texture.rect.h = static_cast<float>(character.sprite[static_cast<int>(playerState.state)].h)
                                        * SCALE_CHARACTER;
            }
            else
            {

            }
            texture.rect.x = position.x;
            texture.rect.y = position.y;

            SDL_RenderTextureRotated(
                    ren, texture.tex, &texture.srcRect, &texture.rect, 0,
                    nullptr, SDL_FLIP_NONE);
        });

    }

    // Returns the sprite rectangle for a given action and frame
    SDL_FRect RenderSystem::getCharacterFrame(const Character& character, State action, const int frame,
                                               const bool shadow)
    {
        return {static_cast<float>(character.sprite[static_cast<int>(action)].x
                    + ((frame % character.sprite[static_cast<int>(action)].frameCount)
                    * (NEXT_FRAME_OFFSET + character.sprite[static_cast<int>(action)].w)))
                ,static_cast<float>(character.sprite[static_cast<int>(action)].y
                    + (shadow ? (SHADOW_OFFSET + character.sprite[static_cast<int>(action)].h) : 0))
                ,static_cast<float>(character.sprite[static_cast<int>(action)].w)
                ,static_cast<float>(character.sprite[static_cast<int>(action)].h)};
    }

    void SoundSystem::run()
    {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }

    void PlayerSystem::run()
    {
        processEntities(mask, [](bagel::Entity &entity) {
            auto& movment = entity.get<Movement>();
            const auto& inputs = entity.get<Inputs>();
            auto& playerState = entity.get<PlayerState>();
            auto& character = entity.get<Character>();

            playerState.prevState = playerState.state;

            if (playerState.prevState == State::WALK_BACKWARDS
                || playerState.prevState == State::WALK_FORWARDS)
            {
                movment.vx = 0;
            }

            if (playerState.busyFrames <= playerState.currFrame)
                playerState.busy = false;
            {
                auto& input = inputs.history[inputs.index];

                if (inputs.test(Inputs::JUMP_PUNCH))
                {
                    playerState.state = State::JUMP_PUNCH;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::JUMP_LOW_KICK))
                {
                    playerState.state = State::JUMP_LOW_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::JUMP_HIGH_KICK))
                {
                    playerState.state = State::JUMP_HIGH_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::CROUCH_BLOCK))
                {
                    playerState.state = State::CROUCH_BLOCK;
                }
                else if (inputs.test(Inputs::BLOCK))
                {
                    playerState.state = State::BLOCK;
                }
                else if (inputs.test(Inputs::CROUCH_KICK))
                {
                    playerState.state = State::CROUCH_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::UP))
                {
                    playerState.state = State::JUMP;
                }
                else if (inputs.test(Inputs::HIGH_SWEEP_KICK_LEFT) || inputs.test(Inputs::HIGH_SWEEP_KICK_RIGHT))
                {
                    playerState.state = State::HIGH_SWEEP_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::LOW_SWEEP_KICK_LEFT) || inputs.test(Inputs::LOW_SWEEP_KICK_RIGHT))
                {
                    playerState.state = State::LOW_SWEEP_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::UPPERCUT))
                {
                    playerState.state = State::UPPERCUT;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::DOWN))
                {
                    playerState.state = State::CROUCH;
                }
                else if (inputs.test(Inputs::LOW_PUNCH))
                {
                    playerState.state = State::LOW_PUNCH;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::HIGH_PUNCH))
                {
                    playerState.state = State::HIGH_PUNCH;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::LOW_KICK))
                {
                    playerState.state = State::LOW_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::HIGH_KICK))
                {
                    playerState.state = State::HIGH_KICK;
                    playerState.busy = true;
                }
                else if (inputs.test(Inputs::WALK_BACKWARDS_RIGHT) || inputs.test(Inputs::WALK_BACKWARDS_LEFT))
                {
                    playerState.state = State::WALK_BACKWARDS;
                    movment.vx = Character::WALK_SPEED_BACKWARDS * (playerState.direction == PlayerState::LEFT ? 1.f : -1.f);
                }
                else if (inputs.test(Inputs::WALK_FORWARDS_RIGHT) || inputs.test(Inputs::WALK_FORWARDS_LEFT))
                {
                    playerState.state = State::WALK_FORWARDS;
                    movment.vx = Character::WALK_SPEED_FORWARDS * (playerState.direction == PlayerState::LEFT ? -1.f : 1.f);
                }
                else
                {
                    playerState.state = State::STANCE;
                }
            }

            if (playerState.state == playerState.prevState)
            {
                playerState.currFrame++;
            }
            else
            {
                playerState.currFrame = 0;
                playerState.busyFrames = character.sprite[static_cast<int>(playerState.state)].frameCount;
            }
        });
    }


    // Helper function to check if a Combo of inputs has been entered
    State PlayerSystem::CheckCombo(const mortal_kombat::Inputs& inputs, const int currentIndex,
                                   Character character, Uint64 maxTimeMs) {
        // WIP

        // Start from the current index and go backwards
        Uint64 startTime = inputs.historyTime[currentIndex];
        return State::STANCE;
    }

    void CollisionSystem::run()
    {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }

    void MatchSystem::run()
    {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }

    void WinSystem::run()
    {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }

    void ClockSystem::run()
    {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }

    void InputSystem::run()
    {
        // Get keyboard state
        SDL_PumpEvents();
        auto keyboardState = SDL_GetKeyboardState(nullptr);

        processEntities(mask, [&](bagel::Entity &entity) {
            auto& inputs = entity.get<Inputs>();

            // Check if entity has PlayerState
            if (entity.has<PlayerState>()) {
                auto& playerState = entity.get<PlayerState>();

                inputs.index = (inputs.index + 1) % mortal_kombat::Inputs::MAX_HISTORY;
                inputs.history[inputs.index] = Inputs::RESET;

                inputs.history[inputs.index] |= (playerState.direction == PlayerState::LEFT) ?
                                                Inputs::DIRECTION_LEFT : Inputs::DIRECTION_RIGHT;
                inputs.history[inputs.index] |= (playerState.isJumping) ?
                                                Inputs::JUMPING : 0;

                // Player 1 controls (using WASD for movement, space, etc. for actions)
                if (entity.has<Character>() && entity.get<Character>().playerNumber == 1) {
                    inputs.history[inputs.index] |= (keyboardState[SDL_SCANCODE_H] ? Inputs::BLOCK : 0)
                                                 | (keyboardState[SDL_SCANCODE_W] ? Inputs::UP : 0)
                                                 | (keyboardState[SDL_SCANCODE_S] ? Inputs::DOWN : 0)
                                                 | (keyboardState[SDL_SCANCODE_A] ? Inputs::LEFT : 0)
                                                 | (keyboardState[SDL_SCANCODE_D] ? Inputs::RIGHT : 0)
                                                 | (keyboardState[SDL_SCANCODE_F] ? Inputs::LOW_PUNCH : 0)
                                                 | (keyboardState[SDL_SCANCODE_R] ? Inputs::HIGH_PUNCH : 0)
                                                 | (keyboardState[SDL_SCANCODE_G] ? Inputs::LOW_KICK : 0)
                                                 | (keyboardState[SDL_SCANCODE_T] ? Inputs::HIGH_KICK : 0);
                }
                // Player 2 controls (using arrow keys and numpad)
                else if (entity.has<Character>() && entity.get<Character>().playerNumber == 2) {
                    inputs.history[inputs.index] |= (keyboardState[SDL_SCANCODE_APOSTROPHE] ? Inputs::BLOCK : 0)
                                                 | (keyboardState[SDL_SCANCODE_UP] ? Inputs::UP : 0)
                                                 | (keyboardState[SDL_SCANCODE_DOWN] ? Inputs::DOWN : 0)
                                                 | (keyboardState[SDL_SCANCODE_LEFT] ? Inputs::LEFT : 0)
                                                 | (keyboardState[SDL_SCANCODE_RIGHT] ? Inputs::RIGHT : 0)
                                                 | (keyboardState[SDL_SCANCODE_K] ? Inputs::LOW_PUNCH : 0)
                                                 | (keyboardState[SDL_SCANCODE_I] ? Inputs::HIGH_PUNCH : 0)
                                                 | (keyboardState[SDL_SCANCODE_L] ? Inputs::LOW_KICK : 0)
                                                 | (keyboardState[SDL_SCANCODE_O] ? Inputs::HIGH_KICK : 0);
                }

                // Update input processing time for combo timing
                inputs.historyTime[inputs.index] = SDL_GetTicks();
            }
        });
    }

    void AttackSystem::run() {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }

    void SpecialAttackSystem::run() {
        processEntities(mask, [](bagel::Entity &entity) {

        });
    }
}