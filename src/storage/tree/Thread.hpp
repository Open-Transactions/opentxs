// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Editor.hpp"
#include "serialization/protobuf/StorageThread.pb.h"
#include "serialization/protobuf/StorageThreadItem.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
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

namespace opentxs::storage
{
class Thread final : public Node
{
private:
    friend Threads;
    using SortKey = std::tuple<std::size_t, std::int64_t, std::string>;
    using SortedItems = std::pmr::map<SortKey, const proto::StorageThreadItem*>;

    std::string id_;
    std::string alias_;
    std::size_t index_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    std::pmr::map<std::string, proto::StorageThreadItem> items_;
    // It's important to use a sorted container for this so the thread ID can be
    // calculated deterministically
    std::pmr::set<std::string> participants_;

    void init(const std::string& hash) final;
    auto save(const Lock& lock) const -> bool final;
    auto serialize(const Lock& lock) const -> proto::StorageThread;
    auto sort(const Lock& lock) const -> SortedItems;
    void upgrade(const Lock& lock);

    Thread(
        const Driver& storage,
        const std::string& id,
        const std::string& hash,
        const std::string& alias,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread(
        const Driver& storage,
        const std::string& id,
        const std::pmr::set<std::string>& participants,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    auto operator=(const Thread&) -> Thread = delete;
    auto operator=(Thread&&) -> Thread = delete;

public:
    auto Alias() const -> std::string;
    auto Check(const std::string& id) const -> bool;
    auto ID() const -> std::string;
    auto Items() const -> proto::StorageThread;
    auto Migrate(const Driver& to) const -> bool final;
    auto UnreadCount() const -> std::size_t;

    auto Add(
        const std::string& id,
        const std::uint64_t time,
        const StorageBox& box,
        const std::string& alias,
        const std::string& contents,
        const std::uint64_t index = 0,
        const std::string& account = {},
        const std::uint32_t chain = {}) -> bool;
    auto Read(const std::string& id, const bool unread) -> bool;
    auto Rename(const std::string& newID) -> bool;
    auto Remove(const std::string& id) -> bool;
    auto SetAlias(const std::string& alias) -> bool;

    ~Thread() final = default;
};
}  // namespace opentxs::storage
