// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "serialization/protobuf/StorageNymList.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{
class Driver;
class Mailbox;
class Nym;
class Thread;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Threads final : public Node
{
    using ot_super = Node;

public:
    auto BlockchainThreadMap(const Data& txid) const noexcept
        -> std::pmr::vector<OTIdentifier>;
    auto BlockchainTransactionList() const noexcept -> std::pmr::vector<OTData>;
    auto Exists(const std::string& id) const -> bool;
    using ot_super::List;
    auto List(const bool unreadOnly) const -> ObjectList;
    auto Migrate(const Driver& to) const -> bool final;
    auto Thread(const std::string& id) const -> const storage::Thread&;

    auto AddIndex(const Data& txid, const Identifier& thread) noexcept -> bool;
    auto Create(
        const std::string& id,
        const std::pmr::set<std::string>& participants) -> std::string;
    auto FindAndDeleteItem(const std::string& itemID) -> bool;
    auto mutable_Thread(const std::string& id) -> Editor<storage::Thread>;
    auto RemoveIndex(const Data& txid, const Identifier& thread) noexcept
        -> void;
    auto Rename(const std::string& existingID, const std::string& newID)
        -> bool;

    ~Threads() final = default;

private:
    friend Nym;

    struct BlockchainThreadIndex {
        using Txid = OTData;
        using ThreadID = OTIdentifier;

        mutable std::mutex lock_{};
        std::pmr::map<Txid, std::pmr::set<ThreadID>> map_{};
    };

    mutable std::pmr::map<std::string, std::unique_ptr<storage::Thread>>
        threads_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    BlockchainThreadIndex blockchain_;

    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;
    auto thread(const std::string& id) const -> storage::Thread*;
    auto thread(const std::string& id, const std::unique_lock<std::mutex>& lock)
        const -> storage::Thread*;

    auto create(
        const Lock& lock,
        const std::string& id,
        const std::pmr::set<std::string>& participants) -> std::string;
    void init(const std::string& hash) final;
    void save(
        storage::Thread* thread,
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);

    Threads(
        const Driver& storage,
        const std::string& hash,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Threads() = delete;
    Threads(const Threads&) = delete;
    Threads(Threads&&) = delete;
    auto operator=(const Threads&) -> Threads = delete;
    auto operator=(Threads&&) -> Threads = delete;
};
}  // namespace opentxs::storage
