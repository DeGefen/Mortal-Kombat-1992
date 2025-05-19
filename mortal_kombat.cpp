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

    // ------------------------------- Game Loop -------------------------------

    void MK::run()
    {

        Uint32 frameStart, frameTime;
        int frame_count = 0;

        createBackground("res/Background.png");

        //createBoundary(0, 0, LEFT);
        //createBoundary(WINDOW_WIDTH, 0, RIGHT);
        bagel::Entity player1 = createPlayer(PLAYER_1_BASE_X, PLAYER_BASE_Y, Characters::SUBZERO, 1);
        bagel::Entity player2 = createPlayer(PLAYER_2_BASE_X, PLAYER_BASE_Y, Characters::LIU_KANG, 2);

        createBar(player1, player2);

        while (true)
        {
            frameStart = SDL_GetTicks();

            if (frame_count % INPUT_FRAME_DELAY == 0) InputSystem();
            if (++frame_count % ACTION_FRAME_DELAY == 0) PlayerSystem();
            ClockSystem();
            CollisionSystem();
            SpecialAttackSystem();
            MovementSystem();
            RenderSystem();
            HealthBarSystem();
            AttackDecaySystem();

            frameTime = SDL_GetTicks() - frameStart;
            if (FRAME_DELAY > frameTime) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }
    }

    // ------------------------------- Systems -------------------------------

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
                                        * (playerState.direction == LEFT ? 1.0f : -1.0f)
                                        * (collider.isBoundarySensor ? 0.0f : 1.0f);

                        break;
                    case State::WALK_FORWARDS:
                        movement.vx = WALK_SPEED_FORWARDS
                                        * (playerState.direction == LEFT ? -1.0f : 1.0f)
                                        * (collider.isPlayerSensor ? 0.0f : 1.0f);
                        break;
                    case State::KICKBACK_TORSO_HIT:
                        movement.vx = KICKBACK_SPEED
                                        * (playerState.direction == LEFT ? 1.0f : -1.0f);
                        break;
                    case State::FALL:
                    case State::UPPERCUT_HIT:
                        if (playerState.currFrame < character.sprite[playerState.state].frameCount / 2)
                        {
                            movement.vx = FALL_SPEED
                                        * (playerState.direction == LEFT ? 1.0f : -1.0f);
                            break;
                        }
                    default:
                        movement.reset();
                        break;
                    }
                }

                position.x += movement.vx;
                position.y += movement.vy;

                if (entity.has<PlayerState>() && entity.get<PlayerState>().isCrouching)
                {
                    b2Body_SetTransform(
                            collider.body,
                            getPosition(position.x, position.y - (CHARACTER_HEIGHT/2.0f)),
                            b2Rot_identity);
                }
                else if (entity.has<SpecialAttack>())
                {
                    auto& sprite = entity.get<Character>().specialAttackSprite[entity.get<SpecialAttack>().type];
                    b2Body_SetTransform(
                            collider.body,
                            getPosition(position.x - sprite.w, position.y - entity.get<Character>().specialAttackOffset_y + (sprite.h / 2.0f)),
                            b2Rot_identity);
                }
                else
                {
                    b2Body_SetTransform(
                              collider.body,
                              getPosition(position),
                              b2Rot_identity);
                }
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

                    flipMode = (playerState.direction == LEFT) ?
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
                    flipMode = (specialAttack.direction == LEFT) ?
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
                        playerState.isSpecialAttack = special;
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
                    if (playerState.busy && playerState.isAttacking)
                    {
                        auto& [x, y] = entity.get<Position>();
                        if (playerState.isSpecialAttack
                            && (playerState.currFrame % character.sprite[playerState.state].frameCount) == character.sprite[playerState.state].frameCount / 2)
                            createSpecialAttack(x, y, SpecialAttacks::FIREBALL,
                                      playerState.playerNumber, playerState.direction, character);
                        else if ((playerState.currFrame % character.sprite[playerState.state].frameCount) == character.sprite[playerState.state].frameCount / 3)
                            createAttack(x, y, playerState.state,
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

                inputs[0] |= (playerState.direction == LEFT) ?
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
                eBody.get<Collider>().isPlayerSensor = true;

            if (eBody.test(maskPlayer) && eSensor.has<Boundary>())
                eBody.get<Collider>().isBoundarySensor = true;

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
                eBody.get<Collider>().isPlayerSensor = false;

            if (eBody.test(maskPlayer) && eSensor.has<Boundary>())
                eBody.get<Collider>().isBoundarySensor = false;
        }
    }

    void MK::MatchSystem() {
        // static const bagel::Mask mask = bagel::MaskBuilder()
        //     .set<Health>()
        //     .set<Character>()
        //     .build();
        //
        // for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        // {
        //     if (bagel::Entity entity{e}; entity.test(mask)) {
        //         auto& health = entity.get<Health>();
        //         auto& character = entity.get<Character>();
        //
        //         if (health.health <= 0) {
        //             // Create win message
        //             std::string winText = std::string(character.name) + " wins!";
        //             SDL_Color white = {255, 255, 255, 255};
        //
        //             SDL_Texture* textTex = renderText(ren, winText, white, yourTTFfont);
        //             if (!textTex) continue;
        //
        //             bagel::Entity msg = bagel::Entity::create();
        //             msg.addAll(
        //                 Position{WINDOW_WIDTH / 2.0f - 100, WINDOW_HEIGHT / 4.0f},
        //                 Texture{
        //                     textTex,
        //                     {0, 0, 300, 50},               // Source size — match your text size
        //                     {WINDOW_WIDTH / 2.0f - 100, WINDOW_HEIGHT / 4.0f, 300, 50}
        //                 },
        //                 WinMessage{winText}
        //             );
        //
        //             break; // stop once one winner is declared
        //         }
        //     }
        // }
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
                --entity.get<Time>().time;
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
                playerState.busy = true;
                break;
            case State::SPECIAL_1:
            case State::SPECIAL_2:
            case State::SPECIAL_3:
                health.health -= 10;
                playerState.reset();
                playerState.state = crouching ? State::CROUCH_HIT : State::TORSO_HIT;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.freezeFrame = playerState.busyFrames - 1;
                playerState.busy = true;
                if (eAttack.has<SpecialAttack>())
                {
                    eAttack.get<SpecialAttack>().explode = true;
                }
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
            .set<Attack>()
            .set<Time>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            if (bagel::Entity entity{e}; entity.test(mask)) {
                auto& collider = entity.get<Collider>();
                auto& time = entity.get<Time>();

                if (time.time <= 0) {
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

    int MK::SpecialAttackSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<SpecialAttack>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
                if (entity.get<SpecialAttack>().explode)
                {
                    auto& spritePrev = entity.get<Character>().specialAttackSprite[entity.get<SpecialAttack>().type];
                    auto& spriteNext = entity.get<Character>().specialAttackSprite[SpecialAttacks::EXPLOSION];
                    entity.get<Movement>().reset();
                    entity.get<Position>().y -= ((spriteNext.h - spritePrev.h) / 2.0f) * SCALE_CHARACTER;
                    if (entity.get<SpecialAttack>().direction == RIGHT)
                    {
                        entity.get<Position>().x += (spriteNext.w / 3.0f) * SCALE_CHARACTER;
                    }
                    entity.get<SpecialAttack>().type = SpecialAttacks::EXPLOSION;
                    entity.get<SpecialAttack>().frame = 0;
                    entity.get<SpecialAttack>().totalFrames = spriteNext.frameCount - 1;
                    entity.get<SpecialAttack>().explode = false;
                    entity.get<Time>().time = 4;
                }
            }
        }
    }

    void MK::HealthBarSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<HealthBarReference>()
            .set<DamageVisual>()
            .set<Texture>()
            .set<Position>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            if (bagel::Entity entity{e}; entity.test(mask)) {

                // Use HealthBarReference to access actual player health
                auto& reference = entity.get<HealthBarReference>();
                if (reference.target.id == -1) continue;

                bagel::Entity player = bagel::Entity{reference.target};
                auto& health = player.get<Health>();

                // Continue as usual
                auto& damage = entity.get<DamageVisual>();
                auto& texture = entity.get<Texture>();
                auto& pos = entity.get<Position>();

                float ratio = std::max(0.0f, health.health / health.max_health);

                // Smooth trailing damage effect
                const float TRAIL_SPEED = 0.5f;
                if (damage.trailingHealth > health.health) {
                    damage.trailingHealth -= TRAIL_SPEED;
                    if (damage.trailingHealth < health.health)
                        damage.trailingHealth = health.health;
                }

                // float trailRatio = std::max(0.0f, damage.trailingHealth / health.max_health);

                // // Red bar (trailing)
                // SDL_FRect redRect = {
                //     pos.x, pos.y,
                //     250.0f * trailRatio,
                //     texture.rect.h
                // };
                // SDL_SetRenderDrawColor(ren, 200, 0, 0, 255);
                // SDL_RenderFillRect(ren, &redRect);

                // Green bar (current health)
                texture.rect.w = 250.0f * ratio;
            }
        }
    }

    // ------------------------------- Helper Functions -------------------------------


    // ------------------------------- Create Functions -------------------------------

    bagel::ent_type MK::createPlayer(float x, float y, Character character, int playerNumber) {

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
            playerState.direction = (playerNumber == 1) ? RIGHT : LEFT;

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
            return entity.entity();
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
                    xOffset = width / 2.0f * (direction == LEFT ? -1.0f : 1.0f);
                    yOffset = 40.0f;
                    break;
                case State::LOW_KICK:
                case State::HIGH_KICK:
                    width = 95.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == LEFT ? -1.0f : 1.0f);
                    yOffset = 0.0f;
                    break;
                case State::HIGH_SWEEP_KICK:
                    width = 95.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == LEFT ? -1.0f : 1.0f);
                    yOffset = 40.0f;
                    break;
                case State::CROUCH_KICK:
                case State::LOW_SWEEP_KICK:
                    width = 85.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == LEFT ? -1.0f : 1.0f);
                    yOffset = -40.0f;
                    break;
                case State::UPPERCUT:
                    width = 50.0f;
                    height = 40.0f;
                    xOffset = width / 2.0f * (direction == LEFT ? -1.0f : 1.0f);
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
                          Attack{type, playerNumber},
                          Time{Attack::ATTACK_LIFE_TIME});

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
            bodyDef.position= getPosition(x, y + CHARACTER_HEIGHT / 2.0f);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;

            b2Polygon boxShape = b2MakeBox((character.specialAttackSprite[type].h / 2.0f) / WINDOW_SCALE,
                                         (character.specialAttackSprite[type].w / 2.0f) / WINDOW_SCALE);

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &boxShape);

            float width = 0.0f;
            float height = 0.0f;
            float xOffset = 0.0f;
            float yOffset = 0.0f;

            State state;
            switch (type)
            {
                case SpecialAttacks::FIREBALL:
                    state = State::SPECIAL_1;
                    break;
                    // add more special attacks here
                default:
                    return; // Invalid special attack type
            }

            // Add components to the entity
            bagel::Entity entity = bagel::Entity::create();
            entity.addAll(Position{x + ((CHAR_SQUARE_WIDTH / 2.0f) * SCALE_CHARACTER),
                                    y + ((character.specialAttackOffset_y - (character.specialAttackSprite[type].h / 2.0f)) * SCALE_CHARACTER)},
                       Movement{(direction == LEFT) ? -15.0f : 15.0f, 0},
                       Collider{body, shape},
                       Texture{texture},
                       Attack{state, playerNumber},
                       SpecialAttack{type, direction},
                       character,
                       Time{SpecialAttack::SPECIAL_ATTACK_LIFE_TIME});

            b2Body_SetUserData(body, new bagel::ent_type{entity.entity()});
        }

        void MK::createBoundary(float x, float y, bool side) {
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_staticBody;
            if (side == LEFT)
            {
                bodyDef.position= getPosition(x - BOUNDARY_WIDTH / 1.2f, y);
            }

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;

            b2Polygon boxShape = b2MakeBox(BOUNDARY_WIDTH / 2.0f / WINDOW_SCALE,
                                            WINDOW_HEIGHT / 2.0f / WINDOW_SCALE);

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &boxShape);

            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Collider{body, shape},
                          Boundary{});

            b2Body_SetUserData(body, new bagel::ent_type{entity.entity()});
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

    void MK::createBar(bagel::Entity player1, bagel::Entity player2) {

        // Dimensions of the green health bar in the texture
        SDL_FRect GREEN_BAR_SRC = { 5406, 49, 163, 12 };  // Green (top bar)
        SDL_FRect RED_BAR_SRC   = { 5406, 63, 163, 12 }; // Red  (bottom bar)
        SDL_FRect NAME_BAR_SRC   = { 5406, 98, 163, 12 }; // Name bar

        // Bar dimensions
        const float BAR_WIDTH = 250.0f;
        const float BAR_HEIGHT = 18.0f;

        // Positioning constants
        const float OFFSET_Y = 10.0f;
        const float MARGIN = 50.0f;
        const float xRight = WINDOW_WIDTH - BAR_WIDTH - MARGIN;

        // Color key for the bar texture
        const Uint8 COLOR_KEY_DAMAGE_BAR_RED = 82;
        const Uint8 COLOR_KEY_DAMAGE_BAR_GREEN = 1;
        const Uint8 COLOR_KEY_DAMAGE_BAR_BLUE = 1;

        // Color key for the bar texture
        const Uint8 COLOR_KEY_NAME_BAR_RED = 0;
        const Uint8 COLOR_KEY_NAME_BAR_GREEN = 165;
        const Uint8 COLOR_KEY_NAME_BAR_BLUE = 0;

        // Load the image as a surface
        SDL_Surface* original = IMG_Load("res/Menus & Text.png");
        if (!original) {
            SDL_Log("Failed to load bar image: %s", SDL_GetError());
            return;
        }

        // Duplicate the surface for damage bar
        SDL_Surface* surfaceDamage = SDL_DuplicateSurface(original);
        if (!surfaceDamage) {
            SDL_Log("Failed to duplicate surface for damage bar: %s", SDL_GetError());
            SDL_DestroySurface(original);
            return;
        }

        // Duplicate the surface for name bar
        SDL_Surface* surfaceName = SDL_DuplicateSurface(original);
        if (!surfaceName) {
            SDL_Log("Failed to duplicate surface for name bar: %s", SDL_GetError());
            SDL_DestroySurface(original);
            SDL_DestroySurface(surfaceDamage);
            return;
        }

        // Free the original
        SDL_DestroySurface(original);

        // Set the color key for the damage bar surface
        const SDL_PixelFormatDetails *damage_fmt = SDL_GetPixelFormatDetails(surfaceDamage->format);

        SDL_SetSurfaceColorKey(surfaceDamage, true, SDL_MapRGB(damage_fmt, nullptr,
                                                      COLOR_KEY_DAMAGE_BAR_RED,
                                                      COLOR_KEY_DAMAGE_BAR_GREEN,
                                                      COLOR_KEY_DAMAGE_BAR_BLUE));

        // Set the color key for the name bar surface
        const SDL_PixelFormatDetails *name_fmt = SDL_GetPixelFormatDetails(surfaceDamage->format);

        SDL_SetSurfaceColorKey(surfaceName, true, SDL_MapRGB(name_fmt, nullptr,
                                                      COLOR_KEY_NAME_BAR_RED,
                                                      COLOR_KEY_NAME_BAR_GREEN,
                                                      COLOR_KEY_NAME_BAR_BLUE));

        // Create textures
        SDL_Texture* barTexture = SDL_CreateTextureFromSurface(ren, surfaceDamage);
        if (!barTexture) {
            SDL_Log("Failed to create damage bar texture: %s", SDL_GetError());
            // Cleanup
            SDL_DestroySurface(surfaceDamage);
            SDL_DestroySurface(surfaceName);
            return;
        }

        SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(ren, surfaceName);
        if (!nameTexture) {
            SDL_Log("Failed to create name bar texture: %s", SDL_GetError());
            SDL_DestroySurface(surfaceDamage);
            SDL_DestroySurface(surfaceName);
            SDL_DestroyTexture(barTexture);
            return;
        }

        // Cleanup surfaces after texture creation
        SDL_DestroySurface(surfaceDamage);
        SDL_DestroySurface(surfaceName);

        // Player 1 - RED background bar
        bagel::Entity red1 = bagel::Entity::create();
        red1.addAll(
            Position{ MARGIN, OFFSET_Y },
            Texture{
                barTexture,
                RED_BAR_SRC,
                SDL_FRect{ MARGIN, OFFSET_Y, BAR_WIDTH, BAR_HEIGHT }
            }
        );

        // Player 1 - GREEN health bar
        bagel::Entity green1 = bagel::Entity::create();
        green1.addAll(
            Position{ MARGIN, OFFSET_Y },
            Texture{
                barTexture,
                GREEN_BAR_SRC,
                SDL_FRect{ MARGIN, OFFSET_Y, BAR_WIDTH, BAR_HEIGHT }
            },
            DamageVisual{100.0f},
            HealthBarReference{ player1.entity() }
        );

        // Player 1 - Player's name
        bagel::Entity name1 = bagel::Entity::create();
        name1.addAll(
            Position{ MARGIN, OFFSET_Y },
            Texture{
                nameTexture,
                NAME_BAR_SRC,
                SDL_FRect{ MARGIN, OFFSET_Y, BAR_WIDTH, BAR_HEIGHT }
            }
        );

        // Player 2 - RED background bar
        bagel::Entity red2 = bagel::Entity::create();
        red2.addAll(
            Position{ xRight, OFFSET_Y },
            Texture{
                barTexture,
                RED_BAR_SRC,
                SDL_FRect{ xRight, OFFSET_Y, BAR_WIDTH, BAR_HEIGHT }
            }
        );

        // Player 2 - GREEN health bar
        bagel::Entity green2 = bagel::Entity::create();
        green2.addAll(
            Position{ xRight, OFFSET_Y },
            Texture{
                barTexture,
                GREEN_BAR_SRC,
                SDL_FRect{ xRight, OFFSET_Y, BAR_WIDTH, BAR_HEIGHT }
            },
            DamageVisual{100.0f},
            HealthBarReference{ player2.entity() }
        );

        // Player 2 - Player's name
        bagel::Entity name2 = bagel::Entity::create();
        name2.addAll(
            Position{ xRight, OFFSET_Y },
            Texture{
                nameTexture,
                NAME_BAR_SRC,
                SDL_FRect{ xRight, OFFSET_Y, BAR_WIDTH, BAR_HEIGHT }
            }
        );
    }


}