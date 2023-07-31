#pragma once

#include "utils/utils.h"
#include <cstdint>
#include <memory>
#include <optional>

typedef struct ModpackInformationIni {
    uint32_t version = -1;
    std::string name;
    std::string titleids;
    std::string path;
    std::string updatepath;
} ModpackInformationIni;

class ModpackInformation {
public:
    ModpackInformation(uint64_t titleId, std::string &path, std::string &WUHBPath) {
        this->mTitleId  = titleId;
        this->mPath     = path;
        this->mWUHBPath = WUHBPath;
    }
    virtual ~ModpackInformation() = default;

    virtual uint32_t getHandle() {
        return reinterpret_cast<uint32_t>(this);
    }

    static std::optional<std::unique_ptr<ModpackInformation>> Parse(ModpackInformationIni &ini, std::string bundlePath) {
        if (ini.version != 1) {
            return {};
        }
        uint64_t titleId = strtoll(ini.titleids.c_str(), nullptr, 16);
        if (titleId == 0) {
            return {};
        }

        return make_unique_nothrow<ModpackInformation>(titleId, ini.path, bundlePath);
    }

    uint64_t mTitleId;
    std::string mPath;
    std::string mWUHBPath;
};
