#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <nn/act.h>
#include <string>
#include <vector>

#define COLOR_WHITE              Color(0xffffffff)
#define COLOR_BLACK              Color(0, 0, 0, 255)
#define COLOR_BACKGROUND         COLOR_BLACK
#define COLOR_BACKGROUND_WARN    Color(255, 40, 0, 255)
#define COLOR_TEXT               COLOR_WHITE
#define COLOR_TEXT2              Color(0xB3ffffff)
#define COLOR_AUTOBOOT           Color(0xaeea00ff)
#define COLOR_BORDER             Color(204, 204, 204, 255)
#define COLOR_BORDER_HIGHLIGHTED Color(0x3478e4ff)

bool handleDiscInsertScreen(uint64_t expectedTitleId);