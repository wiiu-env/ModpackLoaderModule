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

    // TODO: Any form of error handling...

    WHBLogPrintf("Hello from example modpack");

    ModpackLoaderStatus error;
    if ((error = ModpackLoader_InitLibrary()) != MODPACK_LOADER_RESULT_SUCCESS) {
        WHBLogPrintf("Failed to init ModpackLoader. Error %s %d", ModpackLoader_GetStatusStr(error), error);
        OSFatal("Failed to init ModpackLoader.");
    }

    MPLModpackHandle modpack;
    if ((error = ModpackLoader_ParseModpackFromCurrentlyRunningWUHB(&modpack)) != MODPACK_LOADER_RESULT_SUCCESS) {
        WHBLogPrintf("ModpackLoader_ParseModpackFromCurrentlyRunningWUHB failed. Error %s %d", ModpackLoader_GetStatusStr(error), error);
        OSFatal("Failed to parse current running wuhb");
    }

    bool launchable = false;
    if ((error = ModpackLoader_CheckIfLaunchable(modpack, &launchable)) != MODPACK_LOADER_RESULT_SUCCESS || !launchable) {
        WHBLogPrintf("ModpackLoader_CheckIfLaunchable failed. Error %s %d", ModpackLoader_GetStatusStr(error), error);
        OSFatal("Modpack not launchable");
    }

    bool launched = false;
    if ((error = ModpackLoader_LaunchModpack(modpack, &launched)) != MODPACK_LOADER_RESULT_SUCCESS || !launched) {
        WHBLogPrintf("ModpackLoader_LaunchModpack failed. Error %s %d", ModpackLoader_GetStatusStr(error), error);
        OSFatal("Failed to launch modpack");
    }

    WHBProcInit();

    while (WHBProcIsRunning()) {
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    WHBProcShutdown();
    return 0;
}
