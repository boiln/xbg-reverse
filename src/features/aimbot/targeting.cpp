#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace aimbot {
    bool DirectBulletTrace(void* cg, char* entities, int localIdx, int targetIdx, const Vec3& start, const Vec3& end) {

        if (!cg || !entities || localIdx < 0 || targetIdx < 0) return false;
        unsigned char fire[0x40];
        unsigned char trace[0x80];

        for (int i = 0; i < 0x40; ++i) fire[i] = 0;
        for (int i = 0; i < 0x80; ++i) trace[i] = 0;

        *(u32*)(fire + 0x00) = 0x3FE;
        *(u32*)(fire + 0x04) = RI(cg, 0x48248);
        *(float*)(fire + 0x08) = 1.0f;
        *(u32*)(fire + 0x0C) = 9;
        *(Vec3*)(fire + 0x10) = start;
        *(Vec3*)(fire + 0x1C) = start;
        *(Vec3*)(fire + 0x28) = end;

        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float dz = end.z - start.z;
        float length = sqrtf(dx * dx + dy * dy + dz * dz);
        if (length <= 0.0001f) return false;

        Vec3 direction = {dx / length, dy / length, dz / length};
        *(Vec3*)(fire + 0x34) = direction;

        char* localEntity = entities + localIdx * ENT_STRIDE;
        if (!pBulletTrace(0, fire, localEntity, trace, 0, 0)) return false;
        if (CB(MODE_SPECTATOR) == 0 && *(short*)(trace + 0x28) == 0x14) return false;
        if (*(float*)(trace + 0x10) < 0.97f) return false;

        return *(unsigned short*)(trace + 0x20) == (unsigned short)targetIdx;

    }
    static const char* const kBoneNames[] = {
        "j_head_end",
        "tag_eye",
        "j_head",
        "j_neck",
        "j_spine4",
        "tag_stowed_back",
        "j_spineupper",
        "j_mainroot",
        "j_spinelower",
        "j_shoulder_ri",
        "j_shoulder_le",
        "j_clavicle_le",
        "j_clavicle_ri",
        "j_shouldertwist_ri",
        "j_shouldertwist_le",
        "j_shoulderraise_ri",
        "j_shoulderraise_le",
        "j_elbow_ri",
        "j_elbow_le",
        "j_elbow_bulge_ri",
        "j_elbow_bulge_le",
        "j_wrist_ri",
        "j_wrist_le",
        "j_wristtwist_ri",
        "j_wristtwist_le",
        "j_mid_ri_1",
        "j_mid_le_1",
        "back_low",
        "pelvis",
        "j_hip_ri",
        "j_hip_le",
        "j_hiptwist_ri",
        "j_hiptwist_le",
        "j_knee_ri",
        "j_knee_le",
        "j_knee_bulge_ri",
        "j_knee_bulge_le",
        "j_ankle_ri",
        "j_ankle_le",
        "j_ball_ri",
        "j_ball_le",
        "j_pinky_le_3",
        "j_pinky_ri_3",
        "j_index_ri_3",
        "j_index_le_3",
        "j_mid_le_3",
        "j_mid_ri_3",
        "j_ring_le_3",
        "j_ring_ri_3",
        "j_thumb_le_3",
        "j_thumb_ri_3"};
    const int kBoneCount = (int)(sizeof(kBoneNames) / sizeof(kBoneNames[0]));

    volatile int s_autoInput = 0;
    volatile int s_autoCmd = 0;

    autobone::SelectState s_boneSelectState[18];

    bool ResolveDirectBone(void* raw, u8 selector, autobone::Vec3* position) {

        BoneEvalContext* context = (BoneEvalContext*)raw;
        if (!context || !position || selector >= (u8)kBoneCount) return false;
        Vec3 point;
        if (!TagPos(context->entities, context->targetIdx, kBoneNames[selector], &point)) return false;
        if (point.x == 0.0f && point.y == 0.0f && point.z == 0.0f) return false;

        float correction = CF(CFG_POSITION_CORRECTION);

        if (correction > 0.0f) {
            point.y = -(correction - point.y);
            point.z = -(correction - point.z);
        }
        position->x = point.x;
        position->y = point.y;
        position->z = point.z;

        return true;

    }

    bool TryBoneSelector(void* raw, u8 selector, autobone::Vec3* position, float* bestDamage) {

        BoneEvalContext* context = (BoneEvalContext*)raw;
        if (!context || !position || !bestDamage) return false;
        autobone::Vec3 point;
        if (!autobone::ResolvePosition(raw, selector, ResolveDirectBone, &point)) return false;
        Vec3 livePoint = {point.x, point.y, point.z};

        if (!context->allowPenetration) {
            if (!DirectBulletTrace(context->cg, context->entities, context->localIdx, context->targetIdx, context->eye,
                                   livePoint))
                return false;
            context->lastDirect = true;
            *position = point;
            return true;
        }
        autowall::Result wall;
        const float eye[3] = {context->eye.x, context->eye.y, context->eye.z};
        const float end[3] = {point.x, point.y, point.z};

        if (!autowall::Evaluate(context->cg, context->entities, context->localIdx, context->targetIdx, eye, end,
                                CB(MODE_SPECTATOR) != 0, &wall))
            return false;
        // a selector only wins when it improves the current damage score.
        if (wall.score <= *bestDamage) return false;
        *bestDamage = wall.score;
        context->lastDirect = wall.direct;
        *position = point;

        return true;

    }

    float NormDeg(float a) {
        while (a > 180.f) a -= 360.f;
        while (a < -180.f) a += 360.f;

        return a;
    }
    int ANGLE2SHORT(float d) { return (int)(d * DEG2SHORT) & 0xFFFF; }

    void AngleVectors(float pitch, float yaw, float roll, float* fwd, float* right, float* up) {

        const float D = 0.017453292519943295f;

        float sp = sinf(pitch * D);
        float cp = cosf(pitch * D);
        float sy = sinf(yaw * D);
        float cy = cosf(yaw * D);
        float sr = sinf(roll * D);
        float cr = cosf(roll * D);

        if (fwd) {
            fwd[0] = cp * cy;
            fwd[1] = cp * sy;
            fwd[2] = -sp;
        }

        if (right) {
            right[0] = -sr * sp * cy + cr * sy;
            right[1] = -sr * sp * sy - cr * cy;
            right[2] = -sr * cp;
        }

        if (up) {
            up[0] = cr * sp * cy + sr * sy;
            up[1] = cr * sp * sy - sr * cy;
            up[2] = cr * cp;
        }

    }

    void SteerView(void* cg, float pitch, float yaw) {

        if (!cg) return;
        float* ang = (float*)((char*)cg + CG_VIEWANG);
        ang[0] = pitch;
        ang[1] = yaw;
        ang[2] = 0.0f;
        AngleVectors(pitch, yaw, 0.0f, (float*)((char*)cg + CG_AXIS_FWD), (float*)((char*)cg + CG_AXIS_RIGHT),
                     (float*)((char*)cg + CG_AXIS_UP));

    }

    volatile bool s_hasTarget = false;
    volatile bool s_aaHasTarget = false;
    volatile bool s_targetVisible = false;
    volatile float s_aimYaw = 0.0f;
    volatile float s_aimPitch = 0.0f;
    volatile float s_aaAimYaw = 0.0f;
    volatile float s_aaAimPitch = 0.0f;

    float s_spinYaw = 0.0f;
    float s_spinPitch = 0.0f;
    int s_aaTogYaw = 0;
    int s_aaTogPit = 0;
    u32 s_aaRand = 0x1234567u;
    volatile bool s_aaActive = false;
    volatile float s_aaFakeYaw = 0.0f;
    volatile float s_aaFakePitch = 0.0f;
    volatile float s_aaFakeYawDelta = 0.0f;
    bool AntiAimActive() { return s_aaActive; }
    float AntiAimFakeYaw() { return s_aaFakeYaw; }
    float AntiAimFakePitch() { return s_aaFakePitch; }
    u32 AaRand() {
        s_aaRand = s_aaRand * 1103515245u + 12345u;

        return (s_aaRand >> 16) & 0x7FFF;
    }

    volatile int s_call = 0;
    volatile int s_reach = 0;
    volatile int s_cand = 0;
    volatile int s_cost = -1;
    volatile int s_wrote = 0;
    volatile int s_cmdIn = 0;
    volatile int s_myAim = 0;
    int DbgCall() { return s_call; }
    int DbgReach() { return s_reach; }
    int DbgCand() { return s_cand; }
    int DbgCost() { return s_cost; }
    int DbgWrote() { return s_wrote; }
    int DbgCmdIn() { return s_cmdIn; }
    int DbgMyAim() { return s_myAim; }

    volatile int s_path = 0;
    volatile int s_pathMode = -1;
    volatile int s_pathHasTarget = -1;
    int DbgPath() { return s_path; }
    int DbgPathMode() { return s_pathMode; }
    int DbgPathHasTarget() { return s_pathHasTarget; }
    int DbgAutoInput() { return s_autoInput; }
    int DbgAutoCmd() { return s_autoCmd; }
    int DbgAimLock() { return s_hasTarget ? 1 : 0; }
    int DbgFovPass() { return s_targetVisible ? 1 : 0; }

    volatile unsigned s_csVal = 0;
    volatile int s_csNibble = -1;
    unsigned DbgClientState() { return s_csVal; }
    int DbgCSNibble() { return s_csNibble; }

    bool WantAutoShoot() { return CB(CFG_AUTOSHOOT) != 0 && s_hasTarget; }
}
