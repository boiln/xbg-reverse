#pragma once

namespace autowall {
    struct Result {
        bool direct;
        float score;
    };

    bool Evaluate(void* cg, void* entities, int localClient, int targetClient, const float eye[3], const float end[3],
                  bool spectatorMode, Result* result);
}
