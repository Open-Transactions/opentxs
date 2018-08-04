#include "stdafx.hpp"

#include "Bip47Contexts.hpp"

#include "storage/Plugin.hpp"
#include "Bip47Context.hpp"

#include <utility>
#include <string>
#include <memory>
#include <functional>
#include <map>

#define CURRENT_VERSION 1

//#define OT_METHOD "opentxs::storage::Bip47Contexts::"

#include "Node.hpp"

namespace opentxs
{
namespace storage
{
Bip47Contexts::Bip47Contexts(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

void Bip47Contexts::init(const std::string& hash)
{
    // TODO
}

bool Bip47Contexts::save(const Lock& lock) const
{
    // TODO
    return false;
}

bool Bip47Contexts::Migrate(const opentxs::api::storage::Driver& to) const
{
    bool output{true};

    for (const auto index : item_map_) {
        // TODO: const auto& id = index.first;
        // TODO: const auto& node = *bip47context(id);
        // TODO: output &= node.Migrate(to);
    }

    output &= migrate(root_, to);

    return output;
}

class Bip47Context* Bip47Contexts::bip47context(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return bip47context(id, lock);
}

class Bip47Context* Bip47Contexts::bip47context(
    const std::string& id,
    const std::unique_lock<std::mutex>& lock) const
{
    // TODO
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = bip47contexts_[id];

    // TODO:
    const std::set<std::string> chains = {};

    if (!node) {
        node.reset(new class Bip47Context(driver_, id, hash, alias));

        if (!node) {
            std::cerr << __FUNCTION__ << ": Failed to instantiate thread."
                      << std::endl;
            abort();
        }
    }

    return node.get();
}

const class Bip47Context& Bip47Contexts::Bip47Context(
    const std::string& id) const
{
    return *bip47context(id);
}

proto::StorageNymList Bip47Contexts::serialize() const
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_nym());
        }
    }

    return serialized;
}

}  // namespace storage
}  // namespace opentxs
