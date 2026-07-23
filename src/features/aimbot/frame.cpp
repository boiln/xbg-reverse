#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace aimbot {
    void Frame() {

        for (int stateIdx = 0; stateIdx < 18; ++stateIdx) {
            s_espHittable[stateIdx] = 0;
            s_espVisible[stateIdx]  = 0;
        }

        s_reach       = 0;
        s_hasTarget   = false;
        s_aaHasTarget = false;
        int mode   = CB(CFG_TYPE);
        bool aimOn = mode != 0;
        bool aaOn  = CB(AA_ENABLE) != 0 && (CB(AA_YAW) != 0 || CB(AA_PITCH) != 0);
        void* cg   = CG();
        if (!aimOn && !aaOn) return;
        s_reach = 1;
        if (!cg) return;
        s_reach = 2;
        char* rd = (char*)cg + CG_REFDEF;
        if (RI(rd, RD_W) < 1 || RI(rd, RD_H) < 1) return;
        s_reach = 3;
        char* base = Entities();
        if (!base) return;
        s_reach = 4;

        int localIdx     = RI(cg, CG_CLIENTNUM);
        Vec3 localOrigin = RV3(base + localIdx * ENT_STRIDE, E_ORIGIN);
        Vec3 eye         = RV3(rd, RD_ORG);

        __try {
            void* ps = (char*)cg + 0x480A8;

            pGetPlayerViewOrigin(0, ps, &eye);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            eye = RV3(rd, RD_ORG);
        }
        Vec3 fwd       = RV3(rd, RD_AXIS);
        float curYaw   = atan2f(fwd.y, fwd.x) * RAD2DEG;
        float curPitch = -atan2f(fwd.z, sqrtf(fwd.x * fwd.x + fwd.y * fwd.y)) * RAD2DEG;

        s_viewYaw   = curYaw;
        s_viewPitch = curPitch;

        int cand        = 0;
        float best      = 1e18f;
        float bYaw      = 0;
        float bPitch    = 0;
        int bIdx        = -1;
        bool found      = false;
        bool bOnScreen  = false;
        Vec3 bAimPt     = { 0, 0, 0 };
        float bestP     = 1e18f;
        float bYawP     = 0;
        float bPitchP   = 0;
        int bIdxP       = -1;
        bool foundP     = false;
        bool bOnScreenP = false;
        Vec3 bAimPtP    = { 0, 0, 0 };
        float aaBest    = 3.4e38f;
        float aaYaw     = 0.0f;
        float aaPitch   = 0.0f;
        bool aaPriority = false;
        float fovRadius = AimFovPixels();
        float centerX   = RI(rd, RD_W) * 0.5f;
        float centerY   = RI(rd, RD_H) * 0.5f;
        int aimBone     = CB(CFG_HEAD) ? 2 :(int)CB(CFG_AIMTAG);

        if (aimBone < 0 || aimBone >= kBoneCount) aimBone = 2;

        for (int i = 0; i < 18; ++i) {
            if (i == localIdx) continue;

            char* ent = base + i * ENT_STRIDE;
            if ((RB(ent, E_ALIVE) & 0x40) == 0) continue;
            char* ci = (char*)cg + CG_CLIENTINFO + i * CI_STRIDE;
            if (RI(ci, 0) == 0 && RB(ci, CI_NAME) == 0) continue;
            if (CB(CFG_WHITELIST + i)) continue;
            if (pTeam(0, ent) != 0) continue;

            ++cand;

            Vec3 targetOrigin = RV3(ent, E_ORIGIN);
            float odx         = targetOrigin.x - localOrigin.x;
            float ody         = targetOrigin.y - localOrigin.y;
            float odz         = targetOrigin.z - localOrigin.z;
            float distance    = sqrtf(odx * odx + ody * ody + odz * odz);
            Vec3 o            = { 0.0f, 0.0f, 0.0f };
            bool hittable     = false;
            BoneEvalContext boneContext;

            boneContext.cg = cg;
            boneContext.entities = base;
            boneContext.localIdx = localIdx;
            boneContext.targetIdx = i;
            boneContext.eye = eye;
            boneContext.allowPenetration = CB(CFG_AUTOWALL) != 0;
            boneContext.lastDirect = false;

            __try {
                // autobone scans only after the configured bone fails its direct path.
                autobone::Vec3 configuredPoint;
                bool configuredResolved = ResolveDirectBone(&boneContext, (u8)aimBone, &configuredPoint);

                if (configuredResolved) {
                    Vec3 directPoint = { configuredPoint.x, configuredPoint.y, configuredPoint.z };

                    hittable = DirectBulletTrace(cg, base, localIdx, i, eye, directPoint);

                    if (hittable) {
                        o = directPoint;
                        boneContext.lastDirect = true;
                        s_espVisible[i] = 1;
                    }
                }

                if (!hittable && CB(CFG_AUTOBONE) == 0) {
                    float bestDamage = 0.0f;
                    autobone::Vec3 selected;

                    hittable = TryBoneSelector(&boneContext, (u8)aimBone, &selected, &bestDamage);

                    if (hittable) {
                        o.x = selected.x;
                        o.y = selected.y;
                        o.z = selected.z;
                    }
                }

                if (!hittable && CB(CFG_AUTOBONE) != 0) {
                    autobone::SelectInput input;

                    input.configuredSelector = (u8)aimBone;
                    input.alwaysCheckHead    = CB(CFG_HEAD) != 0;
                    input.priorityClient     = CB(MODE_SPECTATOR) == 0 && CB(CFG_PRIORITY + i) != 0;
                    input.shieldWeapon       = RB(ent, 0x1B0) == 0x59 || RB(ent, 0x1B4) == 0x59;

                    input.paused = false;
                    autobone::SelectResult selected;

                    hittable =
                        autobone::Select(input, &s_boneSelectState[i], &boneContext, TryBoneSelector, 0.0f, &selected);

                    if (hittable) {
                        o.x = selected.position.x;
                        o.y = selected.position.y;
                        o.z = selected.position.z;
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                hittable = false;
            }

            if (!hittable) continue;

            s_espHittable[i] = 1;
            // fov uses the live point while aim angles use the predicted point.
            Vec3 fovPoint = o;

            ApplyAimPrediction(cg, base, i, &o);

            float dx      = o.x - eye.x;
            float dy      = o.y - eye.y;
            float dz      = o.z - eye.z;
            float yaw     = atan2f(dy, dx) * RAD2DEG;
            float pitch   = -atan2f(dz, sqrtf(dx * dx + dy * dy)) * RAD2DEG;
            bool priority = CB(CFG_PRIORITY + i) != 0;
            // anti-aim keeps a separate nearest target with priority taking precedence.
            if ((!aaPriority && priority) || (priority == aaPriority && distance < aaBest)) {
                aaBest        = distance;
                aaYaw         = yaw;
                aaPitch       = pitch;
                aaPriority    = priority;
                s_aaHasTarget = true;
            }

            Vec2 screen;
            bool projected = WorldToScreen(cg, fovPoint, &screen);
            bool onScreen = projected && centerX - fovRadius < screen.x && screen.x < centerX + fovRadius &&
            centerY - fovRadius < screen.y && screen.y < centerY + fovRadius;
            float pixelCost = 0.0f;

            if (projected) {
                float pixelDx = screen.x - centerX;
                float pixelDy = screen.y - centerY;

                pixelCost = pixelDx * pixelDx + pixelDy * pixelDy;
            }

            if (distance < best) {
                best      = distance;
                bYaw      = yaw;
                bPitch    = pitch;
                bIdx      = i;
                bAimPt    = o;
                bOnScreen = onScreen;
                found     = true;
                s_cost    = projected ? (int)(sqrtf(pixelCost) + 0.5f) : -1;
            }

            if (priority && distance < bestP) {
                bestP      = distance;
                bYawP      = yaw;
                bPitchP    = pitch;
                bIdxP      = i;
                bAimPtP    = o;
                bOnScreenP = onScreen;
                foundP     = true;
            }
        }

        s_reach = 5;
        s_cand  = cand;

        // the priority pool replaces the normal closest target after the full scan.
        if (foundP) {
            best      = bestP;
            bYaw      = bYawP;
            bPitch    = bPitchP;
            bIdx      = bIdxP;
            bAimPt    = bAimPtP;
            bOnScreen = bOnScreenP;
            found     = true;
        }

        if (!found) s_cost = -1;
        if (!found) return;
        if (bPitch > PITCH_CLAMP) bPitch = PITCH_CLAMP;
        if (bPitch < -PITCH_CLAMP) bPitch = -PITCH_CLAMP;
        s_aimYaw = bYaw;
        s_aimPitch = bPitch;
        s_aaAimYaw = aaYaw;
        s_aaAimPitch = aaPitch;
        s_hasTarget = aimOn;

        bool fovPass = (RI(cg, 0x480B4) & 1) == 0 || bOnScreen;

        s_targetVisible = fovPass;
        bool doSteer = s_hasTarget && mode == 1;

        if (doSteer) SteerView(cg, s_aimPitch, s_aimYaw);

    }
}
