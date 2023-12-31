// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <opentxs/protobuf/StorageNymList.pb.h>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Thread.hpp"

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

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Mailbox;
class Nym;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Threads final : public Node
{
    using ot_super = Node;

public:
    auto BlockchainThreadMap(const blockchain::block::TransactionHash& txid)
        const noexcept -> UnallocatedVector<identifier::Generic>;
    auto BlockchainTransactionList() const noexcept
        -> UnallocatedVector<ByteArray>;
    auto Exists(const identifier::Generic& id) const -> bool;
    using ot_super::List;
    auto List(const bool unreadOnly) const -> ObjectList;
    auto Thread(const identifier::Generic& id) const -> const tree::Thread&;

    auto AddIndex(
        const blockchain::block::TransactionHash& txid,
        const identifier::Generic& thread) noexcept -> bool;
    auto Create(
        const identifier::Generic& id,
        const UnallocatedSet<identifier::Generic>& participants)
        -> identifier::Generic;
    auto FindAndDeleteItem(const identifier::Generic& itemID) -> bool;
    auto mutable_Thread(const identifier::Generic& id) -> Editor<tree::Thread>;
    auto RemoveIndex(
        const blockchain::block::TransactionHash& txid,
        const identifier::Generic& thread) noexcept -> void;
    auto Rename(
        const identifier::Generic& existingID,
        const identifier::Generic& newID) -> bool;

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

    mutable UnallocatedMap<identifier::Generic, std::unique_ptr<tree::Thread>>
        threads_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    BlockchainThreadIndex blockchain_;

    auto dump(const Lock&, const Log&, Vector<Hash>& out) const noexcept
        -> bool final;

    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> protobuf::StorageNymList;
    auto thread(const identifier::Generic& id) const -> tree::Thread*;
    auto thread(
        const identifier::Generic& id,
        const std::unique_lock<std::mutex>& lock) const -> tree::Thread*;

    auto create(
        const Lock& lock,
        const identifier::Generic& id,
        const UnallocatedSet<identifier::Generic>& participants)
        -> identifier::Generic;
    auto init(const Hash& hash) noexcept(false) -> void final;
    void save(
        tree::Thread* thread,
        const std::unique_lock<std::mutex>& lock,
        const identifier::Generic& id);
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Threads(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
};
}  // namespace opentxs::storage::tree
