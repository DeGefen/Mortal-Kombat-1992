#include "mortal_kombat.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
using namespace mortal_kombat;

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

void RenderSystem::run()
{
    processEntities(mask, [](bagel::Entity &entity) {

    });
}

void SoundSystem::run()
{
    processEntities(mask, [](bagel::Entity &entity) {

    });
}

void PlayerSystem::run()
{
    processEntities(mask, [](bagel::Entity &entity) {
        auto& inputs = entity.get<Inputs>();
        auto& playerState = entity.get<PlayerState>();
        auto& character = entity.get<Character>();

        bool move = false;
        const Input high_Sweep_Kick = playerState.direction ? Inputs::HIGH_SWEEP_KICK_LEFT : Inputs::HIGH_SWEEP_KICK_RIGHT;

        switch (inputs.history[inputs.index]) {
            case Inputs::CROUCH_BLOCK:
                playerState.state = State::CROUCH_BLOCK;
                break;
            case Inputs::BLOCK:
                playerState.state = State::BLOCK;
                break;
            case
            Inputs::UP:
                playerState.state = State::JUMP;
                break;
            case Inputs::HIGH_SWEEP_KICK_LEFT:
            case Inputs::HIGH_SWEEP_KICK_RIGHT:
                playerState.state = State::HIGH_SWEEP_KICK;
                break;

        }
    });
}


// Helper function to check if a Combo of inputs has been entered
State PlayerSystem::CheckCombo(const mortal_kombat::Inputs& inputs, int currentIndex,
                               Character character, Uint64 maxTimeMs) {
    // WIP

    // Start from the current index and go backwards
    int historyIndex = currentIndex;
    Uint64 startTime = inputs.historyTime[historyIndex];
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
    auto keyboardState = SDL_GetKeyboardState(nullptr);

    processEntities(mask, [&](bagel::Entity &entity) {
        auto& inputs = entity.get<Inputs>();

        // Check if entity has PlayerState
        if (entity.has<PlayerState>()) {
            auto& playerState = entity.get<PlayerState>();

            inputs.index = (inputs.index + 1) % mortal_kombat::Inputs::MAX_HISTORY;
            inputs.history[inputs.index] = Inputs::RESET;

            // Player 1 controls (using WASD for movement, space, etc. for actions)
            if (entity.has<Character>() && entity.get<Character>().playerNumber == 1) {
                inputs.history[inputs.index] |= (keyboardState[SDL_SCANCODE_H] ? Inputs::BLOCK : 0)
                                             | (keyboardState[SDL_SCANCODE_W] ? Inputs::UP : 0)
                                             | (keyboardState[SDL_SCANCODE_S] ? Inputs::DOWN : 0)
                                             | (keyboardState[SDL_SCANCODE_A] ? Inputs::LEFT : 0)
                                             | (keyboardState[SDL_SCANCODE_D] ? Inputs::RIGHT : 0)
                                             | (keyboardState[SDL_SCANCODE_F] ? Inputs::LOW_PUNCH : 0)
                                             | (keyboardState[SDL_SCANCODE_R] ? Inputs::HIGH_PUNCH : 0)
                                             | (keyboardState[SDL_SCANCODE_G] ? Inputs::LOW_PUNCH : 0)
                                             | (keyboardState[SDL_SCANCODE_T] ? Inputs::HIGH_PUNCH : 0);
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
                                             | (keyboardState[SDL_SCANCODE_L] ? Inputs::LOW_PUNCH : 0)
                                             | (keyboardState[SDL_SCANCODE_O] ? Inputs::HIGH_PUNCH : 0);
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

