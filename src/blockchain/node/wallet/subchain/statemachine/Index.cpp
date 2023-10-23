// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"  // IWYU pragma: associated

#include <memory>
#include <span>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Types.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Index::Imp::Imp(
    const std::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::index,
          alloc,
          {
              {parent->api_.Crypto().Blockchain().Internal().KeyEndpoint(),
               Connect},
          },
          {
              {parent->to_index_endpoint_, Bind},
          },
          {},
          {
              {Push,
               Internal,
               {
                   {parent->to_rescan_endpoint_, Connect},
               }},
              {Push,
               Internal,
               {
                   {parent->to_scan_endpoint_, Connect},
               }},
          })
    , to_rescan_(pipeline_.Internal().ExtraSocket(1))
    , to_scan_(pipeline_.Internal().ExtraSocket(2))
    , last_indexed_(std::nullopt)
{
}

auto Index::Imp::check_mempool(allocator_type monotonic) noexcept -> void
{
    const auto& log = api_.GetOptions().TestMode() ? LogConsole() : log_;
    const auto& oracle = parent_.mempool_oracle_;
    log()(name_)(": checking mempool for transactions").Flush();
    // TODO cache the mempool transactions and process them in batches to place
    // an upper bound on how long this function blocks.

    for (const auto& txid : oracle.Dump(monotonic)) {
        log()(name_)(": found transaction ").asHex(txid)(" in mempool").Flush();

        if (auto tx = oracle.Query(txid, monotonic); tx.IsValid()) {
            parent_.ProcessTransaction(tx, log, monotonic);
            log()(name_)(": transaction ").asHex(txid)(" processed").Flush();
        } else {
            log()(name_)(": transaction ")
                .asHex(txid)(" is not instantiated")
                .Flush();
        }
    }
}

auto Index::Imp::do_process_update(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(api_, msg, clean, dirty);

    for (const auto& [type, position] : clean) {
        if (ScanState::processed == type) {
            log_()(name_)(" re-indexing ")(name_)(" due to processed block ")(
                position)
                .Flush();
        }
    }

    do_work(monotonic);
    to_rescan_.SendDeferred(std::move(msg));
}

auto Index::Imp::do_startup_internal(allocator_type monotonic) noexcept -> void
{
    last_indexed_ = parent_.db_.SubchainLastIndexed(parent_.db_key_);
    do_work(monotonic);
    log_()(name_)(" notifying scan task to begin work").Flush();
    to_scan_.SendDeferred(MakeWork(Work::start_scan));
}

auto Index::Imp::done(database::ElementMap&& elements) noexcept -> void
{
    auto alloc = alloc::Strategy{get_allocator()};  // TODO
    auto& db = parent_.db_;
    const auto& index = parent_.db_key_;
    db.SubchainAddElements(log_, index, elements, alloc);
    last_indexed_ = parent_.db_.SubchainLastIndexed(index);
    parent_.element_cache_.lock()->Add(std::move(elements));
}

auto Index::Imp::forward_to_next(Message&& msg) noexcept -> void
{
    to_rescan_.SendDeferred(std::move(msg));
}

auto Index::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    to_rescan_.SendDeferred(std::move(in));
}

auto Index::Imp::process_filter(
    Message&& in,
    block::Position&&,
    allocator_type) noexcept -> void
{
    to_rescan_.SendDeferred(std::move(in));
}

auto Index::Imp::process_key(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    const auto body = in.Payload();

    assert_true(7_uz < body.size());

    using namespace crypto;
    const auto target = deserialize(body.subspan(1_uz, 3_uz));

    if (base_chain(target) != parent_.chain_) { return; }  // TODO

    const auto owner = api_.Factory().NymIDFromHash(body[4].Bytes());

    if (owner != parent_.owner_) { return; }

    const auto id = api_.Factory().IdentifierFromHash(body[5].Bytes());

    if (id != parent_.id_) { return; }

    const auto subchain = body[6].as<crypto::Subchain>();

    if (subchain != parent_.subchain_) { return; }

    do_work(monotonic);
}

auto Index::Imp::work(allocator_type monotonic) noexcept -> bool
{
    if (State::reorg == state()) { return false; }

    const auto need = need_index(last_indexed_);

    if (need.has_value()) {
        process(last_indexed_, need.value(), monotonic);
        check_mempool(monotonic);
    }

    return Job::work(monotonic);
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Index::Index(std::shared_ptr<Imp>&& imp) noexcept
    : imp_(std::move(imp))
{
    assert_false(nullptr == imp_);
}

Index::Index(Index&& rhs) noexcept
    : Index(std::move(rhs.imp_))
{
}

auto Index::Init() noexcept -> void
{
    imp_->Init(imp_);
    imp_.reset();
}

Index::~Index() = default;
}  // namespace opentxs::blockchain::node::wallet
