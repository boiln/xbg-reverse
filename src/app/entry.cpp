#include "luda/core/arena.hpp"
#include "luda/ui/menu.hpp"

#include <xtl.h>

namespace menu {
    void Engine_Bootstrap(void);
    void Engine_Teardown(void);
}  // namespace menu

namespace reconrender {
    void Start(void);
    void Stop(void);
}  // namespace reconrender

extern "C" {
volatile unsigned long XbgBootFaulted = 0;
volatile unsigned long XbgBooted = 0;
}

extern "C" __declspec(dllexport) unsigned long XbgShutdown(void) {
    __try {
        reconrender::Stop();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    __try {
        xbg::ArenaFree();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return 0;
}

BOOL APIENTRY DllMain(HANDLE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        xbg::ArenaInit();
        __try {
            reconrender::Start();

            XbgBooted = 1;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            XbgBootFaulted = 1;
        }
    } else if (reason == DLL_PROCESS_DETACH) {
        __try {
            reconrender::Stop();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
        __try {
            xbg::ArenaFree();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    return TRUE;
}
