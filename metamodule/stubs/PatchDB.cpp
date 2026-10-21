// Replaces surge/src/common/PatchDB.cpp on MetaModule (see cmake/surge-engine.cmake).
// The patch database is the Surge synth's patch browser: it needs sqlite and a writer
// thread, and the Rack modules don't use it. SurgeStorage still constructs one.

#include "PatchDB.h"

namespace Surge
{
namespace PatchStorage
{

struct PatchDB::WriterWorker
{
};

PatchDB::PatchDB(SurgeStorage *s) : storage(s) {}

PatchDB::~PatchDB() = default;

// SurgeStorage calls these when it scans for patches, which the Rack modules turn off
void PatchDB::prepareForWrites() {}

void PatchDB::considerFXPForLoad(const fs::path &, const std::string &, const std::string &,
                                 const CatType) const
{
}

void PatchDB::addRootCategory(const std::string &, CatType) {}

void PatchDB::addSubCategory(const std::string &, const std::string &, CatType) {}

void PatchDB::erasePatchByID(int) {}

std::unordered_map<std::string, std::pair<int, int64_t>> PatchDB::readAllPatchPathsWithIdAndModTime()
{
    return {};
}

std::vector<std::string> PatchDB::readUserFavorites() { return {}; }

} // namespace PatchStorage
} // namespace Surge
