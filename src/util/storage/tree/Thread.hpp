// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::client::StorageBox

#pragma once

#include <StorageThread.pb.h>
#include <StorageThreadItem.pb.h>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <tuple>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/otx/client/Types.hpp"
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

namespace proto
{
class StorageThreadItem;
}  // namespace proto

namespace storage
{
class Driver;
class Mailbox;
class Threads;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Thread final : public Node
{
public:
    auto Alias() const -> UnallocatedCString;
    auto Check(const UnallocatedCString& id) const -> bool;
    auto ID() const -> UnallocatedCString;
    auto Items() const -> proto::StorageThread;
    auto Migrate(const Driver& to) const -> bool final;
    auto UnreadCount() const -> std::size_t;

    auto Add(
        const UnallocatedCString& id,
        const std::uint64_t time,
        const otx::client::StorageBox& box,
        const UnallocatedCString& alias,
        const UnallocatedCString& contents,
        const std::uint64_t index = 0,
        const UnallocatedCString& account = {},
        const std::uint32_t chain = {}) -> bool;
    auto Read(const UnallocatedCString& id, const bool unread) -> bool;
    auto Rename(const UnallocatedCString& newID) -> bool;
    auto Remove(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& alias) -> bool;

    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    auto operator=(const Thread&) -> Thread = delete;
    auto operator=(Thread&&) -> Thread = delete;

    ~Thread() final = default;

private:
    friend Threads;
    using SortKey = std::tuple<std::size_t, std::int64_t, UnallocatedCString>;
    using SortedItems =
        UnallocatedMap<SortKey, const proto::StorageThreadItem*>;

    UnallocatedCString id_;
    UnallocatedCString alias_;
    std::size_t index_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    UnallocatedMap<UnallocatedCString, proto::StorageThreadItem> items_;
    // It's important to use a sorted container for this so the thread ID can be
    // calculated deterministically
    UnallocatedSet<UnallocatedCString> participants_;

    void init(const UnallocatedCString& hash) final;
    auto save(const Lock& lock) const -> bool final;
    auto serialize(const Lock& lock) const -> proto::StorageThread;
    auto sort(const Lock& lock) const -> SortedItems;
    void upgrade(const Lock& lock);

    Thread(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& id,
        const UnallocatedCString& hash,
        const UnallocatedCString& alias,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& id,
        const UnallocatedSet<UnallocatedCString>& participants,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
};
}  // namespace opentxs::storage
