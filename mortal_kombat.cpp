#include "mortal_kombat_info.h"
#include "mortal_kombat.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

namespace mortal_kombat
{
    MK::MK()
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cout << SDL_GetError() << std::endl;
            return;
        }

        if (!SDL_CreateWindowAndRenderer(
                "MK1992", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &win, &ren)) {
            std::cout << SDL_GetError() << std::endl;
            return;
                }

        SDL_SetRenderDrawColor(ren, 30,30,30,255);
    }

    MK::~MK()
    {
        if (ren != nullptr)
            SDL_DestroyRenderer(ren);
        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }

    void MK::prepareBoxWorld()
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0,0};
        boxWorld = b2CreateWorld(&worldDef);
    }

    void MK::run()
    {
        prepareBoxWorld();
        createPlayer(100, 200, (Characters::SUBZERO), 1);
        createPlayer(500, 200, (Characters::SUBZERO), 2);

        while (true)
        {
            b2World_Step(boxWorld, BOX2D_STEP, 1);
            InputSystem();
            PlayerSystem();
            CollisionSystem();
            MovementSystem();
            RenderSystem();
            SDL_Delay(70);
        }
    }

    void MK::processEntities(bagel::Mask mask, const std::function<void(bagel::Entity&)>& process) {
        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            if (bagel::Entity entity{e}; entity.test(mask)) {
                process(entity);
            }
        }
    }

    void MK::MovementSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Position>()
            .set<Movement>()
            .build();

        static const bagel::Mask maskPlayer = bagel::MaskBuilder()
            .set<PlayerState>()
            .set<Collider>()
            .build();

        MK::processEntities(mask, [](bagel::Entity& entity) {
            auto& position = entity.get<Position>();
            auto& movement = entity.get<Movement>();



            if (entity.test(maskPlayer))
            {

                auto& playerState = entity.get<PlayerState>();
                auto& collider = entity.get<Collider>();

                switch (playerState.state)
                {
                case State::WALK_BACKWARDS:
                    movement.vx = WALK_SPEED_BACKWARDS * (playerState.direction == PlayerState::LEFT ? 1.0f : -1.0f);
                    break;
                case State::WALK_FORWARDS:
                    movement.vx = WALK_SPEED_FORWARDS
                                    * (playerState.direction == PlayerState::LEFT ? -1.0f : 1.0f)
                                    * (collider.isSensor ? 0.0f : 1.0f);
                    break;
                default:
                    movement.reset();
                    break;
                }
                position.x += movement.vx;
                position.y += movement.vy;

                b2Vec2 v;
                v.x = position.x / SCALE_CHARACTER;
                v.y = position.y / SCALE_CHARACTER;
                b2Body_SetTransform(collider.body, v, b2Rot_identity);

            }
        });
    }

    void MK::RenderSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Position>()
            .set<Texture>()
            .build();

        static const bagel::Mask maskPlayer = bagel::MaskBuilder()
            .set<PlayerState>()
            .set<Character>()
            .build();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                exit(0);
            }
        }
        SDL_RenderClear(ren);

        MK::processEntities(mask, [&](bagel::Entity& entity) {
            SDL_FlipMode flipMode = SDL_FLIP_NONE;

            auto& position = entity.get<Position>();
            auto& texture = entity.get<Texture>();

            if (entity.test(maskPlayer)) {
                auto& playerState = entity.get<PlayerState>();
                auto& character = entity.get<Character>();

                flipMode = (playerState.direction == PlayerState::LEFT) ?
                    SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

                const int frame = (playerState.state == State::WALK_BACKWARDS) ?
                    playerState.busyFrames - (playerState.currFrame % playerState.busyFrames): playerState.currFrame;
                texture.srcRect = MK::getCharacterFrame(character, playerState.state, frame);
                texture.rect.w = static_cast<float>((character.sprite[playerState.state].w)) * SCALE_CHARACTER;
                texture.rect.h = static_cast<float>((character.sprite[playerState.state].h)) * SCALE_CHARACTER;
            }

            texture.rect.x = position.x;
            texture.rect.y = position.y;

            SDL_RenderTextureRotated(
                ren, texture.tex, &texture.srcRect, &texture.rect, 0,
                nullptr, flipMode);
        });

        SDL_RenderPresent(ren);
    }

    SDL_FRect MK::getCharacterFrame(const Character& character, State action, const int frame,
                                               const bool shadow)
    {
        return {static_cast<float>(character.sprite[action].x
                    + ((frame % character.sprite[(action)].frameCount)
                    * (NEXT_FRAME_OFFSET + character.sprite[action].w)))
                ,static_cast<float>(character.sprite[action].y
                    + (shadow ? (SHADOW_OFFSET + character.sprite[action].h) : 0))
                ,static_cast<float>(character.sprite[action].w)
                ,static_cast<float>(character.sprite[action].h)};
    }

    void MK::PlayerSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Input>()
            .set<PlayerState>()
            .set<Character>()
            .build();

        MK::processEntities(mask, [](bagel::Entity &entity) {
                const auto& inputs = entity.get<Inputs>();
                auto& playerState = entity.get<PlayerState>();
                auto& character = entity.get<Character>();


                playerState.prevState = playerState.state;

                if (playerState.busyFrames <= playerState.currFrame)
                    playerState.busy = false;

                State state;
                int freezeFrame = PlayerState::NONE;
                int offSetFrame = 0;
                bool busy = true;
                bool crouching = false;

                if (inputs.test(Inputs::JUMP_PUNCH))
                {
                    state = State::JUMP_PUNCH;
                }
                else if (inputs.test(Inputs::JUMP_LOW_KICK))
                {
                    state = State::JUMP_LOW_KICK;
                }
                else if (inputs.test(Inputs::JUMP_HIGH_KICK))
                {
                    state = State::JUMP_HIGH_KICK;
                }
                else if (inputs.test(Inputs::CROUCH_BLOCK))
                {
                    state = State::CROUCH_BLOCK;
                    freezeFrame = character.sprite[state].frameCount / 2 + 1;
                    crouching = true;
                }
                else if (inputs.test(Inputs::BLOCK))
                {
                    state = State::BLOCK;
                    freezeFrame = character.sprite[state].frameCount / 2 + 1;
                }
                else if (inputs.test(Inputs::CROUCH_KICK))
                {
                    state = State::CROUCH_KICK;
                    crouching = true;
                }
                else if (inputs.test(Inputs::UP))
                {
                    state = State::JUMP;
                }
                else if (inputs.test(Inputs::HIGH_SWEEP_KICK_LEFT)
                        || inputs.test(Inputs::HIGH_SWEEP_KICK_RIGHT))
                {
                    state = State::HIGH_SWEEP_KICK;
                }
                else if (inputs.test(Inputs::LOW_SWEEP_KICK_LEFT)
                        || inputs.test(Inputs::LOW_SWEEP_KICK_RIGHT))
                {
                    state = State::LOW_SWEEP_KICK;
                }
                else if (inputs.test(Inputs::UPPERCUT))
                {
                    state = State::UPPERCUT;
                    crouching = true;
                }
                else if (inputs.test(Inputs::DOWN))
                {
                    state = State::CROUCH;
                    freezeFrame = character.sprite[state].frameCount / 2 + 1;
                    crouching = true;
                }
                else if (inputs.test(Inputs::LOW_PUNCH))
                {
                    state = State::LOW_PUNCH;
                }
                else if (inputs.test(Inputs::HIGH_PUNCH))
                {
                    state = State::HIGH_PUNCH;
                }
                else if (inputs.test(Inputs::LOW_KICK))
                {
                    state = State::LOW_KICK;
                }
                else if (inputs.test(Inputs::HIGH_KICK))
                {
                    state = State::HIGH_KICK;
                }
                else if (inputs.test(Inputs::WALK_BACKWARDS_RIGHT)
                        || inputs.test(Inputs::WALK_BACKWARDS_LEFT))
                {
                    state = State::WALK_BACKWARDS;
                    busy = false;
                }
                else if (inputs.test(Inputs::WALK_FORWARDS_RIGHT)
                        || inputs.test(Inputs::WALK_FORWARDS_LEFT))
                {
                    state = State::WALK_FORWARDS;
                    busy = false;
                }
                else
                {
                    state = State::STANCE;
                    busy = false;
                }


                if ((!playerState.busy && state != playerState.state)
                    || playerState.state == State::CROUCH && crouching)
                {
                    playerState.state = state;
                    playerState.currFrame = (playerState.isCrouching && state == State::CROUCH) ? 2 : 0;
                    playerState.busyFrames = character.sprite[playerState.state].frameCount;
                    playerState.freezeFrame = freezeFrame;
                    playerState.isCrouching = crouching;
                    playerState.busy = busy;
                }
                else if (state == playerState.state)
                {
                    playerState.currFrame = (playerState.freezeFrame != PlayerState::NONE
                                            && playerState.currFrame >= playerState.freezeFrame)
                                            ? playerState.freezeFrame : playerState.currFrame + 1;
                }
                else
                {
                    playerState.currFrame = playerState.currFrame + 1;
                }
        });
    }

    void MK::InputSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Inputs>()
            .build();

        SDL_PumpEvents();
        auto keyboardState = SDL_GetKeyboardState(nullptr);

        MK::processEntities(mask, [&](bagel::Entity& entity) {
            auto& inputs = entity.get<Inputs>();
            auto& playerState = entity.get<PlayerState>();

            inputs.index = (inputs.index + 1) % MK::Inputs::MAX_HISTORY;
            inputs.history[inputs.index] = Inputs::RESET;

            inputs.history[inputs.index] |= (playerState.direction == PlayerState::LEFT) ?
                                            Inputs::DIRECTION_LEFT : Inputs::DIRECTION_RIGHT;
            inputs.history[inputs.index] |= (playerState.isJumping) ?
                                            Inputs::JUMPING : 0;

            // Player 1 controls (using WASD for movement, space, etc. for actions)
            if (playerState.playerNumber == 1) {
                inputs.history[inputs.index] |=
                    (keyboardState[SDL_SCANCODE_H] ? Inputs::BLOCK : 0)
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
            else if (playerState.playerNumber == 2) {
                inputs.history[inputs.index] |=
                    (keyboardState[SDL_SCANCODE_APOSTROPHE] ? Inputs::BLOCK : 0)
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
        });
    }

    void MK::CollisionSystem() {
        const auto se = b2World_GetSensorEvents(boxWorld);
		for (int i = 0; i < se.endCount; ++i)
		{
		    b2BodyId b = b2Shape_GetBody(se.endEvents[i].visitorShapeId);
		    auto *e = static_cast<bagel::ent_type*>(b2Body_GetUserData(b));
		    auto collider = bagel::Entity{(*e)}.get<Collider>();
		    collider.isSensor = true;
            printf("Collision detected between entities: ", e->id);
		}
    }

    void MK::MatchSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Health>()
            .build();

        MK::processEntities(mask, [](bagel::Entity& entity) {
            // Match logic here
        });
    }

    void MK::WinSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Score>()
            .build();

        MK::processEntities(mask, [](bagel::Entity& entity) {
            // Win logic here
        });
    }

    void MK::ClockSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Time>()
            .build();

        MK::processEntities(mask, [](bagel::Entity& entity) {
            // Clock logic here
        });
    }

    void MK::AttackSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Attack>()
            .set<Health>()
            .build();

        MK::processEntities(mask, [](bagel::Entity& entity) {
            // Attack logic here
        });
    }

    void MK::SpecialAttackSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Attack>()
            .set<Health>()
            .build();

        MK::processEntities(mask, [](bagel::Entity& entity) {
            // Special attack logic here
        });
    }
}