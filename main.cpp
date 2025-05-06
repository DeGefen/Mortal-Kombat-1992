#include "char_info.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include <iostream>

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Mortal Kombat Animation", 1280, 720, 0);
    if (!window) {
        std::cerr << "Window could not be created: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Load character sprite sheet
    SDL_Texture* spriteSheet = IMG_LoadTexture(renderer, "path/to/sub_zero_sprites.png");
    if (!spriteSheet) {
        std::cerr << "Failed to load sprite sheet: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Precompute sprites for better performance
    mortal_kombat::precomputeCharacterRects("Sub_Zero");

    // Create animation sequence
    mortal_kombat::AnimationSequence sequence = mortal_kombat::createCombatSequence("Sub_Zero");

    // Character position
    int characterX = 500;
    int characterY = 400;
    bool facingLeft = false;

    // Main game loop
    bool quit = false;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    // Reset animation on spacebar
                    sequence.reset();
                } else if (event.key.keysym.sym == SDLK_f) {
                    // Toggle facing direction on F key
                    facingLeft = !facingLeft;
                }
            }
        }

        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        Uint32 delta = currentTime - lastTime;
        lastTime = currentTime;

        // Limit the frame rate by adding a small delay if needed
        if (delta < 16) {  // Target ~60 FPS
            SDL_Delay(16 - delta);
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the animation sequence
        mortal_kombat::renderAnimationSequence(
            renderer,
            spriteSheet,
            sequence,
            characterX,
            characterY,
            facingLeft
        );

        // If sequence is finished, reset it to loop
        if (sequence.isFinished()) {
            sequence.reset();
        }

        // Present renderer
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}



/*
 * while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

 */

