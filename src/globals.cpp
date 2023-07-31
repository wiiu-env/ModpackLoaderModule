#include "globals.h"

std::unique_ptr<ModpackInformation> gLaunchedModpack = {};
std::forward_list<std::unique_ptr<ModpackInformation>> gParsedModpacks;
CRLayerHandle gRedirectionHandle = 0;