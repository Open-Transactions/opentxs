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

#define OT_METHOD "opentxs::storage::Bip47Contexts::"

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

bool Bip47Contexts::Delete(const std::string& id) { return delete_item(id); }

bool Bip47Contexts::Exists(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return item_map_.find(id) != item_map_.end();
}

void Bip47Contexts::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load thread list index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) { version_ = 2; }

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Bip47Contexts::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
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

bool Bip47Contexts::Load(
    const std::string& id,
    std::shared_ptr<proto::Bip47Context>& output,
    const bool checking) const
{
    std::string alias{};

    return load_proto<proto::Bip47Context>(id, output, alias, checking);
}

bool Bip47Contexts::Store(const proto::Bip47Context& data)
{
    std::string alias{};
    std::string plaintext{};

    return store_proto(data, data.paymentcode(), alias, plaintext);
}

}  // namespace storage
}  // namespace opentxs
