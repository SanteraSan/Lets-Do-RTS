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

inline float cappedFrameSeconds(float frameSeconds, float maxSeconds = 0.25f) {
    if (frameSeconds <= 0.0f || maxSeconds <= 0.0f) {
        return 0.0f;
    }
    if (frameSeconds > maxSeconds) {
        return maxSeconds;
    }
    return frameSeconds;
}
