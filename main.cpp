#include "mortal_kombat.h"
#include "mortal_kombat_info.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include <iostream>

int main() {
    mortal_kombat::RenderSystem renderSystem;
    mortal_kombat::createPlayer(100, 100, (mortal_kombat::Characters::SUBZERO), mortal_kombat::RenderSystem::getRenderer());
    while (true)
    {
        mortal_kombat::InputSystem::run();
        mortal_kombat::PlayerSystem::run();
        mortal_kombat::MovementSystem::run();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                exit(0);
            }
        }
        SDL_RenderClear(mortal_kombat::RenderSystem::getRenderer());
        renderSystem.run();
        SDL_RenderPresent(mortal_kombat::RenderSystem::getRenderer());

        SDL_Delay(70);
    }
}