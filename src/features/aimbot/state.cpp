#include "internal.hpp"

namespace aimbot {
    volatile unsigned char s_espHittable[18];
    volatile unsigned char s_espVisible[18];

    bool EspHittable(int client) { return client >= 0 && client < 18 && s_espHittable[client] != 0; }
    bool EspVisible(int client) { return client >= 0 && client < 18 && s_espVisible[client] != 0; }
}
