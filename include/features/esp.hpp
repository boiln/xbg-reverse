#pragma once

namespace esp {

    void Init();

    void Render();

    int LocalPing();

    bool InGame();

    int LocalClientIdx();

    void SyncSessionMode();

    const char* PlayerName(int index);

    void OnBulletEmit(unsigned owner, const void* endpoint);

    void Shutdown();
}
