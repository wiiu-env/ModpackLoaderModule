#include <modpack_loader/loader.h>
#include <wums/exports.h>

ModpackLoaderStatus MPLGetVersion(ModpackLoaderVersion *outVersion) {
    if (outVersion == nullptr) {
        return MODPACK_LOADER_RESULT_INVALID_ARGUMENT;
    }
    *outVersion = 1;
    return MODPACK_LOADER_RESULT_SUCCESS;
}

WUMS_EXPORT_FUNCTION(MPLGetVersion);