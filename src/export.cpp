#include "ModpackInformation.h"
#include "globals.h"
#include "utils/MenuUtils.h"
#include "utils/ini.h"
#include "utils/logger.h"
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <forward_list>
#include <memory>
#include <modpack_loader/loader.h>
#include <nn/acp/title.h>
#include <rpxloader/rpxloader.h>
#include <string>
#include <sysapp/launch.h>
#include <sysapp/title.h>
#include <wuhb_utils/utils.h>
#include <wums/exports.h>

std::mutex sParsedModpacksMutex;


static ModpackLoaderStatus CheckModpack(std::unique_ptr<ModpackInformation> &modpack, bool *readyToLaunch) {
    if (!readyToLaunch) {
        return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
    }

    if (SYSCheckTitleExists(modpack->mTitleId)) {
        *readyToLaunch = true;
        return MODPACK_LOADER_RESULT_SUCCESS;
    }

    uint64_t titleIdOfDisc = 0;
    bool discInserted;

    uint32_t attempt = 0;
    while (!GetTitleIdOfDisc(&titleIdOfDisc, &discInserted)) {
        if (++attempt > 20) {
            break;
        }
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    if (discInserted && titleIdOfDisc == modpack->mTitleId) {
        *readyToLaunch = true;
        return MODPACK_LOADER_RESULT_SUCCESS;
    }

    *readyToLaunch = false;

    return MODPACK_LOADER_RESULT_SUCCESS;
}

ModpackLoaderStatus MPLCheckIfLaunchable(MPLModpackHandle handle, bool *readyToLaunchOut) {
    std::lock_guard<std::mutex> lock(sParsedModpacksMutex);

    for (auto &modpack : gParsedModpacks) {
        if ((uint32_t) modpack.get() == (uint32_t) handle) {
            return CheckModpack(modpack, readyToLaunchOut);
        }
    }

    return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
}

static ModpackLoaderStatus LaunchModpack(std::unique_ptr<ModpackInformation> &modpack, bool *launchedOut) {
    if (!launchedOut) {
        return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
    }

    uint64_t titleIdToLaunch = modpack->mTitleId;

    *launchedOut = false;

    if (!SYSCheckTitleExists(titleIdToLaunch)) {
        // Displaying the menu is broken.
        if (!handleDiscInsertScreen(titleIdToLaunch)) {
            // Aborted or wrong disc inserted.
            return MODPACK_LOADER_RESULT_SUCCESS;
        }
    }

    MCPTitleListType titleInfo;
    int32_t handle = MCP_Open();
    auto err       = MCP_GetTitleInfo(handle, titleIdToLaunch, &titleInfo);
    MCP_Close(handle);

    if (err == 0) {
        ACPAssignTitlePatch(&titleInfo);
        _SYSLaunchTitleWithStdArgsInNoSplash(titleIdToLaunch, nullptr);
        *launchedOut = true;
    }

    return MODPACK_LOADER_RESULT_SUCCESS;
}

ModpackLoaderStatus MPLLaunchModpack(MPLModpackHandle handle, bool *launchedOut) {
    DEBUG_FUNCTION_LINE_ERR("Launch modpack!");
    bool launchSuccess = false;
    bool launched      = false;
    {
        std::lock_guard<std::mutex> lock(sParsedModpacksMutex);

        for (auto &modpack : gParsedModpacks) {
            if ((uint32_t) modpack.get() == (uint32_t) handle) {
                auto res = LaunchModpack(modpack, &launched);
                if (res == MODPACK_LOADER_RESULT_SUCCESS && launched) {
                    launchSuccess    = true;
                    gLaunchedModpack = make_unique_nothrow<ModpackInformation>(modpack->mTitleId, modpack->mPath, modpack->mWUHBPath);
                    if (gLaunchedModpack == nullptr) {
                        OSFatal("make_unique_nothrow returned NULL");
                    }
                    *launchedOut = true;
                }
                break;
            }
        }
    }

    if (launchSuccess) {
        if (remove_locked_first_if(sParsedModpacksMutex, gParsedModpacks, [handle](auto &cur) { return cur->getHandle() == handle; })) {
            return MODPACK_LOADER_RESULT_SUCCESS;
        }
    }

    return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
}


static int parseINIhandler(void *user, const char *section, const char *name, const char *value) {
    auto *fInfo = (ModpackInformationIni *) user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("meta", "version")) {
        fInfo->version = strtol(value, nullptr, 10);
    } else if (MATCH("modpack", "name")) {
        fInfo->name = value;
    } else if (MATCH("modpack", "titleids")) {
        fInfo->titleids = value;
    } else if (MATCH("modpack", "path")) {
        fInfo->path = value;
    } else if (MATCH("modpack", "updatepath")) {
        fInfo->updatepath = value;
    } else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

ModpackLoaderStatus MPLParseModpackByPath(const char *bundle_path, MPLModpackHandle *outHandle) {
    if (!bundle_path || !outHandle) {
        return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
    }

    ModpackLoaderStatus result      = MODPACK_LOADER_RESULT_SUCCESS;
    std::string completePath        = std::string("/vol/external01/") + bundle_path;
    auto res                        = -1;
    ModpackInformationIni parsedIni = {};
    if (WUHBUtils_MountBundle("mpl", completePath.c_str(), BundleSource_FileDescriptor_CafeOS, &res) == WUHB_UTILS_RESULT_SUCCESS && res == 0) {
        uint8_t *buffer;
        uint32_t bufferSize;
        if (WUHBUtils_ReadWholeFile("mpl:/content/modpack.ini", &buffer, &bufferSize) == WUHB_UTILS_RESULT_SUCCESS) {
            // The string must be null-terminated...
            auto bufferWrapper = (char *) malloc(bufferSize + 1);
            if (bufferWrapper == nullptr) {
                result = MODPACK_LOADER_RESULT_NO_MEMORY;
            } else {
                memcpy(bufferWrapper, buffer, bufferSize);
                bufferWrapper[bufferSize] = 0;
                if (ini_parse_string((const char *) bufferWrapper, parseINIhandler, &parsedIni) < 0) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to load and parse \"content/modpack.ini\"");
                    result = MODPACK_LOADER_RESULT_UNKNOWN_ERROR;
                }
                free(bufferWrapper);
            }
            free(buffer);
        } else {
            result = MODPACK_LOADER_RESULT_UNKNOWN_ERROR;
            DEBUG_FUNCTION_LINE_ERR("Failed to read \"content/modpack.ini\"");
        }

        auto outRes = 0;
        if (WUHBUtils_UnmountBundle("mpl", &outRes) != WUHB_UTILS_RESULT_SUCCESS || outRes != WUHB_UTILS_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to unmount bundle");
        }
    } else {
        result = MODPACK_LOADER_RESULT_UNKNOWN_ERROR;
    }
    if (result != MODPACK_LOADER_RESULT_SUCCESS) {
        return result;
    }

    auto modpackInfoOpt = ModpackInformation::Parse(parsedIni, bundle_path);
    if (!modpackInfoOpt) {
        return MODPACK_LOADER_RESULT_NO_MEMORY;
    }

    *outHandle = modpackInfoOpt.value()->getHandle();
    {
        std::lock_guard<std::mutex> lock(sParsedModpacksMutex);
        gParsedModpacks.push_front(std::move(modpackInfoOpt.value()));
    }

    return result;
}

ModpackLoaderStatus MPLParseModpackFromCurrentlyRunningWUHB(MPLModpackHandle *outHandle) {
    if (!outHandle) {
        return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
    }
    if (RPXLoader_UnmountCurrentRunningBundle() != RPX_LOADER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to unmount current running bundle");
        return MODPACK_LOADER_RESULT_UNKNOWN_ERROR;
    }
    char buffer[256] = {};
    if (RPXLoader_GetPathOfRunningExecutable(buffer, sizeof(buffer)) != RPX_LOADER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to unmount current running bundle");
        return MODPACK_LOADER_RESULT_UNKNOWN_ERROR;
    }

    return MPLParseModpackByPath(buffer, outHandle);
}

ModpackLoaderStatus MPLGetVersion(ModpackLoaderVersion *outVersion) {
    if (outVersion == nullptr) {
        return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
    }
    *outVersion = 1;
    return MODPACK_LOADER_RESULT_SUCCESS;
}

WUMS_EXPORT_FUNCTION(MPLGetVersion);
WUMS_EXPORT_FUNCTION(MPLParseModpackByPath);
WUMS_EXPORT_FUNCTION(MPLParseModpackFromCurrentlyRunningWUHB);
WUMS_EXPORT_FUNCTION(MPLCheckIfLaunchable);
WUMS_EXPORT_FUNCTION(MPLLaunchModpack);
