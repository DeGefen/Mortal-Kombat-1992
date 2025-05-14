#include "mortal_kombat_info.h"
#include "mortal_kombat.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

namespace mortal_kombat
{
    std::unordered_map<std::string, SDL_Texture*> MK::TextureSystem::textureCache;

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

        SDL_SetRenderDrawColor(ren, 255,255,255,0);

        prepareBoxWorld();
    }

    MK::~MK()
    {
        TextureSystem::clearCache();
        if (b2World_IsValid(boxWorld))
            b2DestroyWorld(boxWorld);
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

        Uint32 frameStart, frameTime;
        int frame_count = 0;

        createBackground("res/Background.png");

        createPlayer(PLAYER_1_BASE_X, PLAYER_BASE_Y, (Characters::SUBZERO), 1);
        createPlayer(PLAYER_2_BASE_X, PLAYER_BASE_Y, (Characters::LIU_KANG), 2);

        while (true)
        {
            frameStart = SDL_GetTicks();

            if (frame_count % INPUT_FRAME_DELAY == 0)
            {
                InputSystem();
            }
            if (++frame_count % ACTION_FRAME_DELAY == 0)
            {
                PlayerSystem();
            }
            CollisionSystem();
            MovementSystem();
            RenderSystem();
            AttackDecaySystem();

            frameTime = SDL_GetTicks() - frameStart;
            if (FRAME_DELAY > frameTime) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }
    }

    void MK::MovementSystem()
    {
        static constexpr float WALK_SPEED_BACKWARDS = 3.0f * SCALE_CHARACTER;
        static constexpr float WALK_SPEED_FORWARDS = 4.0f * SCALE_CHARACTER;
        static constexpr float KICKBACK_SPEED = 3.0f * SCALE_CHARACTER;
        static constexpr float FALL_SPEED = 4.0f * SCALE_CHARACTER;

        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Position>()
            .set<Movement>()
            .set<Collider>()
            .build();

        static const bagel::Mask maskPlayer = bagel::MaskBuilder()
            .set<PlayerState>()
            .set<Character>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
                auto& position = entity.get<Position>();
                auto& movement = entity.get<Movement>();
                auto& collider = entity.get<Collider>();

                if (entity.test(maskPlayer))
                {
                    auto& playerState = entity.get<PlayerState>();
                    auto& character = entity.get<Character>();

                    switch (playerState.state)
                    {
                    case State::WALK_BACKWARDS:
                        movement.vx = WALK_SPEED_BACKWARDS
                                        * (playerState.direction == PlayerState::LEFT ? 1.0f : -1.0f);
                        break;
                    case State::WALK_FORWARDS:
                        movement.vx = WALK_SPEED_FORWARDS
                                        * (playerState.direction == PlayerState::LEFT ? -1.0f : 1.0f)
                                        * (collider.isSensor ? 0.0f : 1.0f);
                        break;
                    case State::KICKBACK_TORSO_HIT:
                        movement.vx = KICKBACK_SPEED
                                        * (playerState.direction == PlayerState::LEFT ? 1.0f : -1.0f);
                        break;
                    case State::FALL:
                    case State::UPPERCUT_HIT:
                        if (playerState.currFrame < character.sprite[playerState.state].frameCount / 2)
                        {
                            movement.vx = FALL_SPEED
                                        * (playerState.direction == PlayerState::LEFT ? 1.0f : -1.0f);
                            break;
                        }
                    default:
                        movement.reset();
                        break;
                    }

                    position.x += movement.vx;
                    position.y += movement.vy;

                    if (playerState.isCrouching)
                    {
                        b2Body_SetTransform(
                            collider.body,
                            getPosition(position.x, position.y - (CHARACTER_HEIGHT/2.0f)),
                            b2Rot_identity);
                    }
                    else
                    {
                        b2Body_SetTransform(
                                collider.body,
                                getPosition(position),
                                b2Rot_identity);
                    }
                    continue;
                }
                position.x += movement.vx;
                position.y += movement.vy;
            }
        }
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

        static const bagel::Mask maskSpecialAttack = bagel::MaskBuilder()
            .set<SpecialAttack>()
            .set<Character>()
            .build();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                exit(0);
            }
        }
        SDL_RenderClear(ren);

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
                SDL_FlipMode flipMode = SDL_FLIP_NONE;

                auto& position = entity.get<Position>();
                auto& texture = entity.get<Texture>();

                if (entity.test(maskPlayer)) {
                    auto& playerState = entity.get<PlayerState>();
                    auto& character = entity.get<Character>();

                    flipMode = (playerState.direction == PlayerState::LEFT) ?
                        SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

                    const int frame = (playerState.state == State::WALK_BACKWARDS)
                        ? (playerState.busyFrames - (playerState.currFrame % playerState.busyFrames)): (playerState.currFrame);

                    texture.srcRect = MK::getSpriteFrame(character, playerState.state, frame);
                    texture.rect.w = static_cast<float>((character.sprite[playerState.state].w)) * SCALE_CHARACTER;
                    texture.rect.h = static_cast<float>((character.sprite[playerState.state].h)) * SCALE_CHARACTER;
                }
                else if (entity.test(maskSpecialAttack))
                {
                    auto& specialAttack = entity.get<SpecialAttack>();
                    auto& character = entity.get<Character>();
                    flipMode = (specialAttack.direction == PlayerState::LEFT) ?
                        SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

                    texture.srcRect = MK::getSpriteFrame(character, specialAttack.type, specialAttack.frame);

                    texture.rect.w = static_cast<float>((character.specialAttackSprite[specialAttack.type].w)) * SCALE_CHARACTER;
                    texture.rect.h = static_cast<float>((character.specialAttackSprite[specialAttack.type].h)) * SCALE_CHARACTER;
                }

                texture.rect.x = position.x;
                texture.rect.y = position.y;

                SDL_RenderTextureRotated(
                    ren, texture.tex, &texture.srcRect, &texture.rect, 0,
                    nullptr, flipMode);
            }
        }

        SDL_RenderPresent(ren);
    }

    SDL_FRect MK::getSpriteFrame(const Character& character, State action, const int frame,
                                               const bool shadow)
    {
        return {static_cast<float>(character.sprite[action].x
                    + ((frame % character.sprite[(action)].frameCount)
                    * (NEXT_FRAME_OFFSET + character.sprite[action].w))) + 1
                ,static_cast<float>(character.sprite[action].y
                    + (shadow ? (SHADOW_OFFSET + character.sprite[action].h) : 0)) + 1
                ,static_cast<float>(character.sprite[action].w) - 2
                ,static_cast<float>(character.sprite[action].h) - 2};
    };

    SDL_FRect MK::getSpriteFrame(const Character& character, SpecialAttacks action,
                                            int frame)
    {
        return {static_cast<float>(character.specialAttackSprite[action].x
                    + ((frame % character.specialAttackSprite[(action)].frameCount)
                    * (NEXT_FRAME_OFFSET + character.specialAttackSprite[action].w))) + 1
                ,static_cast<float>(character.specialAttackSprite[action].y) + 1
                ,static_cast<float>(character.specialAttackSprite[action].w) - 2
                ,static_cast<float>(character.specialAttackSprite[action].h) - 2};
    };

    void MK::PlayerSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Inputs>()
            .set<PlayerState>()
            .set<Character>()
            .build();

            for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
            {
                if (bagel::Entity entity{e}; entity.test(mask))
                {
                    const auto& inputs = entity.get<Inputs>();
                    auto& playerState = entity.get<PlayerState>();
                    auto& character = entity.get<Character>();

                    State state = State::STANCE;
                    int freezeFrame = NONE;
                    int offSetFrame = 0;
                    int freezeFrameDuration = 0;
                    bool busy = true;
                    bool crouching = false;
                    bool attack = false;
                    bool special = false;

                    // Handels specail attacks
                    for (int i = 0; i < Character::SPECIAL_ATTACKS_COUNT && !special; ++i)
                    {
                        if (inputs == character.specialAttacks[i]
                            || inputs == character.specialAttacks[i + 1])
                        {
                            state = static_cast<State>(i + static_cast<int>(State::SPECIAL_1));
                            attack = true;
                            special = true;
                        }
                    }

                    if (!special)
                    {
                        if (inputs == Inputs::JUMP_PUNCH)
                        {
                            state = State::JUMP_PUNCH;
                            attack = true;
                        }
                        else if (inputs == Inputs::JUMP_LOW_KICK)
                        {
                            state = State::JUMP_LOW_KICK;
                            attack = true;
                        }
                        else if (inputs == Inputs::JUMP_HIGH_KICK)
                        {
                            state = State::JUMP_HIGH_KICK;
                            attack = true;
                        }
                        else if (inputs == Inputs::CROUCH_BLOCK)
                        {
                            state = State::CROUCH_BLOCK;
                            freezeFrame = character.sprite[state].frameCount / 2 + 1;
                            freezeFrameDuration = 1;
                            crouching = true;
                        }
                        else if (inputs == Inputs::BLOCK)
                        {
                            state = State::BLOCK;
                            freezeFrame = character.sprite[state].frameCount / 2 + 1;
                            freezeFrameDuration = 1;
                        }
                        else if (inputs == Inputs::CROUCH_KICK)
                        {
                            state = State::CROUCH_KICK;
                            crouching = true;
                            attack = true;
                        }
                        else if (inputs == Inputs::UP)
                        {
                            state = State::JUMP;
                        }
                        else if (inputs == Inputs::HIGH_SWEEP_KICK_LEFT
                                || inputs == Inputs::HIGH_SWEEP_KICK_RIGHT)
                        {
                            state = State::HIGH_SWEEP_KICK;
                            attack = true;
                        }
                        else if (inputs == Inputs::LOW_SWEEP_KICK_LEFT
                                || inputs == Inputs::LOW_SWEEP_KICK_RIGHT)
                        {
                            state = State::LOW_SWEEP_KICK;
                            attack = true;
                        }
                        else if (inputs == Inputs::UPPERCUT)
                        {
                            state = State::UPPERCUT;
                            crouching = true;
                            attack = true;
                        }
                        else if (inputs == Inputs::DOWN)
                        {
                            state = State::CROUCH;
                            freezeFrame = (character.sprite[state].frameCount / 2) + 1;
                            freezeFrameDuration = 1;
                            crouching = true;
                        }
                        else if (inputs == Inputs::LOW_PUNCH)
                        {
                            state = State::LOW_PUNCH;
                            attack = true;
                        }
                        else if (inputs == Inputs::HIGH_PUNCH)
                        {
                            state = State::HIGH_PUNCH;
                            attack = true;
                        }
                        else if (inputs == Inputs::LOW_KICK)
                        {
                            state = State::LOW_KICK;
                            attack = true;
                        }
                        else if (inputs == Inputs::HIGH_KICK)
                        {
                            state = State::HIGH_KICK;
                            attack = true;
                        }
                        else if (inputs == Inputs::WALK_BACKWARDS_RIGHT
                                || inputs == Inputs::WALK_BACKWARDS_LEFT)
                        {
                            state = State::WALK_BACKWARDS;
                            busy = false;
                        }
                        else if (inputs == Inputs::WALK_FORWARDS_RIGHT
                                || inputs == Inputs::WALK_FORWARDS_LEFT)
                        {
                            state = State::WALK_FORWARDS;
                            busy = false;
                        }
                        else
                        {
                            state = State::STANCE;
                            busy = false;
                        }
                    }

                    // Check if the player is busy and update the state accordingly
                    if (playerState.busyFrames - 1 <= playerState.currFrame
                        && playerState.freezeFrameDuration <= 0)
                        playerState.busy = false;

                    // Check if the player is laying and not busy
                    // -> if so get up
                    if (playerState.isLaying && !playerState.busy)
                    {
                        playerState.reset();
                        playerState.state = State::GETUP;
                        playerState.busyFrames = character.sprite[playerState.state].frameCount;
                        playerState.busy = true;
                    }

                    // Check if the player is in a different state and update accordingly
                    if ((!playerState.busy && (state != playerState.state || attack))
                        || playerState.state == State::CROUCH && crouching
                        && state != State::CROUCH)
                    {
                        playerState.reset();
                        playerState.state = state;
                        playerState.currFrame = (playerState.isCrouching && state == State::CROUCH) ? 2 : 0;
                        playerState.busyFrames = character.sprite[playerState.state].frameCount;
                        playerState.freezeFrame = freezeFrame;
                        playerState.freezeFrameDuration = freezeFrameDuration;
                        playerState.isCrouching = crouching;
                        playerState.isAttacking = attack;
                        playerState.isSpecicalAttacking = special;
                        playerState.specialAttackCooldown = special ? playerState.busyFrames * 2 : 0;
                        playerState.busy = busy;
                    }

                    // increases duration of freeze frames if the state is the same
                    // e.g. when the player is blocking
                    if (playerState.freezeFrame != NONE && state == playerState.state)
                    {
                        ++playerState.freezeFrameDuration;
                    }

                    // Update the current frame
                    if (playerState.freezeFrame != NONE
                        && playerState.currFrame + 1 >= playerState.freezeFrame
                        && playerState.freezeFrameDuration > 0)
                    {
                        --playerState.freezeFrameDuration;
                        playerState.currFrame = playerState.freezeFrame;
                    }
                    else
                        ++playerState.currFrame;

                    // Handels when player is attacking
                    if (playerState.busy && playerState.isAttacking
                        && (playerState.currFrame % character.sprite[playerState.state].frameCount) == character.sprite[playerState.state].frameCount / 3 )
                    {
                        auto& [x, y] = entity.get<Position>();
                        if (playerState.isSpecicalAttacking)
                            createSpecialAttack( x, y, SpecialAttacks::FIREBALL,
                                      playerState.playerNumber, playerState.direction, character);
                        else
                            createAttack( x, y, playerState.state,
                                      playerState.playerNumber, playerState.direction);
                    }
                }
            }
    }

    void MK::InputSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Inputs>()
            .build();

        SDL_PumpEvents();
        auto keyboardState = SDL_GetKeyboardState(nullptr);

        if (keyboardState[SDL_SCANCODE_ESCAPE])
        {
            exit(0);
        }

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
                auto& inputs = entity.get<Inputs>();
                auto& playerState = entity.get<PlayerState>();

                inputs++;

                inputs[0] |= (playerState.direction == PlayerState::LEFT) ?
                                                Inputs::DIRECTION_LEFT : Inputs::DIRECTION_RIGHT;
                inputs[0] |= (playerState.isJumping) ?
                                                Inputs::JUMPING : 0;

                // Player 1 controls (using WASD for movement, space, etc. for actions)
                if (playerState.playerNumber == 1) {
                    inputs[0] |=
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
                    inputs[0] |=
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
            }
        }
    }

    void MK::CollisionSystem() {

        static const bagel::Mask maskAttack = bagel::MaskBuilder()
            .set<Attack>()
            .build();

        static const bagel::Mask maskPlayer = bagel::MaskBuilder()
            .set<PlayerState>()
            .build();


        b2World_Step(boxWorld, BOX2D_STEP, 4);

        const auto se = b2World_GetSensorEvents(boxWorld);

        for (int i = 0; i < se.beginCount; ++i) {
            if (!b2Shape_IsValid(se.beginEvents[i].visitorShapeId)) continue;
            b2BodyId b = b2Shape_GetBody(se.beginEvents[i].visitorShapeId);
            const auto* e_b = static_cast<bagel::ent_type*>(b2Body_GetUserData(b));
            if (!b2Shape_IsValid(se.beginEvents[i].sensorShapeId)) continue;
            b2BodyId s = b2Shape_GetBody(se.beginEvents[i].sensorShapeId);
            const auto* e_s = static_cast<bagel::ent_type*>(b2Body_GetUserData(s));
            if (!e_b || !e_s) continue;

            bagel::Entity eBody = bagel::Entity{(*e_s)};
            bagel::Entity eSensor = bagel::Entity{(*e_b)};

            if (eBody.test(maskPlayer) && eSensor.test(maskPlayer))
                eBody.get<Collider>().isSensor = true;

            if (eSensor.test(maskAttack) && eBody.test(maskPlayer)
                && eSensor.get<Attack>().attacker != eBody.get<PlayerState>().playerNumber)
                    CombatSystem(eSensor, eBody);
        }

        // Handle end events
        for (int i = 0; i < se.endCount; ++i) {
            if (!b2Shape_IsValid(se.endEvents[i].visitorShapeId)) continue;
            b2BodyId b = b2Shape_GetBody(se.endEvents[i].visitorShapeId);
            const auto* e_b = static_cast<bagel::ent_type*>(b2Body_GetUserData(b));
            if (!b2Shape_IsValid(se.endEvents[i].sensorShapeId)) continue;
            b2BodyId s = b2Shape_GetBody(se.endEvents[i].sensorShapeId);
            const auto* e_s = static_cast<bagel::ent_type*>(b2Body_GetUserData(s));
            if (!e_b || !e_s) continue;

            bagel::Entity eBody = bagel::Entity{(*e_s)};
            bagel::Entity eSensor = bagel::Entity{(*e_b)};

            if (eBody.test(maskPlayer) && eSensor.test(maskPlayer))
                eBody.get<Collider>().isSensor = false;
        }
    }

    void MK::MatchSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Health>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {

            }
        }
    }

    void MK::WinSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Score>()
            .build();
        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
            }
        }
    }

    void MK::ClockSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Time>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
            }
        }
    }

    void MK::CombatSystem(bagel::Entity &eAttack, bagel::Entity &ePlayer) {
        auto& attack = eAttack.get<Attack>();
        auto& playerState = ePlayer.get<PlayerState>();
        auto& health = ePlayer.get<Health>();
        auto& character = ePlayer.get<Character>();

        // Blocking and evading attacks
        if (playerState.state == State::CROUCH_BLOCK || playerState.state == State::GETUP
            || playerState.isLaying || (playerState.state == State::BLOCK
                && attack.type != State::LOW_SWEEP_KICK && attack.type != State::CROUCH_KICK))
        {
            health.health -= 1;
            --(playerState.currFrame);
            return;
        }

        bool crouching = playerState.isCrouching;

        switch (attack.type)
        {
            case State::LOW_PUNCH:
                health.health -= 5;
                playerState.reset();
                playerState.state = crouching ? State::CROUCH_HIT : State::TORSO_HIT ;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.busy = true;
                break;
            case State::HIGH_PUNCH:
                health.health -= 5;
                playerState.reset();
                playerState.state = crouching ? State::CROUCH_HIT : State::HEAD_HIT;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.busy = true;
                break;
            case State::LOW_KICK:
                health.health -= 8;
                playerState.reset();
                playerState.state = crouching ? State::CROUCH_HIT : State::KICKBACK_TORSO_HIT;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.busy = true;
                break;
            case State::HIGH_KICK:
                health.health -= 8;
                playerState.reset();
                playerState.state = crouching ? State::CROUCH_HIT : State::HEAD_HIT;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.busy = true;
                break;
            case State::LOW_SWEEP_KICK:
                health.health -= 12;
                playerState.reset();
                playerState.state = State::FALL_INPLACE;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.freezeFrame = playerState.busyFrames - 1;
                playerState.freezeFrameDuration = 2;
                playerState.isLaying = true;
                playerState.busy = true;
                break;
            case State::HIGH_SWEEP_KICK:
                health.health -= 14;
                playerState.reset();
                playerState.state = State::FALL;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.freezeFrame = playerState.busyFrames - 1;
                playerState.freezeFrameDuration = 2;
                playerState.isLaying = true;
                playerState.busy = true;
                break;
            case State::UPPERCUT:
                health.health -= 14;
                playerState.reset();
                playerState.state = State::UPPERCUT_HIT;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.freezeFrame = playerState.busyFrames - 1;
                playerState.freezeFrameDuration = 2;
                playerState.isLaying = true;
                playerState.busy = true;
                break;
            case State::CROUCH_KICK:
                health.health -= 7;
                playerState.reset();
                playerState.state = crouching ? State::CROUCH_HIT : State::TORSO_HIT;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.freezeFrame = playerState.busyFrames - 1;
                playerState.freezeFrameDuration = 2;
                playerState.busy = true;
                break;
            default:
        }
    }

    void MK::AttackSystem(bagel::Entity &ePlayer)
    {

    }

    void MK::AttackDecaySystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Collider>()
            .set<Time>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            if (bagel::Entity entity{e}; entity.test(mask)) {
                if (entity.has<Attack>()) {
                    auto& collider = entity.get<Collider>();

                    // Only delete the user data if the body is valid
                    if (b2Body_IsValid(collider.body)) {
                        const auto* e_p = static_cast<bagel::ent_type*>(b2Body_GetUserData(collider.body));
                        b2DestroyBody(collider.body);
                        delete e_p;
                    }

                    // Mark the body as invalid after deletion
                    collider.body = b2_nullBodyId;

                    bagel::World::destroyEntity(e);
                }
                else if (entity.has<SpecialAttack>()) {
                    auto& collider = entity.get<Collider>();
                    auto& time = entity.get<Time>();

                    if (++(time.time) > SpecialAttack::SPECIAL_ATTACK_LIFE_TIME) {
                        if (b2Body_IsValid(collider.body)) {
                            const auto* e_p = static_cast<bagel::ent_type*>(b2Body_GetUserData(collider.body));
                            b2DestroyBody(collider.body);
                            delete e_p;
                        }

                        collider.body = b2_nullBodyId;
                        bagel::World::destroyEntity(e);
                    }
                }
            }
        }
    }

    int MK::SpecialAttackSystem(const bagel::Entity& ePlayer) {
        return 0;
    }

    void MK::createPlayer(float x, float y, Character character, int playerNumber) {

            // Construct the texture path
            std::string texturePath = "res/" + std::string(character.name) + ".png";
            auto texture = TextureSystem::getTexture(ren, texturePath, true);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_kinematicBody;
            bodyDef.position= getPosition(x, y);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;

            b2Polygon boxShape = b2MakeBox((CHARACTER_WIDTH / 2.0f) / WINDOW_SCALE,
                                            (CHARACTER_HEIGHT / 2.0f) / WINDOW_SCALE);

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &boxShape);

            // Add components to the entity
            PlayerState playerState;
            playerState.playerNumber = playerNumber;
            playerState.direction = (playerNumber == 1) ? PlayerState::RIGHT : PlayerState::LEFT;

            bagel::Entity entity = bagel::Entity::create();
            entity.addAll(Position{x, y},
                          Movement{0, 0},
                          Collider{body, shape},
                          Texture{texture},
                          playerState,
                          Inputs{},
                          character,
                          Health{100, 100});

            b2Body_SetUserData(body, new bagel::ent_type{entity.entity()});
        }

        void MK::createAttack(float x, float y, State type, int playerNumber, bool direction) {

            float width = 0.0f;
            float height = 0.0f;
            float xOffset = 0.0f;
            float yOffset = 0.0f;

            switch (type)
            {
                case State::LOW_PUNCH:
                case State::HIGH_PUNCH:
                    width = 70.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == PlayerState::LEFT ? -1.0f : 1.0f);
                    yOffset = 40.0f;
                    break;
                case State::LOW_KICK:
                case State::HIGH_KICK:
                    width = 95.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == PlayerState::LEFT ? -1.0f : 1.0f);
                    yOffset = 0.0f;
                    break;
                case State::HIGH_SWEEP_KICK:
                    width = 95.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == PlayerState::LEFT ? -1.0f : 1.0f);
                    yOffset = 40.0f;
                    break;
                case State::CROUCH_KICK:
                case State::LOW_SWEEP_KICK:
                    width = 85.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == PlayerState::LEFT ? -1.0f : 1.0f);
                    yOffset = -40.0f;
                    break;
                case State::UPPERCUT:
                    width = 50.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == PlayerState::LEFT ? -1.0f : 1.0f);
                    yOffset = 40.0f;
                    break;
                default: // Type is not a valid attack
                    return;
            }

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_kinematicBody;
            bodyDef.position= getPosition(x + xOffset, y + yOffset);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;

            b2Polygon boxShape = b2MakeBox((width / 2.0f) / WINDOW_SCALE,
                                           (height / 2.0f) / WINDOW_SCALE);

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &boxShape);

            bagel::Entity entity = bagel::Entity::create();
            entity.addAll(Position{x, y},
                          Collider{body, shape},
                          Attack{type, playerNumber});

            b2Body_SetUserData(body, new bagel::ent_type{entity.entity()});
        }

        void MK::createSpecialAttack(float x, float y, SpecialAttacks type, int playerNumber,
                                    bool direction, Character& character)
        {
            // Construct the texture path
            std::string texturePath = "res/" + std::string(character.name) + ".png";
            auto texture = TextureSystem::getTexture(ren, texturePath, true);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_kinematicBody;
            bodyDef.position= getPosition(x, y);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;

            b2Polygon boxShape = b2MakeBox((character.specialAttackSprite[SpecialAttacks::FIREBALL].h / 2.0f) / WINDOW_SCALE,
                                         (character.specialAttackSprite[SpecialAttacks::FIREBALL].w / 2.0f) / WINDOW_SCALE);

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &boxShape);

            // Add components to the entity
            bagel::Entity entity = bagel::Entity::create();
            entity.addAll(Position{x + (direction == PlayerState::LEFT) ? 100.0f: -100, y + 120},
                       Movement{(direction == PlayerState::LEFT) ? -15.0f : 15.0f, 0},
                       Collider{body, shape},
                       Texture{texture},
                       SpecialAttack{type, playerNumber, direction},
                       character,
                       Time{});

            b2Body_SetUserData(body, new bagel::ent_type{entity.entity()});
        }

        void MK::createBoundary(float x, float y, float width, float height) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Position{x, y},
                          Collider{});
        }

        void MK::createGameInfo(float initialTime) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Time{initialTime},
                          Score{0, 0, 0},
                          Position{0, 0},
                          Texture{nullptr, SDL_FRect{0, 0, 100, 50}});

        }

    void MK::createBackground(std::string backgroundPath) {

        // Load the image as a surface
        SDL_Surface* surface = IMG_Load(backgroundPath.c_str());
        if (!surface) {
            SDL_Log("Failed to load image: %s, SDL_Error: %s", backgroundPath.c_str(), SDL_GetError());
            return;
        }

        const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);

        SDL_SetSurfaceColorKey(surface, true, SDL_MapRGB(fmt, nullptr,
                                                      BACKGROUND_COLOR_IGNORE_RED,
                                                      BACKGROUND_COLOR_IGNORE_GREEN,
                                                      BACKGROUND_COLOR_IGNORE_BLUE));

        // Create a texture from the surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
        SDL_DestroySurface(surface); // Free the surface after creating the texture

        if (!texture) {
            SDL_Log("Failed to create texture: %s, SDL_Error: %s", backgroundPath.c_str(), SDL_GetError());
            return;
        }

        // Create fence
        bagel::Entity fence = bagel::Entity::create();
        fence.addAll(
            Position{0, 0},
            Texture{
                texture,
                { fenceX, fenceY, fenceW, fenceH }, // Only show the red/black part
                { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 1.3f} // Stretch or place as needed
            }
        );

        // Create temple
        bagel::Entity temple = bagel::Entity::create();
        temple.addAll(
            Position{0, 0},
            Texture{
                texture,
                { templeX, templeY, templeW, templeH }, // Only show the red/black part
                { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT } // Stretch to fit window
            }
        );
    }
}