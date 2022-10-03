// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageNymList.pb.h>
#include <memory>
#include <mutex>
#include <tuple>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
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
class Driver;
class Mailbox;
class Nym;
class Thread;
}  // namespace storage

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Threads final : public Node
{
    using ot_super = Node;

public:
    auto BlockchainThreadMap(const Data& txid) const noexcept
        -> UnallocatedVector<identifier::Generic>;
    auto BlockchainTransactionList() const noexcept
        -> UnallocatedVector<ByteArray>;
    auto Exists(const UnallocatedCString& id) const -> bool;
    using ot_super::List;
    auto List(const bool unreadOnly) const -> ObjectList;
    auto Migrate(const Driver& to) const -> bool final;
    auto Thread(const UnallocatedCString& id) const -> const storage::Thread&;

    auto AddIndex(const Data& txid, const identifier::Generic& thread) noexcept
        -> bool;
    auto Create(
        const UnallocatedCString& id,
        const UnallocatedSet<UnallocatedCString>& participants)
        -> UnallocatedCString;
    auto FindAndDeleteItem(const UnallocatedCString& itemID) -> bool;
    auto mutable_Thread(const UnallocatedCString& id)
        -> Editor<storage::Thread>;
    auto RemoveIndex(
        const Data& txid,
        const identifier::Generic& thread) noexcept -> void;
    auto Rename(
        const UnallocatedCString& existingID,
        const UnallocatedCString& newID) -> bool;

    Threads() = delete;
    Threads(const Threads&) = delete;
    Threads(Threads&&) = delete;
    auto operator=(const Threads&) -> Threads = delete;
    auto operator=(Threads&&) -> Threads = delete;

    ~Threads() final = default;

private:
    friend Nym;

    struct BlockchainThreadIndex {
        using Txid = ByteArray;
        using ThreadID = identifier::Generic;

        mutable std::mutex lock_{};
        UnallocatedMap<Txid, UnallocatedSet<ThreadID>> map_{};
    };

    mutable UnallocatedMap<UnallocatedCString, std::unique_ptr<storage::Thread>>
        threads_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    BlockchainThreadIndex blockchain_;

    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;
    auto thread(const UnallocatedCString& id) const -> storage::Thread*;
    auto thread(
        const UnallocatedCString& id,
        const std::unique_lock<std::mutex>& lock) const -> storage::Thread*;

    auto create(
        const Lock& lock,
        const UnallocatedCString& id,
        const UnallocatedSet<UnallocatedCString>& participants)
        -> UnallocatedCString;
    void init(const UnallocatedCString& hash) final;
    void save(
        storage::Thread* thread,
        const std::unique_lock<std::mutex>& lock,
        const UnallocatedCString& id);

    Threads(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& hash,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
};
}  // namespace opentxs::storage
