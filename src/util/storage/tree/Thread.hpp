// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/StorageThread.pb.h>
#include <opentxs/protobuf/StorageThreadItem.pb.h>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>

#include "internal/util/Mutex.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
namespace tree
{
class Mailbox;
class Threads;
}  // namespace tree

namespace driver
{
class Plugin;
}  // namespace driver
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Thread final : public Node
{
public:
    auto Alias() const -> UnallocatedCString;
    auto Check(const identifier::Generic& id) const -> bool;
    auto ID() const -> identifier::Generic;
    auto Items() const -> protobuf::StorageThread;
    auto UnreadCount() const -> std::size_t;

    auto Add(
        const identifier::Generic& id,
        Time time,
        const otx::client::StorageBox& box,
        std::string_view alias,
        const UnallocatedCString& contents,
        const std::uint64_t index = 0,
        const identifier::Generic& workflow = {},
        const std::uint32_t chain = {}) -> bool;
    auto Read(const identifier::Generic& id, const bool unread) -> bool;
    auto Rename(const identifier::Generic& newID) -> bool;
    auto Remove(const identifier::Generic& id) -> bool;
    auto SetAlias(std::string_view alias) -> bool;

    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    auto operator=(const Thread&) -> Thread = delete;
    auto operator=(Thread&&) -> Thread = delete;

    ~Thread() final = default;

private:
    friend Threads;
    using SortKey = std::tuple<std::size_t, std::int64_t, identifier::Generic>;
    using SortedItems =
        UnallocatedMap<SortKey, const protobuf::StorageThreadItem*>;

    identifier::Generic id_;
    UnallocatedCString alias_;
    std::size_t index_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    UnallocatedMap<identifier::Generic, protobuf::StorageThreadItem> items_;
    // It's important to use a sorted container for this so the thread ID can be
    // calculated deterministically
    UnallocatedSet<identifier::Generic> participants_;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const Lock& lock) const -> bool final;
    auto serialize(const Lock& lock) const -> protobuf::StorageThread;
    auto sort(const Lock& lock) const -> SortedItems;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Thread(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const identifier::Generic& id,
        const Hash& hash,
        std::string_view alias,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const identifier::Generic& id,
        const UnallocatedSet<identifier::Generic>& participants,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
};
}  // namespace opentxs::storage::tree
