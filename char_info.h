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

    // Add to char_info.h
    struct ActionPosition {
        int row;
        int column;
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

    // Character-specific animations
    const CharacterAnimations SUB_ZERO_ANIMATIONS = {
        "Sub_Zero",
        {
            {Action::STANCE, 12},
            {Action::WALK, 9},
            {Action::HIGH_PUNCH, 5},
            {Action::HIGH_PUNCH_SPREE, 10},
            {Action::BODY_TO_BODY_PUNCH, 5},
            {Action::LOW_PUNCH, 5},
            {Action::LOW_PUNCH_SPREE, 10},
            {Action::BODY_TO_BODY_KICK, 5},
            {Action::LOW_KICK, 7},
            {Action::LOWKICK_SWEEP, 8},
            {Action::HIGH_KICK, 10},
            {Action::HIGHKICK_SWEEP, 8},
            {Action::CROUCH, 5},
            {Action::UPPERCUT, 5},
            {Action::CROUCH_KICK, 5},
            {Action::JUMP, 1},
            {Action::JUMP_PUNCH, 5},
            {Action::JUMP_HIGHKICK, 3},
            {Action::LANDING, 3},
            {Action::JUMP_BACK, 1},
            {Action::ROLL, 7},
            {Action::FORWARD_JUMP_PUNCH, 5},
            {Action::JUMP_LOWKICK, 5},
            {Action::TORSO_HIT, 4},
            {Action::HEAD_HIT, 4},
            {Action::KICKBACK_TORSO_HIT, 6},
            {Action::CROUCH_HIT, 3},
            {Action::FALL, 5},
            {Action::UPPERCUT_HIT, 6},
            {Action::NUTS_HIT, 6},
            {Action::FALL_INPLACE, 6},
            {Action::GETUP, 5},
            {Action::CAUGHT, 6},
            {Action::THROWN, 7},
            {Action::BLOCK, 5},
            {Action::CROUCH_BLOCK, 5},
            {Action::TURN_RIGHT_TO_LEFT, 4},
            {Action::TURN_LEFT_TO_RIGHT, 4},
            {Action::SPECIAL_1, 10},
            {Action::GIDDY, 7},
            {Action::GIDDY_FALL, 7},
            {Action::WIN, 4}
        }
    };

    // Add similar structures for other characters like Scorpion, etc.

    // Function to calculate the source rect for a specific character, action and frame
    SDL_Rect getCharacterSpriteRect(const std::string& characterName, Action action, int frameIndex) {
        // Get the position from the action map
        auto it = ACTION_POSITIONS.find(action);
        if (it == ACTION_POSITIONS.end()) {
            // Default to STANCE if action not found
            return getCharacterSpriteRect(characterName, Action::STANCE, 0);
        }

        const ActionPosition& pos = it->second;

        SDL_Rect rect;
        rect.x = SPRITE_SHEET_INFO.firstFrameX +
                 pos.column * SPRITE_SHEET_INFO.nextActionRight +
                 frameIndex * SPRITE_SHEET_INFO.nextFrameOffset;
        rect.y = SPRITE_SHEET_INFO.firstFrameY +
                 pos.row * SPRITE_SHEET_INFO.nextActionDown;
        rect.w = SPRITE_SHEET_INFO.charSquareWidth;
        rect.h = SPRITE_SHEET_INFO.charSquareHeight;

        return rect;
    }

    // Get a character's animation data by name
    const CharacterAnimations& getCharacterByName(const std::string& name) {
        if (name == "Sub_Zero") {
            return SUB_ZERO_ANIMATIONS;
        }
        // Add other characters here

        // Default to Sub-Zero if character not found
        return SUB_ZERO_ANIMATIONS;
    }


    // Precomputed sprite rects for each character, action and frame
    // Map structure: character_name -> action -> vector of frames
    struct SpriteRectCache {
        std::unordered_map<std::string,
        std::unordered_map<Action,
                std::vector<SDL_Rect>>> rects;

        // Get a precomputed sprite rect
        SDL_Rect getRect(const std::string& characterName, Action action, int frameIndex) const {
            auto charIt = rects.find(characterName);
            if (charIt == rects.end()) return {0, 0, 0, 0};

            auto actionIt = charIt->second.find(action);
            if (actionIt == charIt->second.end()) return {0, 0, 0, 0};

            const auto& frames = actionIt->second;
            if (frameIndex >= frames.size()) return {0, 0, 0, 0};

            return frames[frameIndex];
        }
    };

    // Global cache instance
    inline SpriteRectCache spriteRectCache;

    // Initialize all sprite rects for a character
    void precomputeCharacterRects(const std::string& characterName) {
        const auto& charData = getCharacterByName(characterName);

        for (const auto& [action, frameCount] : charData.frameCount) {
            std::vector<SDL_Rect> rects;
            for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                // Use existing getCharacterSpriteRect function
                SDL_Rect rect = getCharacterSpriteRect(characterName, action, frameIndex);
                rects.push_back(rect);
            }
            spriteRectCache.rects[characterName][action] = std::move(rects);
        }
    }

    // Get a sprite rect efficiently
    SDL_Rect getSpriteRect(const std::string& characterName, Action action, int frameIndex) {
        // Try the cache first
        SDL_Rect rect = spriteRectCache.getRect(characterName, action, frameIndex);

        // If not in cache (rect is empty), compute on-demand
        if (rect.w == 0 && rect.h == 0) {
            rect = getCharacterSpriteRect(characterName, action, frameIndex);

            // Store computed rect for future use
            const auto& charData = getCharacterByName(characterName);
            int maxFrames = charData.getFrameCount(action);

            if (frameIndex < maxFrames) {
                // Ensure the vector is large enough
                auto& actionRects = spriteRectCache.rects[characterName][action];
                if (actionRects.size() <= frameIndex) {
                    actionRects.resize(maxFrames);
                }
                actionRects[frameIndex] = rect;
            }
        }

        return rect;
    }

} // namespace mortal_kombat