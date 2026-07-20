#pragma once

namespace reconrender {
    // maps a configuration address to its byte storage.
    unsigned char* CfgByte(unsigned va);
    // maps a configuration address to its float storage.
    float* CfgFloat(unsigned va);
    // starts the menu lifecycle and installs its runtime hooks.
    void Start();
    // removes runtime hooks, stops workers, and restores owned patches.
    void Stop();
}
