#include "character.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
#include <vector>

using namespace character;

MK::MK()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    if (!SDL_CreateWindowAndRenderer(
            "MK", 800,600, 0, &win, &ren)) {
        std::cout << SDL_GetError() << std::endl;
        return;
    }
    SDL_Surface *surf = IMG_Load("res/Sub-Zero.png");
    if (surf == nullptr) {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surf->format);

    SDL_SetSurfaceColorKey(surf, true, SDL_MapRGB(fmt, nullptr,
                                                  CharacterAnimations::COLOR_IGNORE_RED,
                                                  CharacterAnimations::COLOR_IGNORE_GREEN,
                                                  CharacterAnimations::COLOR_IGNORE_BLUE));

    tex = SDL_CreateTextureFromSurface(ren, surf);
    if (tex == nullptr) {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_DestroySurface(surf);
}

MK::~MK()
{
    if (tex != nullptr)
        SDL_DestroyTexture(tex);
    if (ren != nullptr)
        SDL_DestroyRenderer(ren);
    if (win != nullptr)
        SDL_DestroyWindow(win);

    SDL_Quit();
}

void MK::run()
{
    SDL_SetRenderDrawColor(ren, 30,30,30,255);
    SDL_FRect r{200,300,
                CharacterAnimations::CHAR_SQUARE_WIDTH * CharacterAnimations::SCALE_CHARACTER,
                CharacterAnimations::CHAR_SQUARE_HEIGHT * CharacterAnimations::SCALE_CHARACTER};

    SDL_Event event;
    SDL_FRect rect;
    std::vector<CharacterAnimations::Squence> seq = {
            {CharacterAnimations::Action::STANCE, 30},
            {CharacterAnimations::Action::WALK, 20},
            {CharacterAnimations::Action::LOW_PUNCH, 5},
            {CharacterAnimations::Action::STANCE, 1},
            {CharacterAnimations::Action::UPPERCUT, 5},
            {CharacterAnimations::Action::STANCE, 3},
            {CharacterAnimations::Action::LOW_PUNCH_SPREE, 10},
            {CharacterAnimations::Action::STANCE, 1},
            {CharacterAnimations::Action::HIGH_KICK, 10},
            {CharacterAnimations::Action::STANCE, 1},
            {CharacterAnimations::Action::LOWKICK_SWEEP, 8},
            {CharacterAnimations::Action::STANCE, 4},
            {CharacterAnimations::Action::WALK, 20, false, true},
            {CharacterAnimations::Action::TORSO_HIT, 5},
            {CharacterAnimations::Action::STANCE, 2},
            {CharacterAnimations::Action::HEAD_HIT, 5},
            {CharacterAnimations::Action::STANCE, 2},
            {CharacterAnimations::Action::UPPERCUT_HIT, 7},
            {CharacterAnimations::Action::GETUP, 5},
            {CharacterAnimations::Action::STANCE, 10},
    };


    for (auto & s : seq) {
        for (int j = 0; j < s.frame; ++j) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                return;
            }
        }

        int k = s.walkBack ? s.frame - j - 1 : j;
        if (s.action == CharacterAnimations::Action::WALK)
            r.x += (k % 7 > 3 ? 6.f : 3.f) * (s.walkBack ? (-1) : 1);

        if (s.action == CharacterAnimations::Action::TORSO_HIT)
            r.x += -1;

        if (s.action == CharacterAnimations::Action::UPPERCUT_HIT
            && j >= CharacterData::SUBZERO_SPRITE[static_cast<int>(CharacterAnimations::Action::UPPERCUT_HIT)].frameCount - 1)
            k = CharacterData::SUBZERO_SPRITE[static_cast<int>(CharacterAnimations::Action::UPPERCUT_HIT)].frameCount - 1;

        rect = CharacterAnimations::getFrame(CharacterAnimations::SUBZERO,
                                             s.action ,
                                             k);

        SDL_RenderClear(ren);
        SDL_RenderTextureRotated(
                ren, tex, &rect, &r, 0,
                nullptr, SDL_FLIP_NONE);
        SDL_RenderPresent(ren);

        SDL_Delay(75);
        }
    }
}
