#include "char_info.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <utility>

namespace mortal_kombat {

    void AnimationSequence::addStep(Action action, int durationFrames, bool loop) {
        steps.push_back({action, durationFrames, loop});

        // Initialize the first step if this is the first one
        if (steps.size() == 1) {
            stepFramesRemaining = durationFrames;
        }
    }

    SDL_Rect AnimationSequence::update() {
        if (steps.empty()) {
            return {0, 0, 0, 0};
        }

        // Get current step info
        const SequenceStep& step = steps[currentStep];
        const auto& charData = getCharacterByName(characterName);
        int maxFrames = charData.getFrameCount(step.action);

        // Ensure currentActionFrame is within bounds
        if (currentActionFrame >= maxFrames) {
            currentActionFrame = 0;
        }

        // Get the sprite rectangle for the current frame
        SDL_Rect rect = getSpriteRect(characterName, step.action, currentActionFrame);

        // Decrement remaining frames for this step
        stepFramesRemaining--;

        // Advance animation frame
        currentActionFrame++;
        if (currentActionFrame >= maxFrames) {
            currentActionFrame = step.loop ? 0 : maxFrames - 1;
        }

        // Move to next step if needed
        if (stepFramesRemaining <= 0) {
            if (currentStep < steps.size() - 1) {
                currentStep++;
                stepFramesRemaining = steps[currentStep].durationFrames;
                currentActionFrame = 0;
            } else {
                // Loop animation by resetting to the first step
                currentStep = 0;
                stepFramesRemaining = steps[currentStep].durationFrames;
                currentActionFrame = 0;
            }
        }

        return rect;
    }

    bool AnimationSequence::isFinished() const {
        return currentStep >= steps.size() - 1 && stepFramesRemaining <= 0;
    }

    void AnimationSequence::reset() {
        currentStep = 0;
        currentActionFrame = 0;
        stepFramesRemaining = steps.empty() ? 0 : steps[0].durationFrames;
    }

    // Example function to create a combat sequence
    AnimationSequence createCombatSequence(const std::string& characterName) {
        AnimationSequence sequence(characterName);

        // Start with stance
        sequence.addStep(Action::STANCE, 60, true);

        // Walk forward
        sequence.addStep(Action::WALK, 30, true);

        // Attack combo
        sequence.addStep(Action::HIGH_PUNCH, 15, false);
        sequence.addStep(Action::LOW_KICK, 15, false);
        sequence.addStep(Action::UPPERCUT, 20, false);

        // Victory pose
        sequence.addStep(Action::WIN, 60, true);

        return sequence;
    }

    // Example usage in a game loop:
    void renderAnimationSequence(SDL_Renderer* renderer, SDL_Texture* spritesheet,
                                 AnimationSequence& sequence, int x, int y, bool flipHorizontal) {
        SDL_Rect srcRect = sequence.update();

        // Ensure valid source rectangle
        if (srcRect.w == 0 || srcRect.h == 0) {
            return;
        }

        SDL_Rect dstRect = {x, y, srcRect.w, srcRect.h};

        SDL_RenderSetFlip(renderer, flipHorizontal ? SDL_FLIP_HORIZONTAL : 0);
        SDL_RenderTexture(renderer, spritesheet, &srcRect, &dstRect);
    }

} // namespace mortal_kombat
