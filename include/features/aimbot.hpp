#pragma once

namespace aimbot {
    // evaluates targets and updates aim state for the current frame.
    void Frame();
    // reports whether the indexed client passed the last penetration test.
    bool EspHittable(int client);
    // reports whether the indexed client passed the last visibility test.
    bool EspVisible(int client);
    // mutates the current user command with configured aim and fire behavior.
    void InjectCmd();
    // reports whether the current target state requests synthesized fire input.
    bool WantAutoShoot();
    // applies configured automatic actions to controller buttons and trigger state.
    void ApplyInputSynth(unsigned short* buttons, unsigned char* rightTrigger);
    // observes an entity event to drive reload timing state.
    void OnEntityEvent(void* centity, unsigned event);
    // applies configured anti-aim angles to the local rendered model.
    void ApplyFakeModel(int localClientNum, void* centity);
    // reports whether anti-aim currently controls the local model.
    bool AntiAimActive();
    // returns the last fake yaw selected by anti-aim.
    float AntiAimFakeYaw();
    // returns the last fake pitch selected by anti-aim.
    float AntiAimFakePitch();
}
