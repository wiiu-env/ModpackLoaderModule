#include "MenuUtils.h"
#include "utils/DrawUtils.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <coreinit/debug.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <cstring>
#include <malloc.h>
#include <sysapp/title.h>
#include <vpad/input.h>

bool handleDiscInsertScreen(uint64_t expectedTitleId) {
    if (SYSCheckTitleExists(expectedTitleId)) {
        return true;
    }

    auto result       = false;
    auto screenBuffer = DrawUtils::InitOSScreen();
    if (!screenBuffer) {
        OSFatal("Failed to alloc memory for screen");
    }

    uint32_t tvBufferSize  = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

    DrawUtils::initBuffers(screenBuffer, tvBufferSize, (void *) ((uint32_t) screenBuffer + tvBufferSize), drcBufferSize);
    if (!DrawUtils::initFont()) {
        OSFatal("Failed to init font");
    }
    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);
    DrawUtils::endDraw();

    uint64_t titleIdOfDisc = 0;
    bool discInserted;

    uint32_t attempt = 0;
    while (!GetTitleIdOfDisc(&titleIdOfDisc, &discInserted)) {
        if (++attempt > 20) {
            break;
        }
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    bool wrongDiscInserted = discInserted && (titleIdOfDisc != expectedTitleId);

    if (discInserted && !wrongDiscInserted) {
        DrawUtils::deinitFont();
        free(screenBuffer);
        return true;
    }

    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);
    DrawUtils::setFontColor(COLOR_TEXT);

    DrawUtils::setFontSize(48);

    if (wrongDiscInserted) {
        const char *title = "The disc inserted into the console";
        DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(title) / 2, 40 + 48 + 8, title, true);
        title = "is for a different software title.";
        DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(title) / 2, 40 + 2 * 48 + 8, title, true);
        title = "Please change the disc.";
        DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(title) / 2, 40 + 4 * 48 + 8, title, true);
    } else {
        const char *title = "Please insert a disc.";
        DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(title) / 2, 40 + 48 + 8, title, true);
    }

    DrawUtils::setFontSize(18);
    const char *exitHints = "\ue000 Launch Wii U Menu";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHints) / 2, SCREEN_HEIGHT - 8, exitHints, true);

    DrawUtils::endDraw();

    // When an unexpected disc was inserted we need to eject it first.
    bool allowDisc = !wrongDiscInserted;
    while (true) {
        VPADStatus vpad{};
        VPADRead(VPAD_CHAN_0, &vpad, 1, nullptr);

        if (vpad.trigger & VPAD_BUTTON_A) {
            result = false;
            break;
        }

        if (GetTitleIdOfDisc(&titleIdOfDisc, &discInserted)) {
            if (discInserted) {
                if (!allowDisc) {
                    continue;
                }
                DEBUG_FUNCTION_LINE("Disc inserted! %016llX", titleIdOfDisc);
                result = expectedTitleId == titleIdOfDisc;
                break;
            }
        } else {
            allowDisc = true;
        }
    }

    DrawUtils::clear(COLOR_BLACK);
    DrawUtils::endDraw();

    DrawUtils::deinitFont();

    free(screenBuffer);

    return result;
}
