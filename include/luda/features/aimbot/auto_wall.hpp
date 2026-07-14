#pragma once

namespace autowall {
    typedef unsigned int u32;

    struct Result {
        bool hit;
        bool direct;
        float budget;
        float score;
        int walls;
        int stage;
        float lastThickness;
        float lastDepth;
        float lastBudget;
        int reason;
        int edgeBits;
        u32 fireHitId;
        u32 forwardHitId;
        u32 traceType;
        u32 reverseType;
    };

    bool Evaluate(void* cg, void* entities, int localClient, int targetClient, const float eye[3], const float end[3],
                  bool spectatorMode, Result* result);
}  // namespace autowall
