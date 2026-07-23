#pragma once

namespace aimbot {

    void Frame();

    bool EspHittable(int client);

    bool EspVisible(int client);

    void InjectCmd();

    bool WantAutoShoot();

    void ApplyInputSynth(unsigned short* buttons, unsigned char* rightTrigger);

    void OnEntityEvent(void* centity, unsigned event);

    void ApplyFakeModel(int localClientNum, void* centity);

    bool AntiAimActive();

    float AntiAimFakeYaw();

    float AntiAimFakePitch();
}
