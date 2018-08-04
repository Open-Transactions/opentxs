#include "stdafx.hpp"

#include "Bip47Context.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include "storage/Plugin.hpp"

#define OT_METHOD "opentxs::storage::Bip47Context::"

namespace opentxs
{
namespace storage
{

Bip47Context::Bip47Context(
    const opentxs::api::storage::Driver& storage,
    const std::string& id,
    const std::string& hash,
    const std::string& alias)
    : Node(storage, hash)
    , id_(id)
    , alias_(alias)
    , index_(0)
{
    version_ = 1;
    root_ = Node::BLANK_HASH;
}

// std::string Bip47Context::PaymentCode() const { return paymentcode_; }

void Bip47Context::init(const std::string& hash)
{
    std::shared_ptr<proto::Bip47Context> serialized;
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to load thread index file." << std::endl;
        OT_FAIL;
    }

    version_ = serialized->version();

    if (1 > version_) { version_ = 1; }

    /*for (const auto& it : serialized->item()) {
        const auto& index = it.index();
        items_.emplace(it.id(), it);

        if (index >= index_) { index_ = index + 1; }
        }*/

    Lock lock(write_lock_);
    upgrade(lock);
}

std::size_t Bip47Context::ChainCount() const
{
    Lock lock(write_lock_);
    std::size_t output{0};

    /*for (const auto& it : items_) {
        const auto& item = it.second;

        if (item.unread()) { ++output; }
        }*/

    return output;
}

bool Bip47Context::Migrate(const opentxs::api::storage::Driver& to) const
{
    return Node::migrate(root_, to);
}

proto::Bip47Context Bip47Context::serialize(const Lock& lock) const
{
    proto::Bip47Context serialized;
    return serialized;
}

bool Bip47Context::save(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto serialized = serialize(lock);

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

void Bip47Context::upgrade(const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    bool changed{false};

    for (auto& it : items_) {
        auto& item = it.second;
        const auto box = static_cast<StorageBox>(item.box());

        switch (box) {
            case StorageBox::MAILOUTBOX:
            case StorageBox::OUTGOINGBLOCKCHAIN: {
                if (item.unread()) {
                    item.set_unread(false);
                    changed = true;
                }
            } break;
            default: {
            }
        }
    }

    if (changed) { save(lock); }
}

}  // namespace storage
}  // namespace opentxs
