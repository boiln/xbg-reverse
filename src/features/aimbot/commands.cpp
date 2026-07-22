#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace aimbot {
    static float WriteAxis(float fake, float view, int* slot) {
        float f = fake - view;
        if (f > 180.0f) f -= 360.0f;
        if (f < -180.0f) f += 360.0f;
        *slot = (int)(f * DEG2SHORT) & 0xFFFF;

        return fake;
    }

    static void SpreadRandom(u32 serverTime, float* x, float* y) {
        u32 seed = serverTime;
        pSeedScramble(&seed);
        float angle = pRandomFloat(&seed) * 360.0f * 0.01745329238474369f;
        pAdvanceSeed4(&seed);
        float magnitude = pRandomFloat(&seed);
        *x = cosf(angle) * magnitude;
        *y = sinf(angle) * magnitude;
    }

    static bool CurrentSpread(void* cg, float* spread) {
        if (!cg || !spread) return false;

        void* ps = (char*)cg + 0x480A8;
        u32 weapon = (u32)pGetViewmodelWeapon(ps);
        float minimum = 0.0f;
        float maximum = 0.0f;

        pGetSpreadForWeapon(ps, weapon, &minimum, &maximum);

        if (RF(cg, 0x48288) != 0.0f) {
            void* def = pWeaponDef(weapon);
            if (!def) return false;
            minimum = RF(def, 0x7E0);
        }
        *spread = (maximum - minimum) * RF(cg, 0x24618) * 0.003921568859368563f + minimum;

        return true;
    }

    static void ApplyNoSpread(void* cg, char* previous, char* oldSlot, char* newSlot, int mode) {
        if (!CB(CFG_SPREAD) || !cg || !previous || !oldSlot || !newSlot) return;

        float spread = 0.0f;
        if (!CurrentSpread(cg, &spread)) return;

        float randomX = 0.0f;
        float randomY = 0.0f;
        SpreadRandom(*(u32*)(newSlot + U_SERVERTIME), &randomX, &randomY);

        bool leanActive = false;

        if (mode == 2 && !(leanActive && CB(CFG_LEAN) != 0)) {
            u32 pitchCorrection = (u32)(int)(-(randomY * spread) * 182.04444885253906f) & 0xFFFF;
            u32 yawCorrection = (u32)(int)(-(randomX * spread) * 182.04444885253906f) & 0xFFFF;
            *(u32*)(oldSlot + U_PITCH) -= pitchCorrection;
            *(u32*)(oldSlot + U_YAW) -= yawCorrection;
            *(u32*)(previous + U_PITCH) -= pitchCorrection;
            *(u32*)(previous + U_YAW) -= yawCorrection;
            *(u32*)(oldSlot + U_BUTTONS) &= 0x7FFFFFFFu;
            return;
        }

        float sourcePitch = 0.0f;
        float sourceYaw = 0.0f;
        float sourceRoll = 0.0f;

        if (RB(cg, 0x4809C) == 0) {
            sourcePitch = RF(cg, 0x69810);
            sourceYaw = RF(cg, 0x69814);
        } else {
            sourcePitch = RF(cg, 0x64920);
            sourceYaw = RF(cg, 0x64924);
            sourceRoll = RF(cg, 0x64928);
        }

        float forward[3];
        float right[3];
        float up[3];
        AngleVectors(sourcePitch, sourceYaw, sourceRoll, forward, right, up);

        float spreadRadians = spread * 0.01745329238474369f;
        float cone = (sinf(spreadRadians) / cosf(spreadRadians)) * 8192.0f;
        Vec3 shot = {forward[0] * 8192.0f + right[0] * (randomX * cone) + up[0] * (randomY * cone),
                     forward[1] * 8192.0f + right[1] * (randomX * cone) + up[1] * (randomY * cone),
                     forward[2] * 8192.0f + right[2] * (randomX * cone) + up[2] * (randomY * cone)};

        float shotYaw = atan2f(shot.y, shot.x) * RAD2DEG;
        float shotPitch = -atan2f(shot.z, sqrtf(shot.x * shot.x + shot.y * shot.y)) * RAD2DEG;

        u32 pitchCorrection = (u32)(int)((sourcePitch - shotPitch) * 182.04444885253906f) & 0xFFFF;
        u32 yawCorrection = (u32)(int)((sourceYaw - shotYaw) * 182.04444885253906f) & 0xFFFF;

        *(u32*)(oldSlot + U_PITCH) += pitchCorrection;
        *(u32*)(oldSlot + U_YAW) += yawCorrection;
        *(u32*)(previous + U_PITCH) += pitchCorrection;
        *(u32*)(previous + U_YAW) += yawCorrection;
    }

    static void AntiAim(void* cmd, bool writeCmd) {
        s_aaActive = false;
        if (!CB(AA_ENABLE)) return;

        int my = CB(AA_YAW);
        int mp = CB(AA_PITCH);
        if (my == 0 && mp == 0) return;

        float vPitch;
        float vYaw;
        ViewRef(&vPitch, &vYaw);

        int dumP;
        int dumY;
        int* cmdP = writeCmd ? (int*)((char*)cmd + U_PITCH) : &dumP;
        int* cmdY = writeCmd ? (int*)((char*)cmd + U_YAW) : &dumY;

        bool hasT = s_aaHasTarget;
        bool yawWrote = false;

        float fYaw = vYaw;
        float fPitch = vPitch;

        if (mp == 1) {
            if (hasT)
                fPitch = AA_TRACKP;
            else {
                s_spinPitch += AA_PSPIN;
                if (s_spinPitch > AA_CLAMP) s_spinPitch = -AA_CLAMP;
                fPitch = s_spinPitch;
            }
            fPitch = WriteAxis(fPitch, vPitch, cmdP);
        } else if (mp == 2)
            fPitch = WriteAxis(AA_UP, vPitch, cmdP);
        else if (mp == 3)
            fPitch = WriteAxis(AA_DOWN, vPitch, cmdP);
        else if (mp == 4) {
            s_aaTogPit ^= 1;
            fPitch = WriteAxis(s_aaTogPit ? AA_DOWN : AA_UP, vPitch, cmdP);
        } else if (mp == 5) {
            s_spinPitch += CF(AA_SPINPIT);
            if (s_spinPitch > AA_CLAMP) s_spinPitch = -AA_CLAMP;
            fPitch = WriteAxis(s_spinPitch, vPitch, cmdP);
        } else if (mp == 6) {
            if (hasT && s_slitherOn && !s_slitherHittable) {
                fPitch = WriteAxis(AA_DOWN, vPitch, cmdP);
            } else if (hasT) {
                float dp = -s_aaAimPitch - 36.0f;
                if (dp > AA_CLAMP) dp = AA_CLAMP;
                if (dp < -AA_CLAMP) dp = -AA_CLAMP;
                fPitch = WriteAxis(dp, vPitch, cmdP);
            } else {
                s_spinPitch += AA_PSPIN;
                if (s_spinPitch > AA_CLAMP) s_spinPitch = -AA_CLAMP;
                fPitch = WriteAxis(s_spinPitch, vPitch, cmdP);
            }
        }

        if (my == 1) {
            if (hasT) {
                fYaw = WriteAxis(s_aaAimYaw - 180.0f, vYaw, cmdY);
                yawWrote = true;
            }
        } else if (my == 2) {
            void* cg = CG();
            if (cg) {
                fYaw = WriteAxis(*(float*)((char*)cg + CG_A2A) - 180.0f, vYaw, cmdY);
                yawWrote = true;
            }
        } else if (my == 3) {
            s_spinYaw += CF(AA_SPINYAW);
            if (s_spinYaw > 360.0f) s_spinYaw = 0.0f;
            fYaw = WriteAxis(s_spinYaw, vYaw, cmdY);
            yawWrote = true;
        } else if (my == 4) {
            if (hasT && s_slitherOn && !s_slitherHittable) {
                fYaw = WriteAxis(s_aaAimYaw, vYaw, cmdY);
                yawWrote = true;
            } else if (hasT) {
                fYaw = WriteAxis(s_aaAimYaw - 180.0f, vYaw, cmdY);
                yawWrote = true;
            } else {
                s_spinYaw += AA_YDYN;
                if (s_spinYaw > 360.0f) s_spinYaw -= 360.0f;
                fYaw = WriteAxis(s_spinYaw, vYaw, cmdY);
                yawWrote = true;
            }
        } else if (my == 5) {
            fYaw = WriteAxis((float)(AaRand() % 360), vYaw, cmdY);
            yawWrote = true;
        }

        s_aaFakeYaw = fYaw;
        if (yawWrote) s_aaFakeYawDelta = fYaw - vYaw;
        s_aaFakePitch = fPitch;
        s_aaActive = true;
        s_cmdIn = (int)vYaw;
        s_myAim = (int)fYaw;
    }

    void ApplyFakeModel(int localClientNum, void* entity) {
        if (localClientNum != 0 || !entity) return;

        if (!CB(AA_ENABLE)) {
            s_aaActive = false;
            return;
        }
        if (!CB(CFG_SHOWAA) || !s_aaActive) return;
        void* cg = CG();
        if (!cg) return;
        if (RB(cg, 0x48263) == 'Y') return;
        char* e = (char*)entity;

        char* entities = Entities();
        if (!entities) return;

        int owner = RI(cg, 0x00);
        if (owner < 0 || owner >= 18) return;
        if ((int)(signed char)RB(e, 0x2C3) != owner) return;
        if (RI(entities + owner * ENT_STRIDE, E_CLIENTNUM) != RI(e, E_CLIENTNUM)) return;
        __try {
            int clientInfoIndex = RI(e, E_CLIENTNUM);
            if (clientInfoIndex < 0 || clientInfoIndex >= 18) return;

            float* clientPitch = (float*)((char*)cg + CG_CLIENTINFO + clientInfoIndex * CI_STRIDE + CI_FAKEPITCH);

            if (CB(AA_PITCH) != 0) *clientPitch = s_aaFakePitch - AA_PITCH_REF;
            if (CB(AA_YAW) != 0) {
                float basePitch;
                float baseYaw;

                ViewRef(&basePitch, &baseYaw);
                *(float*)(e + E_YAW) = baseYaw + s_aaFakeYawDelta;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    void InjectCmd() {
        s_call++;

        void* cg = CG();
        ApplyAimbotPatches(cg);

        char* cs = ClientState();
        s_csVal = (unsigned)(u64)cs;

        u32 csLo = (u32)(u64)cs;
        s_csNibble = cs ? (int)(csLo >> 28) : -1;

        if (!cs || (csLo >> 28) != 0xB) {
            s_path = 1;
            return;
        }

        u32 cmdNum = *(u32*)(cs + CS_CMDNUM);
        char* previous = cs + CS_CMDRING + ((cmdNum - 1) & 0x7F) * CMD_STRIDE;
        char* oldSlot = cs + CS_CMDRING + (cmdNum & 0x7F) * CMD_STRIDE;
        char* newSlot = cs + CS_CMDRING + ((cmdNum + 1) & 0x7F) * CMD_STRIDE;

        int mode = CB(CFG_TYPE);
        float vPitch;
        float vYaw;
        ViewRef(&vPitch, &vYaw);

        s_pathMode = mode;
        s_pathHasTarget = s_hasTarget ? 1 : 0;

        bool aimActive = mode != 0 && s_hasTarget;
        bool aaActive = CB(MODE_SPECTATOR) == 0 && CB(AA_ENABLE) != 0 && (CB(AA_YAW) != 0 || CB(AA_PITCH) != 0) && cg &&
                        ((RI(cg, 0x481A4) & 0x4000) == 0) && ((RI(cg, 0x480B4) & 0x5) == 0);
        bool manualFire = (*(u32*)(oldSlot + U_BUTTONS) & BTN_ATTACK) != 0;

        if (!aaActive) s_aaActive = false;
        if (!aimActive && manualFire) aaActive = false;
        if (!aimActive && !aaActive) return;

        // cloning first preserves every non-time field in the synthesized command.
        for (u32 i = 0; i < CMD_STRIDE; ++i) newSlot[i] = oldSlot[i];
        *(int*)(oldSlot + U_SERVERTIME) -= 1;
        *(int*)(newSlot + U_SERVERTIME) += 1;
        *(u32*)(cs + CS_CMDNUM) = cmdNum + 1;

        if (aimActive) {
            float dPitch = NormDeg(s_aimPitch - vPitch);
            float dYaw = NormDeg(s_aimYaw - vYaw);

            *(int*)(oldSlot + U_PITCH) = ANGLE2SHORT(dPitch);
            *(int*)(oldSlot + U_YAW) = ANGLE2SHORT(dYaw);
            if (mode == 1) {
                *(float*)(cs + CS_MVPITCH) = dPitch;
                *(float*)(cs + CS_MVYAW) = dYaw;
                *(float*)(cs + CS_MVROLL) = 0.0f;
                SteerView(cg, s_aimPitch, s_aimYaw);
                s_path = 4;
            } else {
                s_path = 5;
            }

            // rotating movement by the yaw delta preserves its world-space direction.
            float shift = NormDeg(dYaw - *(float*)(cs + CS_MVYAW)) * 0.017453292519943295f;
            float sr = sinf(shift);
            float cr = cosf(shift);

            signed char* mf = (signed char*)(oldSlot + 0x24);
            signed char* mr = (signed char*)(oldSlot + 0x25);

            int fv = *mf;
            int rv = *mr;
            int nf = (int)(cr * fv - sr * rv);
            int nr = (int)(sr * fv + cr * rv);

            *mf = (signed char)(nf < -128 ? -128 : (nf > 127 ? 127 : nf));
            *mr = (signed char)(nr < -128 ? -128 : (nr > 127 ? 127 : nr));

            ApplyNoSpread(cg, previous, oldSlot, newSlot, mode);
            s_myAim = *(int*)(oldSlot + U_YAW) & 0xFFFF;
            s_wrote++;
        }

        u32 fireMask = BTN_ATTACK;
        bool canShoot = true;
        char* entities = Entities();

        if (entities && cg) {
            int local = RI(cg, CG_CLIENTNUM);
            int weapon = RI(entities + local * ENT_STRIDE, 0x2B4);
            if (pIsDualWield((u32)weapon)) fireMask = 0x80100080u;
            u8 viewWeapon = RB(entities + local * ENT_STRIDE, 0x1B3);
            void* weaponDef = pWeaponDef(viewWeapon);
            if (weaponDef && RI(weaponDef, 0x1C) == 8) canShoot = false;
            if (RB(cg, 0x48278) == 0x0B) canShoot = false;
        }

        s_autoCmd = 0;

        if (canShoot && aimActive && CB(CFG_AUTOSHOOT) != 0 && s_targetVisible) {
            // moving fire to the cloned slot prevents both commands from attacking.
            *(u32*)(newSlot + U_BUTTONS) |= fireMask;
            *(u32*)(oldSlot + U_BUTTONS) &= ~fireMask;
            s_autoCmd = 1;
        } else if (CB(CFG_RAPID) != 0 || (aimActive && CB(CFG_AUTOSHOOT) == 0)) {
            *(u32*)(oldSlot + U_BUTTONS) &= ~fireMask;
            s_autoCmd = 2;
        } else if (!canShoot) {
            s_autoCmd = -1;
        } else if (!aimActive) {
            s_autoCmd = -2;
        } else if (!s_targetVisible) {
            s_autoCmd = -3;
        }

        if (aaActive) AntiAim(newSlot, true);
        s_path = 6;
    }
}
