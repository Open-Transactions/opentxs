// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain

#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Index::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::index,
          alloc,
          {
              {CString{
                   parent->api_.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
          },
          {
              {parent->to_index_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->to_rescan_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_scan_endpoint_, Direction::Connect},
               }},
          })
    , to_rescan_(pipeline_.Internal().ExtraSocket(1))
    , to_scan_(pipeline_.Internal().ExtraSocket(2))
    , last_indexed_(std::nullopt)
{
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
            log_(OT_PRETTY_CLASS())(name_)(" re-indexing ")(
                name_)(" due to processed block ")(position)
                .Flush();
        }
    }

    do_work(monotonic);
    to_rescan_.SendDeferred(std::move(msg), __FILE__, __LINE__);
}

auto Index::Imp::do_startup_internal(allocator_type monotonic) noexcept -> void
{
    last_indexed_ = parent_.db_.SubchainLastIndexed(parent_.db_key_);
    do_work(monotonic);
    log_(OT_PRETTY_CLASS())(name_)(" notifying scan task to begin work")
        .Flush();
    to_scan_.SendDeferred(MakeWork(Work::start_scan), __FILE__, __LINE__);
}

auto Index::Imp::done(database::ElementMap&& elements) noexcept -> void
{
    auto& db = parent_.db_;
    const auto& index = parent_.db_key_;
    db.SubchainAddElements(index, elements);
    last_indexed_ = parent_.db_.SubchainLastIndexed(index);
    parent_.element_cache_.lock()->Add(std::move(elements));
}

auto Index::Imp::forward_to_next(Message&& msg) noexcept -> void
{
    to_rescan_.Send(std::move(msg), __FILE__, __LINE__);
}

auto Index::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    to_rescan_.Send(std::move(in), __FILE__, __LINE__);
}

auto Index::Imp::process_filter(
    Message&& in,
    block::Position&&,
    allocator_type) noexcept -> void
{
    to_rescan_.Send(std::move(in), __FILE__, __LINE__);
}

auto Index::Imp::process_key(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(4u < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != parent_.chain_) { return; }

    const auto owner = api_.Factory().NymIDFromHash(body.at(2).Bytes());

    if (owner != parent_.owner_) { return; }

    const auto id = api_.Factory().IdentifierFromHash(body.at(3).Bytes());

    if (id != parent_.id_) { return; }

    const auto subchain = body.at(4).as<crypto::Subchain>();

    if (subchain != parent_.subchain_) { return; }

    do_work(monotonic);
}

auto Index::Imp::work(allocator_type monotonic) noexcept -> bool
{
    if (State::reorg == state()) { return false; }

    const auto need = need_index(last_indexed_);

    if (need.has_value()) { process(last_indexed_, need.value(), monotonic); }

    return Job::work(monotonic);
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Index::Index(boost::shared_ptr<Imp>&& imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(imp_);
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
