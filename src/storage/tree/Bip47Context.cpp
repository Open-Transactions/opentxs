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

    for (const auto& it : serialized->chain()) {
        items_.emplace(it.type(), it);
    }

    Lock lock(write_lock_);
    save(lock);
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

proto::Bip47Context Bip47Context::Items() const
{
    Lock lock(write_lock_);

    return serialize(lock);
}

}  // namespace storage
}  // namespace opentxs
