#include "filesystem.h"
#include "amul.file.h"
#include "amul.test.h"
#include "filesystem.inl.h"
#include "logging.h"

std::string gameDir{};

// File names
const char *gameFile = "Data/GameData.amulo";
const char *npcDataFile = "Data/npcd.amulo";
const char *npcCmdFile = "Data/npcc.amulo";

constexpr bool
isSeparator(char ptr)
{
    return (ptr == '/' || ptr == '\\');
}

void
PathAdd(std::string &into, const string_view rhs)
{
    bool wantSlash = !into.empty() && !isSeparator(into.back());
    into.reserve(into.length() + rhs.length());

    // only write single slashes, and only after seeing a
    // non-slash, so "////" => "" but "////a" => "/a"
    for (auto c : rhs) {
        /// TODO: CLEANUP: Jump between slashes
        if (isSeparator(c)) {
            wantSlash = true;
            continue;
        }
        if (wantSlash) {
            into += "/";
            wantSlash = false;
        }
        into += c;
    }
    // special case: /
    if (wantSlash && into.empty())
        into = "/";
}

error_t
PathJoin(std::string &into, const string_view lhs, const string_view rhs)
{
    into.clear();
    REQUIRE(!lhs.empty());
    REQUIRE(!rhs.empty());

    PathAdd(into, lhs);
    if (into.empty())
        into += ".";
    PathAdd(into, rhs);
    return 0;
}

void
CloseFile(FILE **fp)
{
    if (fp && *fp) {
        fclose(*fp);
        *fp = nullptr;
    }
}

void
UnlinkGameFile(const char *gamefile)
{
    std::string filepath{};
    if (gamedir_joiner(gamefile) == 0) {
        unlink(filepath.c_str());
    }
}

error_t
GetFilesSize(string_view filepath, size_t *sizep, bool required)
{
    REQUIRE(!filepath.empty() && sizep);

    struct stat sb = {};
    error_t err = stat(filepath.data(), &sb);
    if (err != 0) {
        if (required)
            LogFatal("Could not access file (", errno, "): ", filepath);
        return ENOENT;
    }
    *sizep = sb.st_size;
    return 0;
}