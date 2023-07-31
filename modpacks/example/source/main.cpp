#include <coreinit/debug.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <modpack_loader/loader.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

int main(int argc, char **argv) {
    WHBLogCafeInit();
    WHBLogUdpInit();

    WHBLogPrintf("Hello from example modpack");

    ModpackLoaderStatus error;
    if ((error = ModpackLoader_InitLibrary()) != MODPACK_LOADER_RESULT_SUCCESS) {
        WHBLogPrintf("Failed to init ModpackLoader. Error %s %d",
                     ModpackLoader_GetStatusStr(error), error);
        OSFatal("Failed to init ModpackLoader.");
    }

    WHBProcInit();

    while (WHBProcIsRunning()) {
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    WHBProcShutdown();
    return 0;
}
