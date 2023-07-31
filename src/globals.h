#include "ModpackInformation.h"
#include <content_redirection/redirection.h>
#include <memory>
#include <vector>

extern std::unique_ptr<ModpackInformation> gLaunchedModpack;
extern std::forward_list<std::unique_ptr<ModpackInformation>> gParsedModpacks;
extern CRLayerHandle gRedirectionHandle;