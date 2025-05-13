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

        prepareBoxWorld();
    }

    MK::~MK()
    {
        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id) {
            if (bagel::Entity entity{e}; entity.has<Texture>()) {
                auto& texture = entity.get<Texture>();
                if (texture.tex != nullptr) {
                    SDL_DestroyTexture(texture.tex);
                }
            }
        }
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
        createPlayer(PLAYER_1_BASE_X, PLAYER_BASE_Y, (Characters::SUBZERO), 1);
        createPlayer(PLAYER_2_BASE_X, PLAYER_BASE_Y, (Characters::LIU_KANG), 2);

        while (true)
        {
            InputSystem();
            PlayerSystem();
            CollisionSystem();
            MovementSystem();
            RenderSystem();
            AttackDecaySystem();
            SDL_Delay(70);
        }
    }

    void MK::MovementSystem()
    {
        static constexpr float WALK_SPEED_BACKWARDS = 4.0f;
        static constexpr float WALK_SPEED_FORWARDS = 5.0f;
        static constexpr float KICKBACK_SPEED = 4.0f;
        static constexpr float UPPERCUT_SPEED = 2.0f;
        static constexpr float UPPERCUT_VERTICAL_SPEED = 2.0f;

        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Position>()
            .set<Movement>()
            .build();

        static const bagel::Mask maskPlayer = bagel::MaskBuilder()
            .set<PlayerState>()
            .set<Character>()
            .set<Collider>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {            auto& position = entity.get<Position>();
                auto& movement = entity.get<Movement>();



                if (entity.test(maskPlayer))
                {
                    auto& playerState = entity.get<PlayerState>();
                    auto& character = entity.get<Character>();
                    auto& collider = entity.get<Collider>();

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
                    case State::UPPERCUT_HIT:
                        if (playerState.currFrame < character.sprite[playerState.state].frameCount / 2)
                        {
                            movement.vy = UPPERCUT_VERTICAL_SPEED;
                            movement.vx = UPPERCUT_SPEED
                                        * (playerState.direction == PlayerState::LEFT ? 1.0f : -1.0f);
                        }
                        else
                            movement.vy = -UPPERCUT_VERTICAL_SPEED;

                        break;
                    case State::GETUP:
                        movement.vy = PLAYER_BASE_Y - position.y ;
                        break;
                    default:
                        movement.reset();
                        break;
                    }
                    position.x += movement.vx;
                    position.y += movement.vy;

                    b2Body_SetTransform(collider.body, getPosition(position), b2Rot_identity);
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
            }
        }

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

                    State state;
                    int freezeFrame = NONE;
                    int offSetFrame = 0;
                    int freezeFrameDuration = 0;
                    bool busy = true;
                    bool crouching = false;
                    bool attack = false;

                    if (inputs.test(Inputs::JUMP_PUNCH))
                    {
                        state = State::JUMP_PUNCH;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::JUMP_LOW_KICK))
                    {
                        state = State::JUMP_LOW_KICK;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::JUMP_HIGH_KICK))
                    {
                        state = State::JUMP_HIGH_KICK;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::CROUCH_BLOCK))
                    {
                        state = State::CROUCH_BLOCK;
                        freezeFrame = character.sprite[state].frameCount / 2 + 1;
                        freezeFrameDuration = 1;
                        crouching = true;
                    }
                    else if (inputs.test(Inputs::BLOCK))
                    {
                        state = State::BLOCK;
                        freezeFrame = character.sprite[state].frameCount / 2 + 1;
                        freezeFrameDuration = 1;
                    }
                    else if (inputs.test(Inputs::CROUCH_KICK))
                    {
                        state = State::CROUCH_KICK;
                        crouching = true;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::UP))
                    {
                        state = State::JUMP;
                    }
                    else if (inputs.test(Inputs::HIGH_SWEEP_KICK_LEFT)
                            || inputs.test(Inputs::HIGH_SWEEP_KICK_RIGHT))
                    {
                        state = State::HIGH_SWEEP_KICK;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::LOW_SWEEP_KICK_LEFT)
                            || inputs.test(Inputs::LOW_SWEEP_KICK_RIGHT))
                    {
                        state = State::LOW_SWEEP_KICK;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::UPPERCUT))
                    {
                        state = State::UPPERCUT;
                        crouching = true;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::DOWN))
                    {
                        state = State::CROUCH;
                        freezeFrame = (character.sprite[state].frameCount / 2) + 1;
                        freezeFrameDuration = 1;
                        crouching = true;
                    }
                    else if (inputs.test(Inputs::LOW_PUNCH))
                    {
                        state = State::LOW_PUNCH;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::HIGH_PUNCH))
                    {
                        state = State::HIGH_PUNCH;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::LOW_KICK))
                    {
                        state = State::LOW_KICK;
                        attack = true;
                    }
                    else if (inputs.test(Inputs::HIGH_KICK))
                    {
                        state = State::HIGH_KICK;
                        attack = true;
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

                    if (playerState.busyFrames - 1 <= playerState.currFrame
                        && playerState.freezeFrameDuration <= 0)
                        playerState.busy = false;

                    if (playerState.isLaying && !playerState.busy)
                    {
                        playerState.reset();
                        playerState.state = State::GETUP;
                        playerState.busyFrames = character.sprite[playerState.state].frameCount;
                        playerState.busy = true;
                    }

                    if ((!playerState.busy && state != playerState.state)
                        || playerState.state == State::CROUCH && crouching
                        && state != State::CROUCH)
                    {
                        playerState.reset();
                        playerState.state = state;
                        playerState.currFrame = (playerState.isCrouching && state == State::CROUCH) ? 2 : 0;
                        playerState.busyFrames = character.sprite[playerState.state].frameCount;
                        playerState.freezeFrame = freezeFrame;d
                        playerState.freezeFrameDuration = freezeFrameDuration;
                        playerState.isCrouching = crouching;
                        playerState.isAttacking = attack;
                        playerState.busy = busy;
                    }

                    if (playerState.freezeFrame != NONE && state == playerState.state)
                    {
                        playerState.freezeFrameDuration = 1;
                    }

                    if (playerState.freezeFrame != NONE
                        && playerState.currFrame + 1 >= playerState.freezeFrame
                        && playerState.freezeFrameDuration > 0)
                    {
                        --playerState.freezeFrameDuration;
                        playerState.currFrame = playerState.freezeFrame;
                    }
                    else
                        ++playerState.currFrame;



                    if (playerState.isAttacking && (playerState.currFrame % character.sprite[playerState.state].frameCount) == character.sprite[playerState.state].frameCount / 3 )
                    {
                        auto& position = entity.get<Position>();
                        createAttack(
                            playerState.direction == PlayerState::LEFT ? position.x - 20 : position.x + 20,
                            position.y, state, playerState.playerNumber);
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

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
                auto& inputs = entity.get<Inputs>();
                auto& playerState = entity.get<PlayerState>();

                inputs.index = (inputs.index + 1) % Inputs::MAX_HISTORY;
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

        if (playerState.state == State::BLOCK || playerState.state == State::CROUCH_BLOCK)
        {
            health.health -= 1;
            ++(playerState.currFrame);
            return;
        }

        switch (attack.type)
        {
            case State::LOW_KICK:
            case State::HIGH_KICK:
                health.health -= 8;
                playerState.reset();
                playerState.state = playerState.isCrouching ? State::CROUCH_HIT : State::KICKBACK_TORSO_HIT;
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
            default:
                health.health -= 5;
                playerState.reset();
                playerState.state = playerState.isCrouching ? State::CROUCH_HIT : State::TORSO_HIT ;
                playerState.busyFrames = character.sprite[playerState.state].frameCount;
                playerState.busy = true;
                break;
        }
    }

    void MK::AttackSystem(bagel::Entity &eAttack)
    {

    }

    void MK::AttackDecaySystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Attack>()
            .set<Collider>()
            .set<Time>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
                auto& collider = entity.get<Collider>();
                auto& time = entity.get<Time>();
                if (++(time.time) > Attack::ATTACK_LIFE_TIME)
                {
                    const auto* e_s = static_cast<bagel::ent_type*>(b2Body_GetUserData(collider.body));
                    b2DestroyBody(collider.body);
                    delete e_s;
                    bagel::World::destroyEntity(e);
                }

            }
        }
    }

    void MK::SpecialAttackSystem() {
        static const bagel::Mask mask = bagel::MaskBuilder()
            .set<Attack>()
            .set<Health>()
            .build();

        for (bagel::ent_type e = {0}; e.id <= bagel::World::maxId().id; ++e.id)
        {
            if (bagel::Entity entity{e}; entity.test(mask))
            {
            }
        }
    }

    void MK::createPlayer(float x, float y, Character character, int playerNumber) {

            // Construct the texture path
            std::string texturePath = "res/" + std::string(character.name) + ".png";

            // Load the image as a surface
            SDL_Surface* surface = IMG_Load(texturePath.c_str());
            if (!surface) {
                SDL_Log("Failed to load image: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
                return ;
            }

            const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);

            SDL_SetSurfaceColorKey(surface, true, SDL_MapRGB(fmt, nullptr,
                                                          COLOR_IGNORE_RED,
                                                          COLOR_IGNORE_GREEN,
                                                          COLOR_IGNORE_BLUE));

            // Create a texture from the surface
            SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
            SDL_DestroySurface(surface); // Free the surface after creating the texture
            if (!texture) {
                SDL_Log("Failed to create texture: %s, SDL_Error: %s", texturePath.c_str(), SDL_GetError());
                return ;
            }

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

        void MK::createAttack(float x, float y, State type, int playerNumber) {


            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_kinematicBody;
            bodyDef.position = getPosition(x, y);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;

            b2Polygon boxShape = b2MakeBox((CHARACTER_WIDTH / 2.0f) / WINDOW_SCALE,
                                           (CHARACTER_HEIGHT / 2.0f) / WINDOW_SCALE);

            b2BodyId body = b2CreateBody(boxWorld, &bodyDef);
            b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &boxShape);

            bagel::Entity entity = bagel::Entity::create();
            entity.addAll(Position{x, y},
                          Collider{body, shape},
                          Attack{type, playerNumber},
                          Time{});

            b2Body_SetUserData(body, new bagel::ent_type{entity.entity()});
        }

        void MK::createSpecialAttack(float x, float y, SpecialAttackType type) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Position{x, y},
                          Collider{},
                          SpecialAttack{type, 0.0f, 0.0f, 0, 0.0f, 0.0f});
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

        void MK::createBackground(SDL_Texture* texture) {
            bagel::Entity entity = bagel::Entity::create();

            entity.addAll(Position{0, 0},
                          Texture{texture, SDL_FRect{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}});

        }
}