// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Thread.hpp"  // IWYU pragma: associated

#include <StorageThread.pb.h>
#include <StorageThreadItem.pb.h>
#include <atomic>
#include <memory>
#include <optional>
#include <source_location>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageThread.hpp"
#include "internal/serialization/protobuf/verify/StorageThreadItem.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/otx/client/StorageBox.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Mailbox.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Thread::Thread(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const identifier::Generic& id,
    const Hash& hash,
    std::string_view alias,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          1)
    , id_(id)
    , alias_(alias)
    , index_(0)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , items_()
    , participants_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

Thread::Thread(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const identifier::Generic& id,
    const UnallocatedSet<identifier::Generic>& participants,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(
          crypto,
          factory,
          storage,
          NullHash{},
          std::source_location::current().function_name(),
          1)
    , id_(id)
    , alias_()
    , index_(0)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , items_()
    , participants_(participants)
{
    blank();
}

auto Thread::Add(
    const identifier::Generic& id,
    Time time,
    const otx::client::StorageBox& box,
    std::string_view alias,
    const UnallocatedCString& contents,
    const std::uint64_t index,
    const identifier::Generic& workflow,
    const std::uint32_t chain) -> bool
{
    const auto lock = Lock{write_lock_};

    auto saved{true};
    auto unread{true};

    switch (box) {
        case otx::client::StorageBox::MAILINBOX: {
            saved = mail_inbox_.Store(id, contents, alias);
        } break;
        case otx::client::StorageBox::MAILOUTBOX: {
            saved = mail_outbox_.Store(id, contents, alias);
            unread = false;
        } break;
        case otx::client::StorageBox::OUTGOINGCHEQUE:
        case otx::client::StorageBox::OUTGOINGTRANSFER:
        case otx::client::StorageBox::INTERNALTRANSFER: {
            unread = false;
        } break;
        case otx::client::StorageBox::BLOCKCHAIN:
        case otx::client::StorageBox::INCOMINGCHEQUE:
        case otx::client::StorageBox::INCOMINGTRANSFER: {
        } break;
        default: {
            LogError()()("Warning: unknown box.").Flush();
        }
    }

    if (false == saved) {
        LogError()()("Unable to save item.").Flush();

        return false;
    }

    auto& item = items_[id];
    item.set_version(version_);
    item.set_id(id.asBase58(crypto_));

    if (0 == index) {
        item.set_index(index_++);
    } else {
        item.set_index(index);
    }

    item.set_time(seconds_since_epoch_unsigned(time).value());
    item.set_box(static_cast<std::uint32_t>(box));
    item.set_account(workflow.asBase58(crypto_));
    item.set_unread(unread);

    if (otx::client::StorageBox::BLOCKCHAIN == box) {
        item.set_chain(chain);
        item.set_txid(contents);
    }

    const auto valid = proto::Validate(item, VERBOSE);

    if (false == valid) {
        items_.erase(id);

        return false;
    }

    return save(lock);
}

auto Thread::Alias() const -> UnallocatedCString
{
    const auto lock = Lock{write_lock_};

    return alias_;
}

auto Thread::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageThread>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 1u:
            default: {
                for (const auto& participant : proto.participant()) {
                    participants_.emplace(
                        factory_.IdentifierFromBase58(participant));
                }

                for (const auto& it : proto.item()) {
                    const auto index = convert_to_size(it.index());
                    items_.emplace(factory_.IdentifierFromBase58(it.id()), it);

                    if (index >= index_) { index_ = index + 1; }
                }
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Thread::Check(const identifier::Generic& id) const -> bool
{
    const auto lock = Lock{write_lock_};

    return items_.end() != items_.find(id);
}

auto Thread::ID() const -> identifier::Generic { return id_; }

auto Thread::Items() const -> proto::StorageThread
{
    const auto lock = Lock{write_lock_};

    return serialize(lock);
}

auto Thread::Read(const identifier::Generic& id, const bool unread) -> bool
{
    const auto lock = Lock{write_lock_};

    auto it = items_.find(id);

    if (items_.end() == it) {
        LogError()()("Item does not exist.").Flush();

        return false;
    }

    auto& item = it->second;

    item.set_unread(unread);

    return save(lock);
}

auto Thread::Remove(const identifier::Generic& id) -> bool
{
    const auto lock = Lock{write_lock_};

    auto it = items_.find(id);

    if (items_.end() == it) { return false; }

    auto& item = it->second;
    auto box = static_cast<otx::client::StorageBox>(item.box());
    items_.erase(it);

    switch (box) {
        case otx::client::StorageBox::MAILINBOX: {
            mail_inbox_.Delete(id);
        } break;
        case otx::client::StorageBox::MAILOUTBOX: {
            mail_outbox_.Delete(id);
        } break;
        case otx::client::StorageBox::BLOCKCHAIN: {
        } break;
        default: {
            LogError()()("Warning: unknown box.").Flush();
        }
    }

    return save(lock);
}

auto Thread::Rename(const identifier::Generic& newID) -> bool
{
    const auto lock = Lock{write_lock_};
    const auto oldID = id_;
    id_ = newID;

    if (participants_.contains(oldID)) {
        participants_.erase(oldID);
        participants_.emplace(newID);
    }

    return save(lock);
}

auto Thread::save(const Lock& lock) const -> bool
{
    assert_true(verify_write_lock(lock));

    auto serialized = serialize(lock);

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

auto Thread::serialize(const Lock& lock) const -> proto::StorageThread
{
    assert_true(verify_write_lock(lock));

    proto::StorageThread serialized;
    serialized.set_version(version_);
    serialized.set_id(id_.asBase58(crypto_));

    for (const auto& nym : participants_) {
        if (!nym.empty()) {
            *serialized.add_participant() = nym.asBase58(crypto_);
        }
    }

    auto sorted = sort(lock);

    for (const auto& it : sorted) {
        assert_false(nullptr == it.second);

        const auto& item = *it.second;
        *serialized.add_item() = item;
    }

    return serialized;
}

auto Thread::SetAlias(std::string_view alias) -> bool
{
    const auto lock = Lock{write_lock_};

    alias_ = alias;

    return true;
}

auto Thread::sort(const Lock& lock) const -> Thread::SortedItems
{
    assert_true(verify_write_lock(lock));

    SortedItems output;

    for (const auto& it : items_) {
        const auto& id = it.first;
        const auto& item = it.second;

        if (!id.empty()) {
            SortKey key{item.index(), item.time(), id};
            output.emplace(key, &item);
        }
    }

    return output;
}

auto Thread::UnreadCount() const -> std::size_t
{
    const auto lock = Lock{write_lock_};
    std::size_t output{0};

    for (const auto& it : items_) {
        const auto& item = it.second;

        if (item.unread()) { ++output; }
    }

    return output;
}

auto Thread::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        default: {
            for (auto& it : items_) {
                auto& item = it.second;
                const auto box =
                    static_cast<otx::client::StorageBox>(item.box());

                switch (box) {
                    case otx::client::StorageBox::MAILOUTBOX: {
                        if (item.unread()) {
                            item.set_unread(false);
                            changed = true;
                        }
                    } break;
                    default: {
                    }
                }
            }
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree
