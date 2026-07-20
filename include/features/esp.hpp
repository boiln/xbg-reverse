#pragma once

namespace esp {
    // binds engine functions and resets esp-owned state.
    void Init();
    // draws the current frame's eligible player and world overlays.
    void Render();
    // returns the local client's current ping or zero when unavailable.
    int LocalPing();
    // reports whether a usable game session is active.
    bool InGame();
    // returns the local client index or zero when state is unavailable.
    int LocalClientIdx();
    // resets mode-specific state when the multiplayer/zombies session changes.
    void SyncSessionMode();
    // returns the indexed player name or an empty fallback.
    const char* PlayerName(int index);
    // records a local bullet endpoint for tracer rendering.
    void OnBulletEmit(unsigned owner, const void* endpoint);
    // restores esp-owned patches and clears transient state.
    void Shutdown();
}
