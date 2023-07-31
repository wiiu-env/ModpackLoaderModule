#include "ModpackInformation.h"
#include "globals.h"
#include "utils/StringTools.h"
#include "utils/logger.h"
#include "version.h"
#include <content_redirection/redirection.h>
#include <coreinit/title.h>
#include <romfs_dev.h>
#include <rpxloader/rpxloader.h>
#include <wuhb_utils/utils.h>
#include <wums.h>

WUMS_MODULE_EXPORT_NAME("homebrew_modpack_loader");
WUMS_USE_WUT_DEVOPTAB();
WUMS_DEPENDS_ON(homebrew_content_redirection);
WUMS_DEPENDS_ON(homebrew_rpx_loader);
WUMS_DEPENDS_ON(homebrew_wuhb_utils);

#define MODULE_VERSION "v0.1.0"

WUMS_INITIALIZE() {
    ContentRedirectionStatus error;
    if ((error = ContentRedirection_InitLibrary()) != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init ContentRedirection. Error %s %d", ContentRedirection_GetStatusStr(error), error);
        OSFatal("homebrew_modpack_loader: Failed to init ContentRedirection.");
    }
    // But then use libcontentredirection instead.
    RPXLoaderStatus error1;
    if ((error1 = RPXLoader_InitLibrary()) != RPX_LOADER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init RPXLoader. Error %s %d", RPXLoader_GetStatusStr(error1), error1);
        OSFatal("homebrew_modpack_loader: Failed to init RPXLoader.");
    }

    WUHBUtilsStatus error2;
    if ((error2 = WUHBUtils_InitLibrary()) != WUHB_UTILS_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init WUHBUtils. Error %d", error2);
        OSFatal("homebrew_modpack_loader: Failed to init WUHBUtils.");
    }
}


#define MODPACK_ROMFS_NAME "modpackrom"
#define MODPACK_ROMFS_PATH MODPACK_ROMFS_NAME ":"

bool CheckModpackShouldBeApplied(const std::unique_ptr<ModpackInformation> &modpack) {
    return OSGetTitleID() == modpack->mTitleId;
}

bool ApplyContentRedirection(const std::unique_ptr<ModpackInformation> &modpack) {
    if (gRedirectionHandle != 0) {
        DEBUG_FUNCTION_LINE_ERR("Content redirection is already active");
        return false;
    }
    std::string completePath = std::string("/vol/external01/") + modpack->mWUHBPath;
    if (romfsMount(MODPACK_ROMFS_NAME, completePath.c_str(), RomfsSource_FileDescriptor_CafeOS) == 0) {
        auto device = GetDeviceOpTab(MODPACK_ROMFS_PATH);
        if (device == nullptr || strcmp(device->name, MODPACK_ROMFS_NAME) != 0) {
            romfsUnmount(MODPACK_ROMFS_NAME);
            DEBUG_FUNCTION_LINE_ERR("DeviceOpTab for %s not found.", MODPACK_ROMFS_PATH);
            return false;
        }
        int outRes = -1;
        if (ContentRedirection_AddDevice(device, &outRes) != CONTENT_REDIRECTION_RESULT_SUCCESS || outRes < 0) {
            DEBUG_FUNCTION_LINE_ERR("Failed to AddDevice to ContentRedirection");
            romfsUnmount(MODPACK_ROMFS_NAME);
            return false;
        }
        std::string path = MODPACK_ROMFS_PATH + modpack->mPath;
        DEBUG_FUNCTION_LINE_ERR("Added \"%s\"", path.c_str());
        auto res = ContentRedirection_AddFSLayer(&gRedirectionHandle,
                                                 "Modpack",
                                                 path.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE);
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            return true;
        } else {
            outRes = -1;
            if (ContentRedirection_RemoveDevice(MODPACK_ROMFS_PATH, &outRes) != CONTENT_REDIRECTION_RESULT_SUCCESS || res < 0) {
                DEBUG_FUNCTION_LINE_ERR("Failed to remove device");
            }
            romfsUnmount(MODPACK_ROMFS_PATH);
            DEBUG_FUNCTION_LINE_ERR("ContentRedirection_AddFSLayer had failed for content");
        }
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to mount %s", modpack->mWUHBPath.c_str());
    }
    return false;
}
WUMS_APPLICATION_STARTS() {
    OSReport("Running ModpackLoaderModule " MODULE_VERSION MODULE_VERSION_EXTRA "\n");
    initLogging();

    gParsedModpacks.clear();

    if (gLaunchedModpack != nullptr) {
        if (CheckModpackShouldBeApplied(gLaunchedModpack)) {
            ApplyContentRedirection(gLaunchedModpack);
        } else {
            DEBUG_FUNCTION_LINE_ERR("Modpack was selected but can't be applied.");
        }
        gLaunchedModpack = nullptr;
    }
}

WUMS_APPLICATION_ENDS() {
    deinitLogging();
}
