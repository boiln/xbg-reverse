#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace aimbot {
    static void PatchWord(u32 address, u32 word) {

        volatile u32* p = (volatile u32*)address;
        if (*p == word) return;
        DWORD oldProtect = 0;
        if (!VirtualProtect((void*)p, 4, PAGE_EXECUTE_READWRITE, &oldProtect)) return;
        *p = word;
        __dcbst(0, (void*)p);
        __sync();
        __emit(0x4C00012C);
        DWORD ignored = 0;

        VirtualProtect((void*)p, 4, oldProtect, &ignored);

    }

    void ApplyAimbotPatches(void* cg) {

        const u32 nop = 0x60000000;
        bool recoil   = CB(CFG_RECOIL) != 0;
        // enabled paths use bypass words; disabled paths restore stock instructions.
        PatchWord(0x82259BC8, recoil ? 0x38600001 : 0x48461341);
        PatchWord(0x8223AC00, recoil ? nop : 0x4800000C);

        if (recoil && cg) {
            *(u32*)((char*)cg + 0x697F8) = 0;
            *(u32*)((char*)cg + 0x697FC) = 0;
            *(u32*)((char*)cg + 0x69800) = 0;
        }

        PatchWord(0x826C6E6C, CB(CFG_SWAY) ? nop : 0x4BFFE975);
        PatchWord(0x826C7A64, CB(CFG_FLINCH) ? nop : 0x4BFFF95D);

    }

    float AimFovPixels() { return *(volatile float*)0x83C59ED8 * 2.0f; }
    volatile float s_viewYaw   = 0.0f;
    volatile float s_viewPitch = 0.0f;

    void* CG() {
        void** pp = (void**)(u64)A_CG_Pointer;

        return pp ? *pp : 0;
    }

    char* ClientState() {
        void** pp = (void**)(u64)A_LocalClient;

        return pp ? (char*)*pp : 0;
    }

    void ViewRef(float* pitch, float* yaw) {

        char* cs = ClientState();

        if (cs && (((u32)(u64)cs) >> 28) == 0xB) {
            *pitch = RF(cs, CS_VPITCH);
            *yaw   = RF(cs, CS_VYAW);

            return;
        }

        *pitch = s_viewPitch;
        *yaw   = s_viewYaw;

    }

    char* Entities() {
        void** pp = (void**)(u64)A_EntitiesPointer;

        return pp ? (char*)*pp : 0;
    }

    bool WorldToScreen(void* cg, const Vec3& world, Vec2* out) {

        if (!cg || !out) return false;

        char* rd     = (char*)cg + CG_REFDEF;
        Vec3 eye     = RV3(rd, RD_ORG);
        Vec3 forward = RV3(rd, RD_AXIS);
        Vec3 right   = RV3(rd, RD_AXIS + 12);
        Vec3 up      = RV3(rd, RD_AXIS + 24);
        Vec3 d       = { world.x - eye.x, world.y - eye.y, world.z - eye.z };
        float tx     = d.x * right.x + d.y * right.y + d.z * right.z;
        float ty     = d.x * up.x + d.y * up.y + d.z * up.z;
        float tz     = d.x * forward.x + d.y * forward.y + d.z * forward.z;
        float fx     = RF(rd, RD_FOV);
        float fy     = RF(rd, RD_FOV + 4);
        if (tz < 0.1f || fx == 0.0f || fy == 0.0f) return false;

        out->x = RI(rd, RD_W) * 0.5f * (1.0f - tx / fx / tz);
        out->y = RI(rd, RD_H) * 0.5f * (1.0f - ty / fy / tz);

        return true;

    }

    TeamCheck_t pTeam = (TeamCheck_t)A_TeamCheck;
    WeaponDef_t pWeaponDef = (WeaponDef_t)A_WeaponDef;
    GetViewmodelWeapon_t pGetViewmodelWeapon = (GetViewmodelWeapon_t)A_GetViewmodelWeapon;
    GetSpreadForWeapon_t pGetSpreadForWeapon = (GetSpreadForWeapon_t)A_GetSpreadForWeapon;
    SeedFn_t pSeedScramble = (SeedFn_t)A_SeedScramble;
    RandomFloat_t pRandomFloat = (RandomFloat_t)A_RandomFloat;
    SeedFn_t pAdvanceSeed4 = (SeedFn_t)A_AdvanceSeed4;
    BulletTrace_t pBulletTrace = (BulletTrace_t)A_BulletTrace;
    WeaponPredicate_t pIsDualWield = (WeaponPredicate_t)A_IsDualWield;
    SessionPredicate_t pIsZombieSession = (SessionPredicate_t)A_IsZombieSession;
    GetPlayerViewOrigin_t pGetPlayerViewOrigin = (GetPlayerViewOrigin_t)A_GetPlayerViewOrigin;
    CalcEntityLerpPositions_t pCalcEntityLerpPositions = (CalcEntityLerpPositions_t)A_CalcEntityLerpPositions;
    ClientDObj_t pDObj = (ClientDObj_t)A_ClientDObj;
    SLGet_t pSL = (SLGet_t)A_SLGetString;
    TagPos_t pTag = (TagPos_t)A_TagPos;

    bool TagPos(char* base, int idx, const char* tag, Vec3* out) {

        char* e  = base + idx * ENT_STRIDE;
        int dobj = pDObj(idx, 0);
        if (!e || !dobj) return false;

        int ti = pSL(tag, 0);

        return pTag(e, dobj, ti, out) != 0;

    }

    void ApplyAimPrediction(void* cg, char* entities, int targetIdx, Vec3* point) {

        if (!cg || !entities || !point || targetIdx < 0 || targetIdx >= 18) return;

        char* entity  = entities + targetIdx * ENT_STRIDE;
        Vec3 adjusted = *point;

        if (CB(CFG_POSITION_PREDICT)) {
            __try {
                unsigned char snapshot[ENT_STRIDE];

                for (int i = 0; i < ENT_STRIDE; ++i) snapshot[i] = (unsigned char)entity[i];
                pCalcEntityLerpPositions(cg, snapshot, 0);
                float amount = CF(CFG_POSITION_PREDICT_AMOUNT);

                adjusted.x += (*(float*)(snapshot + E_ORIGIN + 0)- RF(entity, E_ORIGIN + 0)) * amount;
                adjusted.y += (*(float*)(snapshot + E_ORIGIN + 4)- RF(entity, E_ORIGIN + 4)) * amount;
                adjusted.z += (*(float*)(snapshot + E_ORIGIN + 8)- RF(entity, E_ORIGIN + 8)) * amount;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
            }
        }

        __try {
            float oldX = RF(entity, 0x1E8);
            float oldY = RF(entity, 0x1EC);
            float oldZ = RF(entity, 0x1F0);
            float curX = RF(entity, E_ORIGIN + 0);
            float curY = RF(entity, E_ORIGIN + 4);
            float curZ = RF(entity, E_ORIGIN + 8);

            if (!CB(CFG_PING_PREDICT)) {
                float amount = CF(CFG_PREDICTION_AMOUNT);

                if (amount > 0.1f) {
                    adjusted.x -= (curX - oldX) * amount * 0.25f;
                    adjusted.y -= (curY - oldY) * amount * 0.25f;
                    adjusted.z -= (curZ - oldZ) * amount * 0.25f;
                }
            } else {
                u32 firstPing     = *(volatile u32*)((char*)cg + 0x84);
                u32 secondPing    = *(volatile u32*)((char*)cg + 0x24084);
                float pingSeconds = (float)((firstPing + secondPing)>> 1)* 0.001f;

                adjusted.x += (curX - oldX) * pingSeconds;
                adjusted.y += (curY - oldY) * pingSeconds;
                adjusted.z += (curZ - oldZ) * pingSeconds;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
        }

        *point = adjusted;

    }
}
