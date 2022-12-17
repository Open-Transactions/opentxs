// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/database/Headers.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>
#include <BlockchainBlockLocalData.pb.h>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "blockchain/node/UpdateTransaction.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::database
{
class Headers::IsSameTip
{
public:
    auto operator()(const std::monostate& rhs) const noexcept -> bool
    {
        return false;
    }
    auto operator()(const TipData& rhs) const noexcept -> bool
    {
        return tip_ == rhs;
    }
    auto operator()(const ReorgData& rhs) const noexcept -> bool
    {
        return false;
    }

    IsSameTip(const block::Position& tip) noexcept
        : tip_(tip)
    {
    }

private:
    const block::Position& tip_;
};

class Headers::IsSameReorg
{
public:
    auto operator()(const std::monostate& rhs) const noexcept -> bool
    {
        return false;
    }
    auto operator()(const TipData& rhs) const noexcept -> bool { return false; }
    auto operator()(const ReorgData& rhs) const noexcept -> bool
    {
        return (parent_ == rhs.first) && (tip_ == rhs.second);
    }

    IsSameReorg(
        const block::Position& parent,
        const block::Position& tip) noexcept
        : parent_(parent)
        , tip_(tip)
    {
    }

private:
    const block::Position& parent_;
    const block::Position& tip_;
};
}  // namespace opentxs::blockchain::database

namespace opentxs::blockchain::database
{
Headers::Headers(
    const api::Session& api,
    const node::Manager& network,
    const common::Database& common,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , chain_(type)
    , lock_()
    , publish_tip_internal_([&] {
        using Type = network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Publish);
        const auto rc = out.Bind(
            network.Internal().Endpoints().new_header_publish_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , to_blockchain_api_([&] {
        using Type = network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc = out.Connect(
            api.Endpoints().Internal().BlockchainMessageRouter().data());

        OT_ASSERT(rc);

        return out;
    }())
    , last_update_(std::monostate{})
{
    import_genesis(chain_);

    {
        const auto best = this->best();

        OT_ASSERT(HeaderExists(best.hash_));
        OT_ASSERT(0 <= best.height_);
    }

    {
        const auto header = CurrentBest();

        OT_ASSERT(header.IsValid());
        OT_ASSERT(0 <= header.Position().height_);
    }
}

auto Headers::ApplyUpdate(const node::UpdateTransaction& update) noexcept
    -> bool
{
    if (false == common_.StoreBlockHeaders(update.UpdatedHeaders())) {
        LogError()(OT_PRETTY_CLASS())("Failed to save block headers").Flush();

        return false;
    }

    Lock lock(lock_);
    const auto initialHeight = best(lock).height_;
    auto parentTxn = lmdb_.TransactionRW();

    if (update.HaveCheckpoint()) {
        const auto& checkpoint = update.Checkpoint();

        if (false ==
            lmdb_
                .Store(
                    ChainData,
                    tsv(static_cast<std::size_t>(Key::CheckpointHeight)),
                    tsv(static_cast<std::size_t>(checkpoint.height_)),
                    parentTxn)
                .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save checkpoint height")
                .Flush();

            return false;
        }

        if (false == lmdb_
                         .Store(
                             ChainData,
                             tsv(static_cast<std::size_t>(Key::CheckpointHash)),
                             checkpoint.hash_.Bytes(),
                             parentTxn)
                         .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save checkpoint hash")
                .Flush();

            return false;
        }

        LogConsole()(print(chain_))(" block header checkpoint updated to ")(
            checkpoint)
            .Flush();
    }

    for (const auto& [parent, child] : update.Disconnected()) {
        if (false == lmdb_
                         .Store(
                             BlockHeaderDisconnected,
                             parent.Bytes(),
                             child.Bytes(),
                             parentTxn)
                         .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save disconnected hash")
                .Flush();

            return false;
        }
    }

    for (const auto& [parent, child] : update.Connected()) {
        if (false == lmdb_.Delete(
                         BlockHeaderDisconnected,
                         parent.Bytes(),
                         child.Bytes(),
                         parentTxn)) {
            LogError()(OT_PRETTY_CLASS())("Failed to delete disconnected hash")
                .Flush();

            return false;
        }
    }

    for (const auto& hash : update.SiblingsToAdd()) {
        if (false ==
            lmdb_
                .Store(
                    BlockHeaderSiblings, hash.Bytes(), hash.Bytes(), parentTxn)
                .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save sibling hash")
                .Flush();

            return false;
        }
    }

    for (const auto& hash : update.SiblingsToDelete()) {
        lmdb_.Delete(BlockHeaderSiblings, hash.Bytes(), parentTxn);
    }

    for (const auto& data : update.UpdatedHeaders()) {
        const auto& [hash, pair] = data;
        const auto result = lmdb_.Store(
            BlockHeaderMetadata,
            hash.Bytes(),
            [&] {
                auto out = block::internal::Header::SerializedType{};
                data.second.first.Internal().Serialize(out);

                return proto::ToString(out.local());
            }(),
            parentTxn);

        if (false == result.first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save block metadata")
                .Flush();

            return false;
        }
    }

    if (update.HaveReorg()) {
        for (auto i = initialHeight; i > update.ReorgParent().height_; --i) {
            if (false == pop_best(i, parentTxn)) {
                LogError()(OT_PRETTY_CLASS())("Failed to delete best hash")
                    .Flush();

                return false;
            }
        }
    }

    for (const auto& position : update.BestChain()) {
        push_best(block::Position{position}, false, parentTxn);
    }

    if (0 < update.BestChain().size()) {
        const auto& tip = *update.BestChain().crbegin();

        if (false == lmdb_
                         .Store(
                             ChainData,
                             tsv(static_cast<std::size_t>(Key::TipHeight)),
                             tsv(static_cast<std::size_t>(tip.first)),
                             parentTxn)
                         .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to store best hash").Flush();

            return false;
        }
    }

    if (false == parentTxn.Finalize(true)) {
        LogError()(OT_PRETTY_CLASS())("Database error").Flush();

        return false;
    }

    const auto tip = best(lock);

    if (update.HaveReorg()) {
        const auto& parent = update.ReorgParent();
        auto visitor = IsSameReorg{parent, tip};
        auto isSame = std::visit(visitor, last_update_);

        if (false == isSame) {
            LogConsole()(print(chain_))(
                " reorg detected. Last common ancestor is ")(parent.print())
                .Flush();
            publish_tip_internal_.SendDeferred(
                [&] {
                    auto work = MakeWork(OT_ZMQ_REORG_SIGNAL);
                    work.AddFrame(parent.hash_);
                    work.AddFrame(parent.height_);
                    work.AddFrame(tip.hash_);
                    work.AddFrame(tip.height_);

                    return work;
                }(),
                __FILE__,
                __LINE__);
            to_blockchain_api_.SendDeferred(
                [&] {
                    auto work = MakeWork(WorkType::BlockchainReorg);
                    work.AddFrame(chain_);
                    work.AddFrame(parent.hash_);
                    work.AddFrame(parent.height_);
                    work.AddFrame(tip.hash_);
                    work.AddFrame(tip.height_);

                    return work;
                }(),
                __FILE__,
                __LINE__);
            last_update_ = std::make_pair(parent, tip);
        }
    } else {
        auto visitor = IsSameTip{tip};
        auto isSame = std::visit(visitor, last_update_);

        if (false == isSame) { report(lock, tip); }
    }

    return true;
}

auto Headers::BestBlock(const block::Height position) const noexcept(false)
    -> block::Hash
{
    auto output = block::Hash{};

    if (0 > position) { return output; }

    lmdb_.Load(
        BlockHeaderBest,
        tsv(static_cast<std::size_t>(position)),
        [&](const auto in) -> void {
            const auto rc = output.Assign(in.data(), in.size());

            if (!rc) {
                throw std::runtime_error("Database contains invalid hash");
            }
        });

    if (output.IsNull()) {
        // TODO some callers which should be catching this exception aren't.
        // Clean up those call sites then start throwing this exception.
        // throw std::out_of_range("No best hash at specified height");
    }

    return output;
}

auto Headers::best() const noexcept -> block::Position
{
    Lock lock(lock_);

    return best(lock);
}

auto Headers::best(const Lock& lock) const noexcept -> block::Position
{
    auto output = block::Position{};
    auto height = 0_uz;

    if (false ==
        lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::TipHeight)),
            [&](const auto in) -> void {
                std::memcpy(
                    &height, in.data(), std::min(in.size(), sizeof(height)));
            })) {

        return block::Position{};
    }

    if (false ==
        lmdb_.Load(BlockHeaderBest, tsv(height), [&](const auto in) -> void {
            const auto rc = output.hash_.Assign(in.data(), in.size());

            OT_ASSERT(rc);  // TODO exception
        })) {

        return block::Position{};
    }

    output.height_ = height;

    return output;
}

auto Headers::checkpoint(const Lock& lock) const noexcept -> block::Position
{
    auto output = block::Position{};
    auto height = 0_uz;

    if (false ==
        lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::CheckpointHeight)),
            [&](const auto in) -> void {
                std::memcpy(
                    &height, in.data(), std::min(in.size(), sizeof(height)));
            })) {
        return block::Position{};
    }

    if (false == lmdb_.Load(
                     ChainData,
                     tsv(static_cast<std::size_t>(Key::CheckpointHash)),
                     [&](const auto in) -> void {
                         const auto rc =
                             output.hash_.Assign(in.data(), in.size());

                         OT_ASSERT(rc);  // TODO exception
                     })) {

        return block::Position{};
    }

    output.height_ = height;

    return output;
}

auto Headers::CurrentBest() const noexcept -> block::Header
{
    try {

        return load_header(best().hash_);
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto Headers::CurrentCheckpoint() const noexcept -> block::Position
{
    Lock lock(lock_);

    return checkpoint(lock);
}

auto Headers::DisconnectedHashes() const noexcept -> database::DisconnectedList
{
    Lock lock(lock_);
    auto output = database::DisconnectedList{};
    lmdb_.Read(
        BlockHeaderDisconnected,
        [&](const auto key, const auto value) -> bool {
            output.emplace(block::Hash{key}, block::Hash{value});

            return true;
        },
        storage::lmdb::Dir::Forward);

    return output;
}

auto Headers::HasDisconnectedChildren(const block::Hash& hash) const noexcept
    -> bool
{
    Lock lock(lock_);

    return lmdb_.Exists(BlockHeaderDisconnected, hash.Bytes());
}

auto Headers::HaveCheckpoint() const noexcept -> bool
{
    Lock lock(lock_);

    return 0 < checkpoint(lock).height_;
}

auto Headers::header_exists(const Lock& lock, const block::Hash& hash)
    const noexcept -> bool
{
    return common_.BlockHeaderExists(hash) &&
           lmdb_.Exists(BlockHeaderMetadata, hash.Bytes());
}

auto Headers::HeaderExists(const block::Hash& hash) const noexcept -> bool
{
    Lock lock(lock_);

    return header_exists(lock, hash);
}

auto Headers::import_genesis(const blockchain::Type type) const noexcept -> void
{
    auto success{false};
    const auto& genesis = params::get(type).GenesisBlock().Header();
    const auto& hash = genesis.Hash();

    try {
        const auto serialized = common_.LoadBlockHeader(hash);

        if (false == lmdb_.Exists(BlockHeaderMetadata, hash.Bytes())) {
            auto header = api_.Factory().InternalSession().BlockHeader(
                serialized, {});  // TODO allocator

            OT_ASSERT(header.IsValid());

            const auto result =
                lmdb_.Store(BlockHeaderMetadata, hash.Bytes(), [&] {
                    auto proto = block::internal::Header::SerializedType{};
                    header.Internal().Serialize(proto);

                    return proto::ToString(proto.local());
                }());

            OT_ASSERT(result.first);
        }
    } catch (...) {
        success = common_.StoreBlockHeaders([&] {
            auto out = UpdatedHeader{};
            out.try_emplace(genesis.Hash(), genesis, true);

            return out;
        }());

        OT_ASSERT(success);

        success = lmdb_
                      .Store(
                          BlockHeaderMetadata,
                          hash.Bytes(),
                          [&] {
                              auto proto =
                                  block::internal::Header::SerializedType{};
                              genesis.Internal().Serialize(proto);

                              return proto::ToString(proto.local());
                          }())
                      .first;

        OT_ASSERT(success);
    }

    OT_ASSERT(HeaderExists(hash));

    if (0 > best().height_) {
        auto transaction = lmdb_.TransactionRW();
        success = push_best({0, hash}, true, transaction);

        OT_ASSERT(success);

        success = transaction.Finalize(true);

        OT_ASSERT(success);

        const auto best = this->best();

        OT_ASSERT(0 == best.height_);
        OT_ASSERT(hash == best.hash_);
    }

    OT_ASSERT(0 <= best().height_);
}

auto Headers::IsSibling(const block::Hash& hash) const noexcept -> bool
{
    Lock lock(lock_);

    return lmdb_.Exists(BlockHeaderSiblings, hash.Bytes());
}

auto Headers::load_header(const block::Hash& hash) const -> block::Header
{
    auto proto = common_.LoadBlockHeader(hash);
    const auto haveMeta =
        lmdb_.Load(BlockHeaderMetadata, hash.Bytes(), [&](const auto data) {
            *proto.mutable_local() =
                proto::Factory<proto::BlockchainBlockLocalData>(
                    data.data(), data.size());
        });

    if (false == haveMeta) {
        throw std::out_of_range("Block header metadata not found");
    }

    auto output = api_.Factory().InternalSession().BlockHeader(
        proto, {});  // TODO allocator

    OT_ASSERT(output.IsValid());

    return output;
}

auto Headers::pop_best(block::Height i, storage::lmdb::Transaction& parent)
    const noexcept -> bool
{
    return lmdb_.Delete(BlockHeaderBest, tsv(i), parent);
}

auto Headers::push_best(
    const block::Position next,
    const bool setTip,
    storage::lmdb::Transaction& parent) const noexcept -> bool
{
    auto output = lmdb_.Store(
        BlockHeaderBest,
        tsv(static_cast<std::size_t>(next.height_)),
        next.hash_.Bytes(),
        parent);

    if (output.first && setTip) {
        output = lmdb_.Store(
            ChainData,
            tsv(static_cast<std::size_t>(Key::TipHeight)),
            tsv(static_cast<std::size_t>(next.height_)),
            parent);
    }

    return output.first;
}

auto Headers::RecentHashes(alloc::Default alloc) const noexcept
    -> Vector<block::Hash>
{
    Lock lock(lock_);

    return recent_hashes(lock, alloc);
}

auto Headers::recent_hashes(const Lock& lock, alloc::Default alloc)
    const noexcept -> Vector<block::Hash>
{
    auto output = Vector<block::Hash>{alloc};
    lmdb_.Read(
        BlockHeaderBest,
        [&](const auto, const auto value) -> bool {
            output.emplace_back(value);

            return 100 > output.size();
        },
        storage::lmdb::Dir::Backward);

    return output;
}

auto Headers::report(const Lock& lock) noexcept -> void
{
    report(lock, best(lock));
}

auto Headers::report(const Lock&, const block::Position& tip) noexcept -> void
{
    publish_tip_internal_.SendDeferred(
        [&] {
            auto work = MakeWork(OT_ZMQ_NEW_BLOCK_HEADER_SIGNAL);
            work.AddFrame(tip.hash_);
            work.AddFrame(tip.height_);

            return work;
        }(),
        __FILE__,
        __LINE__);
    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = MakeWork(WorkType::BlockchainNewHeader);
            work.AddFrame(chain_);
            work.AddFrame(tip.hash_);
            work.AddFrame(tip.height_);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Headers::ReportTip() noexcept -> void
{
    auto lock = Lock{lock_};
    report(lock);
}

auto Headers::SiblingHashes() const noexcept -> database::Hashes
{
    Lock lock(lock_);
    auto output = database::Hashes{};
    lmdb_.Read(
        BlockHeaderSiblings,
        [&](const auto, const auto value) -> bool {
            output.emplace(value);

            return true;
        },
        storage::lmdb::Dir::Forward);

    return output;
}

auto Headers::TryLoadHeader(const block::Hash& hash) const noexcept
    -> block::Header
{
    try {
        return LoadHeader(hash);
    } catch (...) {
        return {};
    }
}
}  // namespace opentxs::blockchain::database
