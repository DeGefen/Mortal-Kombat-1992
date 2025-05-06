#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace mortal_kombat {

    // Action types that a character can perform
    enum class Action {
        STANCE,
        WALK,
        HIGH_PUNCH,
        HIGH_PUNCH_SPREE,
        BODY_TO_BODY_PUNCH,
        LOW_PUNCH,
        LOW_PUNCH_SPREE,
        BODY_TO_BODY_KICK,
        LOW_KICK,
        LOWKICK_SWEEP,
        HIGH_KICK,
        HIGHKICK_SWEEP,
        CROUCH,
        UPPERCUT,
        CROUCH_KICK,
        JUMP,
        JUMP_PUNCH,
        JUMP_HIGHKICK,
        LANDING,
        JUMP_BACK,
        ROLL,
        FORWARD_JUMP_PUNCH,
        JUMP_LOWKICK,
        TORSO_HIT,
        HEAD_HIT,
        KICKBACK_TORSO_HIT,
        CROUCH_HIT,
        FALL,
        UPPERCUT_HIT,
        NUTS_HIT,
        FALL_INPLACE,
        GETUP,
        CAUGHT,
        THROWN,
        BLOCK,
        CROUCH_BLOCK,
        TURN_RIGHT_TO_LEFT,
        TURN_LEFT_TO_RIGHT,
        SPECIAL_1,
        SPECIAL_2,
        SPECIAL_3,
        GIDDY,
        FINISH_HIM,
        GIDDY_FALL,
        FATALITY,
        WIN
    };

    // Sprite sheet parameters (shared across all characters)
    struct SpriteSheetInfo {
        int charSquareWidth;
        int charSquareHeight;
        int nextActionRight;
        int nextActionDown;
        int firstFrameX;
        int firstFrameY;
        int nextFrameOffset;
        int shadowOffset;
    };

    // Animation data for a specific action
    struct ActionAnimation {
        Action action;
        int frameCount;
    };

    // Character animation data
    struct CharacterAnimations {
        std::string name;
        std::unordered_map<Action, int> frameCount;

        // Returns the number of frames for a given action
        int getFrameCount(Action action) const {
            auto it = frameCount.find(action);
            return (it != frameCount.end()) ? it->second : 0;
        }
    };

    struct ActionPosition {
        int row;
        int column;
    };

    // Animation sequence for character actions
    class AnimationSequence {
    public:
        struct SequenceStep {
            Action action;
            int durationFrames;  // How many frames to show this action
            bool loop;           // Whether to loop the action during its duration
        };

        std::vector<SequenceStep> steps;
        std::string characterName;
        int currentStep = 0;
        int stepFramesRemaining = 0;
        int currentActionFrame = 0;

        AnimationSequence(const std::string& charName) : characterName(charName) {}

        void addStep(Action action, int durationFrames, bool loop = true);
        SDL_Rect update();
        bool isFinished() const;
        void reset();
    };

    // Map each action to its position in the sprite sheet
    const std::unordered_map<Action, ActionPosition> ACTION_POSITIONS = {
        {Action::STANCE, {0, 0}},
        {Action::WALK, {0, 1}},
        {Action::HIGH_PUNCH, {1, 0}},
        {Action::HIGH_PUNCH_SPREE, {1, 1}},
        {Action::BODY_TO_BODY_PUNCH, {1, 2}},
        {Action::LOW_PUNCH, {2, 0}},
        {Action::LOW_PUNCH_SPREE, {2, 1}},
        {Action::BODY_TO_BODY_KICK, {2, 2}},
        {Action::LOW_KICK, {3, 0}},
        {Action::LOWKICK_SWEEP, {3, 1}},
        {Action::HIGH_KICK, {4, 0}},
        {Action::HIGHKICK_SWEEP, {4, 1}},
        {Action::CROUCH, {5, 0}},
        {Action::UPPERCUT, {5, 1}},
        {Action::CROUCH_KICK, {5, 2}},
        {Action::JUMP, {6, 0}},
        {Action::JUMP_PUNCH, {6, 1}},
        {Action::JUMP_HIGHKICK, {6, 2}},
        {Action::LANDING, {6, 3}},
        {Action::JUMP_BACK, {7, 0}},
        {Action::ROLL, {7, 1}},
        {Action::FORWARD_JUMP_PUNCH, {7, 2}},
        {Action::JUMP_LOWKICK, {7, 3}},
        {Action::TORSO_HIT, {8, 0}},
        {Action::HEAD_HIT, {8, 1}},
        {Action::KICKBACK_TORSO_HIT, {8, 2}},
        {Action::CROUCH_HIT, {8, 3}},
        {Action::FALL, {9, 0}},
        {Action::UPPERCUT_HIT, {9, 1}},
        {Action::NUTS_HIT, {9, 2}},
        {Action::FALL_INPLACE, {10, 0}},
        {Action::GETUP, {10, 1}},
        {Action::CAUGHT, {11, 0}},
        {Action::THROWN, {11, 1}},
        {Action::BLOCK, {12, 0}},
        {Action::CROUCH_BLOCK, {12, 1}},
        {Action::TURN_RIGHT_TO_LEFT, {12, 2}},
        {Action::TURN_LEFT_TO_RIGHT, {12, 3}},
        {Action::SPECIAL_1, {13, 0}},
        {Action::SPECIAL_2, {13, 1}},
        {Action::GIDDY, {14, 0}},
        {Action::FINISH_HIM, {14, 1}},
        {Action::GIDDY_FALL, {14, 2}},
        {Action::FATALITY, {15, 0}},
        {Action::WIN, {16, 0}}
    };

    // Global sprite sheet information
    const SpriteSheetInfo SPRITE_SHEET_INFO = {
        230, 220,    // charSquareWidth, charSquareHeight
        238,         // nextActionRight
        302,         // nextActionDown
        32, 58,      // firstFrameX, firstFrameY
        4,           // nextFrameOffset
        8            // shadowOffset
    };

} // namespace mortal_kombat
