#include "utils/StringTools.h"
#include "utils/logger.h"
#include "version.h"
#include <wums.h>

WUMS_MODULE_EXPORT_NAME("homebrew_modpack_loader");
WUMS_USE_WUT_DEVOPTAB();
WUMS_DEPENDS_ON(homebrew_content_redirection);

#define MODULE_VERSION "v0.1.0"


WUMS_INITIALIZE() {
}

WUMS_APPLICATION_STARTS() {
    OSReport("Running ModpackLoaderModule " MODULE_VERSION MODULE_VERSION_EXTRA "\n");
    initLogging();
}

WUMS_APPLICATION_ENDS() {
    deinitLogging();
}
