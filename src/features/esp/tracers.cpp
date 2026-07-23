#include "internal.hpp"

#include <math.h>
#include <ppcintrinsics.h>
#include <xtl.h>

namespace esp {
    void DrawTracerList(void* cg) {
        if (!CB(V_TRACERS)) {
            s_tracerCount = 0;

            return;
        }

        char* entities = Entities();
        int local = cg ? RI(cg, CG_CLIENTNUM) : -1;

        if (!cg || !entities || !PlayerEligible(cg, entities, local)) {
            s_tracerCount = 0;

            return;
        }

        while (s_tracerCount >= 15) {
            for (int i = 1; i < s_tracerCount; ++i) s_tracers[i - 1] = s_tracers[i];
            --s_tracerCount;
        }

        int material = pFind(6, "gfx_dollycam_tracer", 0);
        u32 now = GetTickCount();
        int write = 0;

        for (int i = 0; i < s_tracerCount; ++i) {
            u32 age = now - s_tracers[i].born;
            float alpha = 0.8f - (float)age * 0.00033333333f;
            if (alpha <= 0.0f) continue;
            s_tracers[i].rgba[3] = alpha;
            int r = TracerChannel(s_tracers[i].rgba[0]);
            int g = TracerChannel(s_tracers[i].rgba[1]);
            int b = TracerChannel(s_tracers[i].rgba[2]);
            int a = TracerChannel(s_tracers[i].rgba[3]);
            struct NativeTracer {
                Vec3 start;
                Vec3 end;
                float width;
                u32 color;
                int material;
            } command;
            Vec2 startScreen;
            Vec2 endScreen;
            bool startVisible = W2S(cg, s_tracers[i].start, &startScreen);
            bool endVisible = W2S(cg, s_tracers[i].end, &endScreen);

            if (material && (startVisible || endVisible)) {
                command.start = s_tracers[i].start;
                command.end = s_tracers[i].end;
                command.width = 0.15f;
                command.color =
                    (((((u32)(a & 255) / 30u) << 12) | ((u32)(r & 255) / 20u)) << 8 | ((u32)(g & 255) / 20u)) << 8 |
                    ((u32)(b & 255) / 20u);
                command.material = material;
                pDrawTracer3D(&command);
            }

            s_tracers[write++] = s_tracers[i];
        }

        s_tracerCount = write;
    }
    static void DrawLabel(float cx, float y, const char* s, ARGB col) {
        if (!s || !s[0]) return;

        DrawText(s, cx - TextW(s) * 0.5f, y, col);
    }

    static const float kLabelScale = 0.5f;
    static void DrawTextL(const char* s, float x, float y, ARGB col) {
        if (!s || !s_font) return;

        float c[4];

        Vec4Of(col, c);
        pText(s, 0x7FFFFFFF, s_font, x, y, kLabelScale, kLabelScale, 0.0f, c, 4);
    }

    static void FramedRect(float x, float y, float w, float h, float t, ARGB fill, ARGB border) {
        DrawRect(x, y, t, h, border);
        DrawRect(x + t, y, w - t, t, border);
        DrawRect(x + t, y + h - t, w - t * 2.0f, t, border);
        DrawRect(x + w - t, y + t, t, h - t, border);
        DrawRect(x + t, y + t, w - t * 2.0f, h - t * 2.0f, fill);
    }

    void BoxedLabel(float cx, float baselineY, const char* s, ARGB border) {
        if (!s || !s[0]) return;

        float sw = TextWSc(s, kLabelScale);
        float sh = (float)FontH() * kLabelScale;
        float x = cx - 2.0f - sw * 0.5f;
        float y = baselineY - sh;

        FramedRect(x, y, sw + 4.0f, sh, 1.0f, Pack(CFp(C_BOX), 0xC0000000), border);
        DrawTextL(s, cx - sw * 0.5f, baselineY, Pack(CFp(C_LABEL_TEXT), 0xFFFFFFFF));
    }
    struct Box;

    void Crosshair(void* cg) {
        int mode = CB(V_CROSSHAIR);
        if (mode == 0 || CB(0x90B4328D) != 0) return;

        char* rd = (char*)cg + CG_REFDEF;
        float W = (float)RI(rd, RD_W);
        float H = (float)RI(rd, RD_H);
        float cx = W * 0.5f;
        float cy = H * 0.5f;

        DrawRect(cx - 10.0f, cy - 1.0f, 19.0f, 1.0f, s_accent);
        DrawRect(cx - 1.0f, cy - 10.0f, 1.0f, 19.0f, s_accent);
    }

    void LocalHealthBar(void* cg) {
        static float smooth = 0.0f;
        int cur = RI(cg, 0x482E8);
        int max = RI(cg, 0x482E0);
        if (cur <= 0 || max <= 0) return;

        int clipped = cur < max ? cur : max;
        int divisor = cur == 0 ? 1 : cur;
        float target = ((float)clipped / (float)divisor) * 300.0f + 2.0f;

        smooth += (target - smooth) * 0.2f;

        char* rd = (char*)cg + CG_REFDEF;
        float W = (float)RI(rd, RD_W);
        float H = (float)RI(rd, RD_H);
        float x = W * 0.5f - 150.0f;
        float y = H - 25.0f;
        ARGB box = Pack(CFp(C_BOX), 0xC0000000);

        FramedRect(x, y, 300.0f, 16.0f, 1.0f, box, box);
        FramedRect(x, y, smooth, 16.0f, 1.0f, s_accent, box);
    }

    void DisableNativeCrosshair(int on) {
        u32* p = (u32*)0x821C7C18ull;

        if (s_crossState < 0) {
            s_crossOwn = (*p == 0x4BFFF211u);
            s_crossState = 0;
        }

        if (!s_crossOwn) return;
        int want = on ? 1 : 0;
        if (s_crossState == want) return;
        DWORD old;
        if (!VirtualProtect(p, 4, PAGE_EXECUTE_READWRITE, &old)) return;
        *p = want ? 0x38600000u : 0x4BFFF211u;
        __dcbst(0, p);
        __sync();
        __emit(0x4C00012C);
        VirtualProtect(p, 4, old, &old);
        s_crossState = want;
    }
}
