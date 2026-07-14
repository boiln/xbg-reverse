#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace esp {
    FindAsset_t pFind = (FindAsset_t)A_FindAsset;
    DrawText_t pText = (DrawText_t)A_DrawText;
    TextWidth_t pTextW = (TextWidth_t)A_TextWidth;
    Stretch_t pStretch = (Stretch_t)A_StretchPic;
    RotPic_t pRot = (RotPic_t)A_RotPic;
    ClientDObj_t pDObj = (ClientDObj_t)A_ClientDObj;
    SLGet_t pSL = (SLGet_t)A_SLGetString;
    TagPos_t pTag = (TagPos_t)A_TagPos;
    TeamCheck_t pTeam = (TeamCheck_t)A_TeamCheck;
    WeaponDef_t pWDef = (WeaponDef_t)A_WeaponDef;
    WeaponDef_t pWDef2 = (WeaponDef_t)A_WeaponDef2;
    GetNumWeapons_t pGetNumWeapons = (GetNumWeapons_t)A_GetNumWeapons;
    LocalizeString_t pLocalizeString = (LocalizeString_t)A_LocalizeString;
    ModelIndex_t pModelIndex = (ModelIndex_t)A_ModelIndex;
    CalcMuzzle_t pCalcMuzzle = (CalcMuzzle_t)A_CalcMuzzle;
    DObjBounds_t pDObjBounds = (DObjBounds_t)A_DObjBounds;
    DrawTracer3D_t pDrawTracer3D = (DrawTracer3D_t)A_DrawTracer3D;
    SessionPredicate_t pIsZombieSession = (SessionPredicate_t)A_IsZombieSession;

    int s_font = 0, s_white = 0;
    int s_radarState = -1, s_crossState = -1;
    bool s_radarOwn = false, s_crossOwn = false;
    ARGB s_accent = 0xFFBD89FF;
    TracerRecord s_tracers[19];
    int s_tracerCount = 0;

    void* CG() {
        void** pp = (void**)(u64)A_CG_Pointer;

        return pp ? *pp : 0;
    }
    char* Entities() {
        void** pp = (void**)(u64)A_EntitiesPointer;

        return pp ? (char*)*pp : 0;
    }
    char* Entity(void* base, int i) { return base ? (char*)base + i * ENT_STRIDE : 0; }

    void Vec4Of(ARGB a, float* c) {
        c[0] = ((a >> 16) & 0xFF) / 255.0f;
        c[1] = ((a >> 8) & 0xFF) / 255.0f;
        c[2] = (a & 0xFF) / 255.0f;
        c[3] = ((a >> 24) & 0xFF) / 255.0f;
    }
    ARGB Pack(const float* rgba, ARGB fb) {
        if (!rgba || rgba[3] <= 0.0f) return fb;
        int r = (int)(rgba[0] * 255), g = (int)(rgba[1] * 255), b = (int)(rgba[2] * 255), a = (int)(rgba[3] * 255);
        if (r < 0) r = 0;
        if (r > 255) r = 255;
        if (g < 0) g = 0;
        if (g > 255) g = 255;
        if (b < 0) b = 0;
        if (b > 255) b = 255;
        if (a < 0) a = 0;
        if (a > 255) a = 255;

        return ((ARGB)a << 24) | ((ARGB)r << 16) | ((ARGB)g << 8) | (ARGB)b;
    }

    bool W2S(void* cg, const Vec3& w, Vec2* o) {
        if (!cg || !o) return false;
        char* rd = (char*)cg + CG_REFDEF;
        char* placement = (char*)(u64)A_ScreenPlacement;
        Vec3 vo = RV3(rd, RD_ORG);
        Vec3 fwd = RV3(rd, RD_AXIS), right = RV3(rd, RD_AXIS + 12), up = RV3(rd, RD_AXIS + 24);
        float fx = RF(rd, RD_FOV), fy = RF(rd, RD_FOV + 4);
        Vec3 d = {w.x - vo.x, w.y - vo.y, w.z - vo.z};
        float tx = d.x * right.x + d.y * right.y + d.z * right.z;
        float ty = d.x * up.x + d.y * up.y + d.z * up.z;
        float tz = d.x * fwd.x + d.y * fwd.y + d.z * fwd.z;
        float width = RF(placement, 0x40);
        float height = RF(placement, 0x44);

        if (tz < 0.0f) {
            o->x = -tx;
            o->y = -ty;
            if (fabsf(o->x) < 0.001f) {
                if (fabsf(o->y) < 0.001f) {
                    o->y = height * 2.0f;
                    return false;
                }
                o->x = 0.001f;
            }
            if (fabsf(o->y) < 0.001f) o->y = 0.001f;
            while (fabsf(o->x) < width) {
                o->x *= width;
                o->y *= width;
            }
            while (fabsf(o->y) < height) {
                o->x *= height;
                o->y *= height;
            }
            return false;
        }
        float inverseDepth = 1.0f / tz;
        o->x = -((tx / fx) * inverseDepth - 1.0f) * width * 0.5f + RF(placement, 0x38);
        o->y = -((ty / fy) * inverseDepth - 1.0f) * height * 0.5f + RF(placement, 0x3C);

        return true;
    }

    float TextWSc(const char* s, float scale) {
        if (!s || !s[0]) return 0.0f;

        if (s_font) {
            int w = pTextW(0, s, 0x7FFFFFFF, s_font);
            if (w > 0) return (float)w * scale;
        }
        int n = 0;
        while (s[n]) ++n;

        return n * scale * 20.0f;
    }
    int FontH() {
        if (!s_font) return 30;
        int h = *(int*)((char*)(u64)(u32)s_font + 4);

        return (h > 0 && h < 512) ? h : 30;
    }
    float TextW(const char* s) { return TextWSc(s, kScale); }
    void DrawText(const char* s, float x, float y, ARGB col) {
        if (!s || !s_font) return;
        float c[4];
        Vec4Of(col, c);
        pText(s, 0x7FFFFFFF, s_font, x, y, kScale, kScale, 0.0f, c, 4);
    }
    void DrawRect(float x, float y, float w, float h, ARGB col) {
        if (!s_white) return;
        float c[4];
        Vec4Of(col, c);
        pStretch(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, c, s_white);
    }

    void DrawLine(float x0, float y0, float x1, float y1, ARGB col) {
        if (!s_white) return;
        float dx = x1 - x0, dy = y1 - y0, len = sqrtf(dx * dx + dy * dy);
        if (len < 0.01f) return;
        float ang = atanf(dy / dx) * 57.295776f, thick = 1.0f, c[4];
        Vec4Of(col, c);
        pRot(A_ScreenPlacement, (x0 + x1) * 0.5f - len * 0.5f, (y0 + y1) * 0.5f, len, thick, ang, c, s_white);
    }

    static bool ReadableVec3(const void* value) {
        if (!value) return false;
        u32 address = (u32)value;
        if ((address & 3u) != 0) return false;
        if (address > 0xFFFFFFF4u) return false;
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(value, &mbi, sizeof(mbi))) return false;
        if (mbi.State != MEM_COMMIT) return false;
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
        u32 regionStart = (u32)mbi.BaseAddress;
        u32 regionEnd = regionStart + (u32)mbi.RegionSize;
        if (regionEnd < regionStart) return false;

        return address >= regionStart && address + sizeof(Vec3) <= regionEnd;
    }

    void OnBulletEmit(unsigned owner, const void* endpoint) {
        if (!CB(V_TRACERS) || !ReadableVec3(endpoint)) return;
        void* cg = CG();
        char* entities = Entities();
        if (!cg || !entities) return;
        int local = RI(cg, CG_CLIENTNUM);
        if ((int)owner != local) return;
        short type = *(short*)(entities + local * ENT_STRIDE + E_TYPE);
        if (type != 1 && type != 0x10) return;
        TracerRecord rec;
        rec.start.x = rec.start.y = rec.start.z = 0.0f;

        if (CB(0x90B433A1)) {
            if (!TagPos(entities, local, "tag_flash", &rec.start)) return;
        } else {
            u8 scratch[0xB4];
            for (int i = 0; i < (int)sizeof(scratch); ++i) scratch[i] = 0;
            char* ent = entities + local * ENT_STRIDE;
            u32 tagFlash = (u32)(unsigned short)pSL("tag_flash", 0);
            int weapon = RI(ent, E_WEAPIDX);
            void* ps = (char*)cg + 0x480A8;
            if (!pCalcMuzzle(0, ent, tagFlash, ps, weapon, 0x20, 1, scratch + 0xB0, &rec.start, scratch + 0xA0,
                             scratch + 0xAC, scratch + 0x70, scratch + 0x60, scratch + 0xA8, scratch + 0xA4))
                return;
        }
        rec.end = *(const Vec3*)endpoint;
        rec.born = GetTickCount();
        rec.rgba[0] = CFp(C_ACCENT)[0];
        rec.rgba[1] = CFp(C_ACCENT)[1];
        rec.rgba[2] = CFp(C_ACCENT)[2];
        rec.rgba[3] = 0.3f;

        if (s_tracerCount < 19) {
            s_tracers[s_tracerCount++] = rec;
        }
    }

    int TracerChannel(float value) { return value < 1.0f ? (int)(value * 255.0f) : 255; }
}  // namespace esp
