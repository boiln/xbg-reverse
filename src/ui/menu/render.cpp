#include "internal.hpp"

namespace reconrender {
    int TopTabIndex(int p) {
        for (int i = 0; i < 5; ++i)
            if (kTopTabs[i] == p) return i;

        return -1;
    }

    int ParentPage(int p) {
        switch (p) {
            case PAGE_RECOVERY:
            case PAGE_GAMERTAG:
            case PAGE_THIRDP:
                return PAGE_MAIN;
            case PAGE_ANTIAIM:
            case PAGE_ADVANCED:
                return PAGE_AIMBOT;
            case PAGE_COLOURS:
                return PAGE_SETTINGS;
            case PAGE_PRESETGT:
            case PAGE_CUSTOMGT:
                return PAGE_GAMERTAG;
            case PAGE_PLAYERACT:
                return PAGE_PLAYER;
            case PAGE_HOST:
                return PAGE_PLAYERACT;
            case PAGE_ESPCOL:
            case PAGE_ED_MENU:
            case PAGE_ED_SCROLL:
            case PAGE_ED_SMOKE:
            case PAGE_ED_UI:
            case PAGE_ED_TEXT:
                return PAGE_COLOURS;
            case PAGE_ED_ENEMY:
            case PAGE_ED_HITTABLE:
            case PAGE_ED_VISIBLE:
            case PAGE_ED_PRIO:
            case PAGE_ED_WL:
            case PAGE_ED_FRIENDLY:
                return PAGE_ESPCOL;
            case PAGE_UIELEM:
                return PAGE_ED_UI;
            case PAGE_PROFILER:
                return PAGE_SETTINGS;
            default:
                return PAGE_MAIN;
        }
    }

    void ItoA(int v, char* b) {
        char t[16];
        int i = 0;
        bool neg = v < 0;
        unsigned u = neg ? (unsigned)(-v) : (unsigned)v;
        if (!u) t[i++] = '0';

        while (u) {
            t[i++] = (char)('0' + u % 10);
            u /= 10;
        }
        int j = 0;
        if (neg) b[j++] = '-';
        while (i) b[j++] = t[--i];
        b[j] = 0;
    }

    void UtoA(u32 v, char* b) {
        char t[16];
        int i = 0;

        do {
            t[i++] = (char)('0' + v % 10u);
            v /= 10u;
        } while (v && i < 15);
        int j = 0;
        while (i) b[j++] = t[--i];
        b[j] = 0;
    }

    void U64ToHex(u64 v, char* b) {
        static const char kHex[] = "0123456789abcdef";
        char t[17];
        int i = 0;

        do {
            t[i++] = kHex[(u32)(v & 0xFu)];
            v >>= 4;
        } while (v && i < 16);
        int j = 0;
        while (i) b[j++] = t[--i];
        b[j] = 0;
    }

    void Adjust(Option& o, int dir) {
        if (o.kind == K_SLIDER || o.kind == K_SLIDER_TOGGLE) {
            if (!o.backFloat) return;
            float lo = o.lo, hi = o.hi;
            float current = *o.backFloat;
            union {
                float f;
                u32 u;
            } bits;
            bits.f = current;
            if ((bits.u & 0x7F800000u) == 0x7F800000u) current = lo;
            if (current < lo) current = lo;
            if (current > hi) current = hi;
            float v = current + dir * o.step;
            if (v < lo) v = lo;
            if (v > hi) v = hi;
            *o.backFloat = v;
            return;
        }

        if ((o.kind == K_ENUM || o.kind == K_ENUM_TOGGLE) && o.backByte) {
            int v = (int)*o.backByte + dir;
            if (v < 0) v = o.names_n - 1;
            if (v >= o.names_n) v = 0;
            *o.backByte = (u8)v;
        }
    }

    static void FmtSlider(const Option& o, char* buf) {
        if (!buf) return;

        if (!o.backFloat) {
            buf[0] = 0;
            return;
        }
        float v = *o.backFloat;
        union {
            float f;
            u32 u;
        } bits;
        bits.f = v;
        if ((bits.u & 0x7F800000u) == 0x7F800000u) v = o.lo;
        if (v < o.lo) v = o.lo;
        if (v > o.hi) v = o.hi;
        if (o.fmt == FMT_255) v = v * 255.0f;

        bool neg = v < 0;
        if (neg) v = -v;

        int decimals = o.fmt == FMT_2DEC ? 2 : (o.fmt == FMT_1DEC ? 1 : 0);
        int scale = decimals == 2 ? 100 : (decimals == 1 ? 10 : 1);
        int whole = (int)v;
        int frac = (int)((v - whole) * (float)scale + 0.5f);

        if (frac >= scale) {
            whole++;
            frac = 0;
        }
        char num[24];
        char* p = num;

        if (neg) *p++ = '-';

        char tmp[16];
        ItoA(whole, tmp);

        int i = 0;
        while (tmp[i]) *p++ = tmp[i++];

        if (decimals) {
            *p++ = '.';
            if (decimals == 2) *p++ = (char)('0' + (frac / 10) % 10);
            *p++ = (char)('0' + frac % 10);
        }
        *p = 0;

        char* b = buf;

        if (o.fmt == FMT_255) {
            *b++ = '[';
        }
        for (i = 0; num[i]; ++i) *b++ = num[i];

        if (o.fmt == FMT_MS) {
            const char* ms = " (ms)";
            for (i = 0; ms[i]; ++i) *b++ = ms[i];
        }

        if (o.fmt == FMT_255) {
            *b++ = ']';
        }
        *b = 0;
    }
    static const char* EnumName(const Option& o) {
        int v = o.backByte ? (int)*o.backByte : 0;
        if (v < 0) v = 0;
        if (v >= o.names_n) v = o.names_n - 1;

        return (o.names_n > 0) ? o.names[v] : "";
    }

    static void FmtEnum(const Option& o, char* buf) {
        const char* n = EnumName(o);
        char* b = buf;
        *b++ = '<';
        *b++ = ' ';
        for (int i = 0; n[i]; ++i) *b++ = n[i];
        *b++ = ' ';
        *b++ = '>';
        *b = 0;
    }

    void RenderMenu() {
        BuildPage(g_curPage);

        float avail = L.scrH - L.menuY - L.titleH - L.tabH - L.barH - 12.0f;

        if (g_optCount > 0 && L.rowH * g_optCount > avail) {
            float c = avail / (float)g_optCount;
            float m = L.emPx + 2.0f;
            L.rowH = c > m ? c : m;
        }

        float x = L.menuX;
        float y = L.menuY;
        float bodyH = g_optCount * L.rowH;
        float totalH = L.titleH + bodyH + L.tabH + L.barH;

        DrawRect(x, y, L.menuW, totalH, kColBg);
        DrawRect(x, y, L.menuW, L.titleH, kColBar);
        DrawTextS(kTitle, x + (L.menuW - TextW(kTitle)) * 0.5f, TextY(y, L.titleH), kColText);
        DrawBorder(x, y, L.menuW, totalH, L.border, kColBorder);

        int sel = g_sel[g_curPage < PAGE_N ? g_curPage : 0];
        float bodyTop = y + L.titleH;
        float rx = x + L.menuW - L.pad;
        char buf[80];

        for (int i = 0; i < g_optCount; ++i) {
            const Option& o = g_options[i];
            float ry = bodyTop + i * L.rowH;
            float ty = TextY(ry, L.rowH);

            DrawTextS(o.label, x + L.pad, ty, i == sel ? kColAccent : kColText);
            switch (o.kind) {
                case K_TOGGLE: {
                    bool on = o.backByte && *o.backByte;
                    DrawRect(rx - L.box, ry + (L.rowH - L.box) * 0.5f, L.box, L.box, on ? kColAccent : kColOff);
                    break;
                }
                case K_SLIDER: {
                    float val = *o.backFloat;
                    float frac = (o.hi > o.lo) ? (val - o.lo) / (o.hi - o.lo) : 0.0f;
                    if (frac < 0.0f) frac = 0.0f;
                    if (frac > 1.0f) frac = 1.0f;
                    float bw = 80.0f, bh = 12.0f;
                    float bx = rx - bw, by = ry + (L.rowH - bh) * 0.5f;
                    DrawBorder(bx, by, bw, bh, 1.0f, kColAccent);
                    DrawRect(bx + 1.0f, by + 1.0f, (bw - 2.0f) * frac, bh - 2.0f, kColAccent);
                    FmtSlider(o, buf);
                    DrawRight(buf, bx - 8.0f, ty, kColText);
                    break;
                }
                case K_SLIDER_TOGGLE: {
                    float val = *o.backFloat;
                    float frac = (o.hi > o.lo) ? (val - o.lo) / (o.hi - o.lo) : 0.0f;
                    if (frac < 0.0f) frac = 0.0f;
                    if (frac > 1.0f) frac = 1.0f;
                    float bw = 80.0f, bh = 12.0f;
                    float bx = rx - bw, by = ry + (L.rowH - bh) * 0.5f;
                    DrawBorder(bx, by, bw, bh, 1.0f, kColAccent);
                    DrawRect(bx + 1.0f, by + 1.0f, (bw - 2.0f) * frac, bh - 2.0f, kColAccent);
                    FmtSlider(o, buf);
                    DrawRight(buf, bx - 8.0f, ty, kColText);
                    float tx = bx - TextW(buf) - 18.0f - L.box;
                    DrawRect(tx, ry + (L.rowH - L.box) * 0.5f, L.box, L.box,
                             o.auxByte && *o.auxByte ? kColAccent : kColOff);
                    break;
                }
                case K_ENUM:
                    FmtEnum(o, buf);
                    DrawRight(buf, rx, ty, kColText);
                    break;
                case K_ENUM_TOGGLE: {
                    FmtEnum(o, buf);
                    DrawRight(buf, rx, ty, kColText);
                    bool on = o.auxByte && *o.auxByte;
                    float bx = rx - TextW(buf) - 10.0f - L.box;
                    DrawRect(bx, ry + (L.rowH - L.box) * 0.5f, L.box, L.box, on ? kColAccent : kColOff);
                    break;
                }
                case K_COLOR:
                    if (o.backFloat) {
                        ARGB c = PackRGBA(o.backFloat, 0xFFFFFFFF);
                        DrawRight("[Preview]", rx - L.box * 1.4f - 8.0f, ty, kColText);
                        DrawRect(rx - L.box * 1.4f, ry + (L.rowH - L.box) * 0.5f, L.box * 1.4f, L.box, c);
                    }
                    break;
                case K_ACTION:
                    DrawRight("->>", rx, ty, kColText);
                    break;
                default:
                    break;
            }
        }

        if (*Bp(0x90B43395u) && g_optCount > 0) {
            float trackX = x + L.menuW - 4.0f;
            float thumbY = bodyTop + ((float)sel + 0.5f) * bodyH / (float)g_optCount - 3.0f;
            DrawRect(trackX, bodyTop, 2.0f, bodyH, 0x60303030u);
            DrawRect(trackX - 1.0f, thumbY, 4.0f, 6.0f, PackRGBA(Fp(0x90B43954u), kColAccent));
        }

        int active = g_curPage;
        if (TopTabIndex(active) < 0) active = ParentPage(active);
        if (TopTabIndex(active) < 0) active = ParentPage(active);

        float tabTop = bodyTop + bodyH, gap = 12.0f, side = L.pad, usableW = L.menuW - side * 2.0f, sumW = 0;
        for (int i = 0; i < 5; ++i) sumW += TextW(kTabLabels[i]);
        float ts = L.scale;

        if (sumW > 0 && sumW + gap * 4 > usableW) {
            ts = L.scale * (usableW - gap * 4) / sumW;
            sumW = 0;
            for (int i = 0; i < 5; ++i) sumW += TextWSc(kTabLabels[i], ts);
        }
        float tx = x + side + (usableW - (sumW + gap * 4)) * 0.5f, tty = TextY(tabTop, L.tabH);
        for (int i = 0; i < 5; ++i) {
            DrawTextSc(kTabLabels[i], tx, tty, ts, kTopTabs[i] == active ? kColAccent : kColText);
            tx += TextWSc(kTabLabels[i], ts) + gap;
        }

        DrawRect(x, tabTop + L.tabH, L.menuW, L.barH, kColAccent);
    }

    static const char* GLYPH_OPEN_HINT = "^BXENONButtontrigL^ + ^BXENONButtondpadL^ To Open/Close luda v1.0.0";

    static u32 s_fpsCount = 0, s_fpsLast = 0, s_fps = 0;
    void TickFps() {
        u32 now = GetTickCount();
        s_fpsCount++;

        if (!s_fpsLast) {
            s_fpsLast = now;
            return;
        }
        u32 elapsed = now - s_fpsLast;
        if (elapsed < 500) return;
        s_fps = s_fpsCount * 1000u / elapsed;
        s_fpsCount = 0;
        s_fpsLast = now;
    }

    static u32 s_worldFrameCg = 0;
    static u32 s_worldFrameSequence = 0;
    bool BeginWorldFrame() {
        __try {
            u32 cg = *(volatile u32*)(u64)A_CG_POINTER;
            if (!cg) {
                s_worldFrameCg = 0;
                s_worldFrameSequence = 0;
                return true;
            }
            u32 sequence = *(volatile u32*)(u64)(cg + 0x480A8u);
            if (cg == s_worldFrameCg && sequence == s_worldFrameSequence) return false;
            s_worldFrameCg = cg;
            s_worldFrameSequence = sequence;
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            s_worldFrameCg = 0;
            s_worldFrameSequence = 0;

            return true;
        }
    }
    static void BuildKV(char* buffer, const char* prefix, int value) {
        int i = 0;

        while (prefix[i]) {
            buffer[i] = prefix[i];
            ++i;
        }
        ItoA(value, buffer + i);
    }

    void WatermarkHUD() {
        if (*Bp(0x90B433A8)) {
            float hpad = 8.0f, hw = HudW(GLYPH_OPEN_HINT) + hpad * 2, hh = L.rowH;
            float hx = 8.0f + *Fp(0x90B43A78u);
            float hy = L.scrH - 8.0f - *Fp(0x90B43A7Cu) - hh;
            DrawRect(hx, hy, hw, hh, kColBar);
            DrawBorder(hx, hy, hw, hh, L.border, kColBorder);
            DrawTextS(GLYPH_OPEN_HINT, hx + hpad, TextY(hy, hh), kColText);
        }

        char fpsBuffer[24], pingBuffer[24];
        BuildKV(fpsBuffer, "FPS: ", (int)s_fps);
        BuildKV(pingBuffer, "Ping: ", esp::LocalPing());
        const char* lines[3] = {kTitle, fpsBuffer, pingBuffer};
        float height = L.rowH, gap = 4.0f, padding = 8.0f;

        float right = L.scrW - 8.0f - *Fp(0x90B43A78u);
        float top = 8.0f + *Fp(0x90B43A7Cu);
        for (int i = 0; i < 3; ++i) {
            float width = HudW(lines[i]) + padding * 2.0f;
            float x = right - width;
            float y = top + i * (height + gap);
            DrawRect(x, y, width, height, kColBar);
            DrawBorder(x, y, width, height, L.border, kColBorder);
            DrawTextS(lines[i], x + padding, TextY(y, height), kColText);
        }
    }

    void DrawLegend() {
        const char* legend;

        if (TopTabIndex(g_curPage) < 0)
            legend = "^BXENONButtonA^ Select   ^BXENONButtonB^ Back";
        else if (g_curPage == PAGE_MAIN)
            legend = "^BXENONButtonA^ Select";
        else
            legend = "^BXENONButtonA^ Select   ^BXENONButtonB^ Main Menu";
        float lw = VisTextW(legend);
        float lx = (L.scrW - lw) * 0.5f, ly = L.scrH - 52.0f;
        DrawTextS(legend, lx, TextY(ly, L.rowH), kColText);
    }

    void SetRGBA(u32 va, float r, float g, float b, float a) {
        float* f = Fp(va);
        f[0] = r;
        f[1] = g;
        f[2] = b;
        f[3] = a;
    }
}
