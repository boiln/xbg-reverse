#include "luda/features/aimbot/auto_wall.hpp"

#include <math.h>

namespace autowall {
    typedef unsigned char u8;
    typedef unsigned short u16;

    static const u32 kEntityStride = 0x374;

    static const u32 kBulletTrace = 0x8224CDA8;

    static const u32 kContinueTrace = 0x826B0148;
    static const u32 kTraceHitId = 0x823F7160;
    static const u32 kPenetrationDepth = 0x826B0278;

    static const u32 kWeaponHasPerk = 0x82691798;
    static const u32 kWeaponDef = 0x826C3E90;
    static const u32 kWeaponMultiplier = 0x826C4EF0;
    static const u32 kTraceAimPoint = 0x823003D8;

    __declspec(align(16)) struct FireParams {
        u8 bytes[0x40];
    };

    __declspec(align(16)) struct TraceResult {
        u8 bytes[0x60];
    };

    static u32& U32(void* p, u32 off) { return *(u32*)((u8*)p + off); }
    static u16& U16(void* p, u32 off) { return *(u16*)((u8*)p + off); }
    static float& F32(void* p, u32 off) { return *(float*)((u8*)p + off); }

    static void Clear(void* p, u32 size) {
        u8* b = (u8*)p;
        for (u32 i = 0; i < size; ++i) b[i] = 0;
    }

    static void Copy(void* dst, const void* src, u32 size) {
        u8* d = (u8*)dst;
        const u8* s = (const u8*)src;
        for (u32 i = 0; i < size; ++i) d[i] = s[i];
    }

    static bool Finite(float f) {
        const u32 bits = *(const u32*)&f;

        return (bits & 0x7F800000u) != 0x7F800000u;
    }

    static bool InitFireParams(FireParams* fp, void* cg, const float eye[3], const float end[3]) {
        // fire parameters require normalized direction and mirrored start fields.
        if (!fp || !cg || !eye || !end) return false;
        Clear(fp, sizeof(*fp));
        U32(fp, 0x00) = 0x3FE;
        U32(fp, 0x04) = U32(cg, 0x48248);
        F32(fp, 0x08) = 1.0f;
        U32(fp, 0x0C) = 9;
        for (int i = 0; i < 3; ++i) {
            F32(fp, 0x10 + i * 4) = eye[i];
            F32(fp, 0x1C + i * 4) = eye[i];
            F32(fp, 0x28 + i * 4) = end[i];
        }
        const float dx = end[0] - eye[0];
        const float dy = end[1] - eye[1];
        const float dz = end[2] - eye[2];
        const float len = sqrtf(dx * dx + dy * dy + dz * dz);
        if (!Finite(len) || len <= 0.0001f) return false;
        F32(fp, 0x34) = dx / len;
        F32(fp, 0x38) = dy / len;
        F32(fp, 0x3C) = dz / len;

        return true;
    }

    typedef int (*BulletTraceFn)(int, FireParams*, void*, TraceResult*, u32, u32);

    typedef int (*ContinueTraceFn)(FireParams*, TraceResult*, float);
    typedef u32 (*TraceHitIdFn)(TraceResult*);
    typedef float (*PenetrationDepthFn)(void*, u32);
    typedef int (*WeaponHasPerkFn)(u32, int);
    typedef void* (*WeaponDefFn)(u32);
    typedef u32 (*WeaponMultiplierFn)(u32, void*, void*);
    typedef void (*TraceAimPointFn)(const float*, const float*, float*);

    static bool RejectedTraceType(TraceResult* tr, bool spectatorMode) {
        return !spectatorMode && U16(tr, 0x28) == 0x14;
    }

    static bool RetraceType14(void* cg, void* targetEnt, int target, FireParams* fp, TraceResult* trace,
                              bool spectatorMode) {
        // trace type 14 requires a target-facing retry.
        TraceAimPointFn aimPointFn = (TraceAimPointFn)kTraceAimPoint;
        BulletTraceFn traceFn = (BulletTraceFn)kBulletTrace;
        ContinueTraceFn continueFn = (ContinueTraceFn)kContinueTrace;

        float adjusted[3] = {0.0f, 0.0f, 0.0f};
        aimPointFn((const float*)((u8*)fp + 0x34), (const float*)trace, adjusted);
        float start[3];
        float end[3];
        for (int i = 0; i < 3; ++i) {
            start[i] = F32(trace, 0x44 + i * 4);
            end[i] = start[i] + adjusted[i] * 100.0f;
        }

        FireParams retry;
        TraceResult retryTrace;

        Clear(&retry, sizeof(retry));
        U32(&retry, 0x00) = 0x3FE;
        U32(&retry, 0x04) = U32(cg, 0x48248);
        F32(&retry, 0x08) = 1.0f;
        U32(&retry, 0x0C) = 9;
        for (int i = 0; i < 3; ++i) {
            F32(&retry, 0x10 + i * 4) = start[i];
            F32(&retry, 0x1C + i * 4) = start[i];
            F32(&retry, 0x28 + i * 4) = end[i];
        }
        const float dx = end[0] - start[0];
        const float dy = end[1] - start[1];
        const float dz = end[2] - start[2];
        const float len = sqrtf(dx * dx + dy * dy + dz * dz);
        if (!Finite(len) || len <= 0.0001f) return false;
        F32(&retry, 0x34) = dx / len;
        F32(&retry, 0x38) = dy / len;
        F32(&retry, 0x3C) = dz / len;

        Clear(&retryTrace, sizeof(retryTrace));
        const int retryHit = traceFn(0, &retry, targetEnt, &retryTrace, 0, 0);
        if (!retryHit) return false;
        continueFn(&retry, &retryTrace, 1.0f);

        if (RejectedTraceType(&retryTrace, spectatorMode)) return false;
        if ((int)U32(&retry, 0x04) != target) return false;
        Copy(trace, &retryTrace, sizeof(*trace));
        F32(fp, 0x08) = 0.30000001192092896f;
        U32(fp, 0x04) = (u32)target;

        return true;
    }
    static void BuildReverse(const FireParams* forward, const TraceResult* hit, const float initialHit[3],
                             FireParams* reverse, TraceResult* reverseTrace) {
        Copy(reverse, forward, sizeof(*reverse));
        Copy(reverseTrace, hit, sizeof(*reverseTrace));
        for (int i = 0; i < 3; ++i) {
            const float dir = -F32((void*)forward, 0x34 + i * 4);
            F32(reverse, 0x34 + i * 4) = dir;
            F32(reverse, 0x1C + i * 4) = F32((void*)forward, 0x28 + i * 4);
            F32(reverse, 0x28 + i * 4) = initialHit[i] + dir * 0.01f;
            F32(reverseTrace, i * 4) = -F32(reverseTrace, i * 4);
        }
    }

    static float DistancePoint(const float a[3], const float b[3]) {
        const float x = b[0] - a[0];
        const float y = b[1] - a[1];
        const float z = b[2] - a[2];

        return sqrtf(x * x + y * y + z * z);
    }

    bool Evaluate(void* cg, void* entities, int localClient, int targetClient, const float eye[3],
                  const float targetPoint[3], bool spectatorMode, Result* result) {
        if (result) {
            result->direct = false;
            result->score = 0.0f;
        }
        if (!cg || !entities || !eye || !targetPoint || !result) return false;

        if (localClient < 0 || localClient >= 18 || targetClient < 0 || targetClient >= 18) return false;

        u8* localEnt = (u8*)entities + localClient * kEntityStride;
        u8* targetEnt = (u8*)entities + targetClient * kEntityStride;
        const u32 weapon = U32(localEnt, 0x1B0);
        const u32 viewWeapon = U32(cg, 0x48260);
        if (!weapon || !viewWeapon) return false;

        WeaponDefFn weaponDefFn = (WeaponDefFn)kWeaponDef;
        PenetrationDepthFn depthFn = (PenetrationDepthFn)kPenetrationDepth;
        BulletTraceFn traceFn = (BulletTraceFn)kBulletTrace;
        ContinueTraceFn continueFn = (ContinueTraceFn)kContinueTrace;
        WeaponHasPerkFn weaponHasPerkFn = (WeaponHasPerkFn)kWeaponHasPerk;
        WeaponMultiplierFn multiplierFn = (WeaponMultiplierFn)kWeaponMultiplier;

        void* weaponDef = weaponDefFn(viewWeapon);
        if (!weaponDef) return false;

        const bool scalesPenetration = weaponHasPerkFn(viewWeapon, 6) != 0;
        const float adsScale = scalesPenetration ? *(float*)(0x845BC7D4u + 0x18u) : 1.0f;
        if (!Finite(adsScale) || adsScale <= 0.0f) return false;

        u8 mulA[12];
        u8 mulB[40];
        Clear(mulA, sizeof(mulA));
        Clear(mulB, sizeof(mulB));

        const float multiplier = (float)multiplierFn(weapon, mulA, mulB);
        if (!Finite(multiplier) || multiplier <= 0.0f) return false;

        FireParams fp;
        TraceResult tr;
        if (!InitFireParams(&fp, cg, eye, targetPoint)) return false;
        Clear(&tr, sizeof(tr));

        const int initialTrace = traceFn(0, &fp, targetEnt, &tr, 0, 0);
        if (!initialTrace) return false;

        if (RejectedTraceType(&tr, spectatorMode)) {
            if (!RetraceType14(cg, targetEnt, targetClient, &fp, &tr, spectatorMode)) return false;

            const float returnedBudget = F32(&fp, 0x08);
            const float score = returnedBudget * multiplier;
            if (!Finite(score) || score <= 0.07f) return false;
            result->direct = F32(&tr, 0x10) >= 1.0f && returnedBudget >= 0.98f;
            result->score = score;
            return true;
        }

        for (int wall = 0; wall < 6; ++wall) {
            float depth = depthFn(weaponDef, U32(&tr, 0x54));
            if (U32(&tr, 0x14) & 4) depth = 100.0f;
            depth *= adsScale;
            if (!Finite(depth) || depth <= 0.0f) {
                break;
            }

            float initialHit[3] = {F32(&tr, 0x44), F32(&tr, 0x48), F32(&tr, 0x4C)};
            TraceHitIdFn hitIdFn = (TraceHitIdFn)kTraceHitId;
            const u32 forwardId = hitIdFn(&tr);
            const int advanced = continueFn(&fp, &tr, 0.135f);

            if (!advanced) {
                break;
            }
            if (RejectedTraceType(&tr, spectatorMode)) {
                U32(&fp, 0x04) = 0xFFFFFFFFu;
                break;
            }

            const u32 surface = U32(&tr, 0x54);
            const bool forwardTrace = traceFn(0, &fp, targetEnt, &tr, surface, 0) != 0;
            if (RejectedTraceType(&tr, spectatorMode)) {
                U32(&fp, 0x04) = 0xFFFFFFFFu;
                break;
            }
            if ((int)forwardId == targetClient) {
                U32(&fp, 0x04) = forwardId;
                U16(&tr, 0x20) = (u16)forwardId;
            }

            FireParams reverse;
            TraceResult reverseTrace;
            BuildReverse(&fp, &tr, initialHit, &reverse, &reverseTrace);
            if (forwardTrace) {
                continueFn(&reverse, &reverseTrace, 0.01f);
            }
            if (RejectedTraceType(&reverseTrace, spectatorMode)) {
                U32(&fp, 0x04) = 0xFFFFFFFFu;
                break;
            }
            const u32 reverseSurface = U32(&reverseTrace, 0x54);
            const bool reverseHit = traceFn(0, &reverse, targetEnt, &reverseTrace, reverseSurface, 0) != 0;
            if (RejectedTraceType(&tr, spectatorMode)) {
                U32(&fp, 0x04) = 0xFFFFFFFFu;
                break;
            }

            const bool solidOverlap = (reverseHit && *((u8*)&reverseTrace + 0x2A) != 0) ||
                                      (*((u8*)&tr + 0x2B) != 0 && *((u8*)&reverseTrace + 0x2B) != 0);
            if ((int)U32(&reverse, 0x04) == targetClient) {
                U32(&fp, 0x04) = (u32)targetClient;
            }

            if (!reverseHit && !solidOverlap) {
                if (!forwardTrace) {
                    break;
                }
                continue;
            }

            if (reverseHit) {
                float reverseDepth = depthFn(weaponDef, U32(&reverseTrace, 0x54));
                reverseDepth *= adsScale;
                if (!Finite(reverseDepth) || reverseDepth <= 0.0f) {
                    break;
                }
                if (reverseDepth < depth) depth = reverseDepth;
                if (U32(&reverseTrace, 0x14) & 4) depth = 100.0f;
            }

            float thickness = 0.0f;
            if (solidOverlap) {
                const float reverseEnd[3] = {F32(&reverse, 0x28), F32(&reverse, 0x2C), F32(&reverse, 0x30)};
                const float forwardEnd[3] = {F32(&tr, 0x44), F32(&tr, 0x48), F32(&tr, 0x4C)};
                thickness = DistancePoint(forwardEnd, reverseEnd);
            } else {
                const float reverseEnd[3] = {F32(&reverseTrace, 0x44), F32(&reverseTrace, 0x48),
                                             F32(&reverseTrace, 0x4C)};
                thickness = DistancePoint(initialHit, reverseEnd);
            }
            if (!Finite(thickness) || thickness < 0.0f) return false;
            if (thickness < 1.0f) thickness = 1.0f;
            F32(&fp, 0x08) -= thickness / depth;
            if (!Finite(F32(&fp, 0x08)) || F32(&fp, 0x08) <= 0.0f) {
                break;
            }
        }
        if ((int)U32(&fp, 0x04) != targetClient || F32(&fp, 0x08) <= 0.0f) {
            return false;
        }
        const float score = F32(&fp, 0x08) * multiplier;

        if (!Finite(score) || score <= 0.07f) {
            return false;
        }
        result->direct = false;
        result->score = score;
        return true;
    }
}
