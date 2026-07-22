#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace esp {
    bool TagPos(void* base, int idx, const char* tag, Vec3* out) {

        char* e = Entity(base, idx);
        int dobj = pDObj(idx, 0);
        if (!e || !dobj) return false;
        int ti = pSL(tag, 0);

        return pTag(e, dobj, ti, out) != 0;

    }
    static const char* WeaponName(void* base, int idx) {

        char* e = Entity(base, idx);
        if (!e) return "";
        int w = RI(e, E_WEAPIDX) & 0xFF;
        if (w == 0 || w > pGetNumWeapons()) return "";
        void* def = pWDef2(w);
        if (!def) return "";
        const char* key = *(const char**)((char*)def + 0x0C);
        const char* n = key ? pLocalizeString(key) : 0;

        return n && n[0] ? n : "";

    }

    struct BonePair {
        const char* child;
        const char* parent;
    };
    static const BonePair kSkel[] = {
        {"tag_eye", "j_neck"},           {"j_neck", "j_shoulder_le"},     {"j_neck", "j_shoulder_ri"},
        {"j_shoulder_le", "j_elbow_le"}, {"j_shoulder_ri", "j_elbow_ri"}, {"j_elbow_le", "j_wrist_le"},
        {"j_elbow_ri", "j_wrist_ri"},    {"j_neck", "j_spinelower"},      {"j_spinelower", "j_knee_le"},
        {"j_spinelower", "j_knee_ri"},   {"j_knee_le", "j_ankle_le"},     {"j_knee_ri", "j_ankle_ri"},
        {"j_ankle_le", "j_ball_le"},     {"j_ankle_ri", "j_ball_ri"}};
    static const int kSkelN = (int)(sizeof(kSkel) / sizeof(kSkel[0]));

    static bool IsZero(const Vec3& v) { return v.x == 0.0f && v.y == 0.0f && v.z == 0.0f; }

    static void DrawBone(void* base, void* cg, int idx, const char* ca, const char* cb, ARGB col) {

        Vec3 a;
        Vec3 b;
        Vec2 sa;
        Vec2 sb;

        if (!TagPos(base, idx, ca, &a) || !TagPos(base, idx, cb, &b)) return;
        if (IsZero(a) || IsZero(b)) return;
        if (!W2S(cg, a, &sa) || !W2S(cg, b, &sb)) return;

        DrawLine(sa.x, sa.y, sb.x, sb.y, col);

    }

    struct Box {
        float l;
        float r;
        float top;
        float bot;
        float distance;
        bool ok;
    };
    static Vec3 HeadTop(void* base, int idx, char* ent) {

        Vec3 h = {0.0f, 0.0f, 0.0f};

        if (TagPos(base, idx, "j_head", &h) && !IsZero(h)) {
            h.z += 10.0f;
            return h;
        }
        h.x = 0.0f;
        h.y = 0.0f;
        h.z = 10.0f;

        return h;

    }
    static Box PlayerBox(void* base, void* cg, int idx, char* ent) {

        Box b;
        b.ok = false;
        b.distance = 0.0f;

        Vec3 feet = RV3(ent, E_ORIGIN);
        Vec3 head = {0.0f, 0.0f, 0.0f};

        TagPos(base, idx, "j_head", &head);

        Vec2 sh;
        Vec2 sf;

        if (!W2S(cg, head, &sh) || !W2S(cg, feet, &sf)) return b;

        int local = RI(cg, CG_CLIENTNUM);
        Vec3 localOrigin = RV3((char*)base + local * ENT_STRIDE, E_ORIGIN);
        Vec3 targetOrigin = RV3(ent, E_ORIGIN);

        float dx = targetOrigin.x - localOrigin.x;
        float dy = targetOrigin.y - localOrigin.y;
        float dz = targetOrigin.z - localOrigin.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        b.distance = dist;

        if (dist > 0.0f) {
            sh.y -= 10000.0f / dist;
            sf.y += 5000.0f / dist;
        }

        float h = sf.y - sh.y;
        float w = h * 0.50f;
        float cx = sh.x;

        b.top = sh.y;
        b.bot = sf.y;
        b.l = cx - w * 0.5f;
        b.r = cx + w * 0.5f;
        b.ok = true;

        return b;

    }
    static void Box2D(const Box& b, ARGB c) {

        float pad = b.distance * 0.0005f;
        DrawRect(b.l, b.top, (b.r - b.l) + pad, 2.0f, c);
        DrawRect(b.l, b.bot, (b.r - b.l) + pad, 2.0f, c);
        DrawRect(b.l, b.top, 2.0f, (b.bot - b.top) + pad, c);
        DrawRect(b.r, b.top, 2.0f, (b.bot - b.top) + pad, c);

    }
    static void BoxCorner(const Box& b, ARGB c) {

        float extent = (b.bot - b.top) + b.distance * 0.001f;
        float quarter = extent * 0.25f;
        float arm = extent * 0.16666667f;
        float horizontal = arm + b.distance * 0.0005f;
        float vertical = extent * 0.2f + b.distance * 0.0005f;
        float cx = (b.l + b.r) * 0.5f;
        float left = cx - quarter;
        float right = cx + quarter;
        float bottomVertical = b.bot - extent * 0.198059037f;
        DrawRect(left, b.top, horizontal, 2.0f, c);
        DrawRect(cx + quarter - arm, b.top, horizontal, 2.0f, c);
        DrawRect(left, b.bot, horizontal, 2.0f, c);
        DrawRect(cx + quarter - arm, b.bot, horizontal, 2.0f, c);
        DrawRect(left, b.top, 2.0f, vertical, c);
        DrawRect(right, b.top, 2.0f, vertical, c);
        DrawRect(left, bottomVertical, 2.0f, vertical, c);
        DrawRect(right, bottomVertical, 2.0f, vertical, c);

    }

    static Vec3 RotatePointExact(const Vec3& point, const Vec3& origin, const Vec3& angles) {

        const float degreesToRadians = 0.01745329252f;
        float dx = point.x - origin.x;
        float dy = point.y - origin.y;
        float dz = point.z - origin.z;
        float cz = cosf(angles.z * degreesToRadians);
        float sz = sinf(angles.z * degreesToRadians);
        float ty = dy * cz - dz * sz;
        float tz = dz * cz + dy * sz;
        float cx = cosf(angles.x * degreesToRadians);
        float sx = sinf(angles.x * degreesToRadians);
        float tx = tz * sx + dx * cx;
        float tz2 = tz * cx - dx * sx;
        float cy = cosf(angles.y * degreesToRadians);
        float sy = sinf(angles.y * degreesToRadians);
        Vec3 out = {tx * cy - ty * sy + origin.x, ty * cy + tx * sy + origin.y, tz2 + origin.z};

        return out;

    }

    static bool EntityAlivePredicate(char* ent) {

        if (((u32)RI(ent, E_FLAGS) & 0x20u) != 0) return false;
        u32 stateBits = ((u32)RI(ent, 0) + 0xE0u) & 0x1400u;

        if ((RB(ent, E_ALIVE) & 0x40u) != 0 && ((u32)RI(ent, 0x1D4) & 0x40000u) == 0 && stateBits != 0) return true;

        return stateBits == 0;

    }
    void Box3D(void* base, void* cg, int idx, char* ent, ARGB col) {

        Vec3 o = RV3(ent, E_ORIGIN);
        Vec3 mins;
        Vec3 maxs;

        int modelHandle = RI(ent, 0x1D0);
        void* dobj = (void*)(u64)(u32)pDObj(modelHandle, 0);

        if (!dobj) return;

        pDObjBounds(dobj, &mins, &maxs);
        if (!(mins.x <= maxs.x && mins.y <= maxs.y && mins.z <= maxs.z)) return;

        short entityType = *(short*)(ent + E_TYPE);
        void** cgsPointer = (void**)(u64)A_CGS_Pointer;
        void* cgs = cgsPointer ? *cgsPointer : 0;
        int maxClients = cgs ? RI(cgs, 0x150) : 0;

        bool specialPlayerBounds = (entityType & 0x10) == 0 && (entityType & 1) != 0 && idx < maxClients;

        if ((entityType & 0x10) != 0 && CB(0x90B4328E) != 0 && RI(ent, 0x1AC) != 0 && EntityAlivePredicate(ent))
            specialPlayerBounds = true;
        Vec3 head = {0.0f, 0.0f, 0.0f};

        if (specialPlayerBounds) {
            TagPos(base, idx, "j_head", &head);
            if (IsZero(head)) specialPlayerBounds = false;
        }

        if (specialPlayerBounds) {
            mins.x *= 1.1f * 1.52f;
            mins.y *= 1.1f;
            mins.z -= 12.0f;
            maxs.x *= 1.1f * 1.52f;
            maxs.y *= 1.1f;
            maxs.z = head.z + 16.0f - o.z;
        }
        mins.x += o.x;
        mins.y += o.y;
        mins.z += o.z;
        maxs.x += o.x;
        maxs.y += o.y;
        maxs.z += o.z;
        Vec3 world[8] = {
            {mins.x, mins.y, mins.z}, {maxs.x, mins.y, mins.z}, {maxs.x, maxs.y, mins.z},
            {mins.x, maxs.y, mins.z}, {mins.x, mins.y, maxs.z}, {maxs.x, mins.y, maxs.z},
            {maxs.x, maxs.y, maxs.z}, {mins.x, maxs.y, maxs.z}};

        Vec3 ang = RV3(ent, E_ANGLES);
        Vec3 cc[8];

        for (int i = 0; i < 8; ++i) cc[i] = RotatePointExact(world[i], o, ang);

        Vec2 s[8];

        for (int i = 0; i < 8; ++i)
            if (!W2S(cg, cc[i], &s[i])) return;
        static const int E[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
            {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

        for (int i = 0; i < 12; ++i) {
            int a = E[i][0];
            int d = E[i][1];

            DrawLine(s[a].x, s[a].y, s[d].x, s[d].y, col);
        }

    }
    static void DrawSnap(void* cg, const Vec2& target, ARGB c) {

        char* rd = (char*)cg + CG_REFDEF;

        float W = (float)RI(rd, RD_W);
        float H = (float)RI(rd, RD_H);

        int mode = CB(V_SNAPLINES);
        Vec2 source = {W * 0.5f, mode == 3 ? 30.0f : mode == 2 ? H * 0.5f : H - 30.0f};

        if (mode == 1 && CB(0x90B433A1)) {
            Vec3 third = RV3(cg, 0x480D0);
            if (!W2S(cg, third, &source)) return;
        }
        DrawLine(target.x, target.y, source.x, source.y, c);

    }

    static void DrawFilledPointerTriangle(float tipX, float tipY, float dirX, float dirY, ARGB col) {

        const int slices = 14;
        const float length = 14.0f;
        const float halfBase = 8.0f;
        float perpX = -dirY;
        float perpY = dirX;
        for (int i = 1; i <= slices; ++i) {
            float t = (float)i / (float)slices;
            float centerX = tipX - dirX * length * t;
            float centerY = tipY - dirY * length * t;
            float halfWidth = halfBase * t;
            DrawLine(centerX - perpX * halfWidth, centerY - perpY * halfWidth, centerX + perpX * halfWidth,
                     centerY + perpY * halfWidth, col);
        }
        DrawRect(tipX - 1.0f, tipY - 1.0f, 2.0f, 2.0f, col);

    }
    void DrawPointer(void* cg, char* ent, ARGB col) {

        char* rd = (char*)cg + CG_REFDEF;

        float W = (float)RI(rd, RD_W);
        float H = (float)RI(rd, RD_H);

        if (W <= 0.0f || H <= 0.0f) return;

        Vec3 o = RV3(ent, E_ORIGIN);
        Vec2 sp;

        bool projected = W2S(cg, o, &sp);
        if (projected && sp.x >= 0.0f && sp.x <= W && sp.y >= 0.0f && sp.y <= H) return;

        float centerX = W * 0.5f;
        float centerY = H * 0.5f;
        float dirX;
        float dirY;

        if (projected) {
            dirX = sp.x - centerX;
            dirY = sp.y - centerY;
        } else {
            Vec3 view = RV3(rd, RD_ORG);
            Vec3 right = RV3(rd, RD_AXIS + 12);
            Vec3 up = RV3(rd, RD_AXIS + 24);
            Vec3 forward = RV3(rd, RD_AXIS);
            Vec3 delta = {o.x - view.x, o.y - view.y, o.z - view.z};
            float side = delta.x * right.x + delta.y * right.y + delta.z * right.z;
            float vertical = delta.x * up.x + delta.y * up.y + delta.z * up.z;
            float depth = delta.x * forward.x + delta.y * forward.y + delta.z * forward.z;
            dirX = -side;
            dirY = -vertical;
            if (depth < 0.0f) {
                dirX = -dirX;
                dirY = -dirY;
            }
        }
        float magnitude = sqrtf(dirX * dirX + dirY * dirY);

        if (magnitude < 0.001f) {
            dirX = 0.0f;
            dirY = 1.0f;
        } else {
            dirX /= magnitude;
            dirY /= magnitude;
        }
        const float margin = 24.0f;
        float edgeX = 1000000.0f;
        float edgeY = 1000000.0f;
        if (fabsf(dirX) > 0.001f) edgeX = (centerX - margin) / fabsf(dirX);
        if (fabsf(dirY) > 0.001f) edgeY = (centerY - margin) / fabsf(dirY);
        float edgeDistance = edgeX < edgeY ? edgeX : edgeY;
        DrawFilledPointerTriangle(centerX + dirX * edgeDistance, centerY + dirY * edgeDistance, dirX, dirY, col);

    }
    static const char* FmtDist(const Vec3& e, const Vec3& l, char* buf) {

        Vec3 d = {e.x - l.x, e.y - l.y, e.z - l.z};
        float m = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z) * 1.20319998f;

        int whole = (int)m;
        int frac = (int)((m - whole) * 100.0f + 0.5f);

        if (frac >= 100) {
            whole++;
            frac = 0;
        }
        char t[16];
        int n = 0;
        if (!whole) t[n++] = '0';
        {
            int v = whole;
            char tmp[12];
            int k = 0;
            while (v) {
                tmp[k++] = (char)('0' + v % 10);
                v /= 10;
            }
            while (k) t[n++] = tmp[--k];
        }
        int j = 0;
        for (int i = 0; i < n; ++i) buf[j++] = t[i];
        buf[j++] = '.';
        buf[j++] = (char)('0' + frac / 10);
        buf[j++] = (char)('0' + frac % 10);
        buf[j++] = 'm';
        buf[j] = 0;

        return buf;

    }

    void DrawPlayer(void* base, void* cg, int idx, char* ent, ARGB col, const Vec3& localOrg) {

        Box b = PlayerBox(base, cg, idx, ent);
        Vec3 o = RV3(ent, E_ORIGIN);
        Vec3 feetPoint = o;
        feetPoint.z -= 5.0f;

        Vec3 head = HeadTop(base, idx, ent);
        Vec2 sf;
        Vec2 sh;

        bool okFeet = W2S(cg, feetPoint, &sf);
        bool okHead = W2S(cg, head, &sh);

        if (!okHead && !okFeet) return;

        float baseX = sf.x;

        int type = CB(V_TYPE);

        if (type == 2)
            Box3D(base, cg, idx, ent, col);
        else if (type == 3 && b.ok)
            BoxCorner(b, col);
        else if (type == 1 && b.ok)
            Box2D(b, col);
        if (CB(V_SNAPLINES)) DrawSnap(cg, sf, col);

        if (CB(V_SKELETON))
            for (int i = 0; i < kSkelN; ++i) DrawBone(base, cg, idx, kSkel[i].child, kSkel[i].parent, col);

        if (CB(V_DIST)) {
            char db[24];
            BoxedLabel(baseX, sh.y - 5.0f, FmtDist(o, localOrg, db), col);
        }

        if (idx < 18 && CB(V_WEAPNAMES)) {
            const char* wn = WeaponName(base, idx);
            if (wn[0]) {
                BoxedLabel(baseX, sf.y + 20.0f, wn, col);
            }
        }

        if (idx < 18 && CB(V_NAMES)) {
            const char* nm = (char*)cg + CG_CLIENTINFO + idx * CI_STRIDE + CI_NAME;
            if (nm[0]) BoxedLabel(baseX, sh.y - 24.0f, nm, col);
        }

    }

    void ConstantRadar(bool on) {

        // toggling uses a nop and restores the stock conditional branch.
        u32* p = (u32*)0x821B8FD4ull;

        if (s_radarState < 0) {
            s_radarOwn = (*p == 0x419A0060u);
            s_radarState = 0;
        }
        if (!s_radarOwn) return;
        int want = on ? 1 : 0;
        if (s_radarState == want) return;
        DWORD old;
        if (!VirtualProtect(p, 4, PAGE_EXECUTE_READWRITE, &old)) return;
        *p = on ? 0x60000000u : 0x419A0060u;
        __dcbst(0, p);
        __sync();
        __emit(0x4C00012C);
        VirtualProtect(p, 4, old, &old);
        s_radarState = want;

    }
}
