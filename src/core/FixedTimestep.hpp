#pragma once

struct FixedTimestep {
    float delta = 1.0f / 60.0f;
    float accumulator = 0.0f;
};

inline void addFrameTime(FixedTimestep& step, float frameSeconds) {
    if (frameSeconds > 0.0f) {
        step.accumulator += frameSeconds;
    }
}

inline bool consumeStep(FixedTimestep& step) {
    if (step.delta <= 0.0f) {
        return false;
    }
    if (step.accumulator < step.delta) {
        return false;
    }
    step.accumulator -= step.delta;
    return true;
}
