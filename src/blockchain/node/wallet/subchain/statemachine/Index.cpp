// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::wallet
{
Index::Imp::Imp(
    const SubchainStateData& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          CString{"index", alloc},
          alloc,
          {
              {parent.shutdown_endpoint_, Direction::Connect},
              {CString{
                   parent.api_.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
          },
          {
              {parent.to_index_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent.to_rescan_endpoint_, Direction::Connect},
               }},
          })
    , to_rescan_(pipeline_.Internal().ExtraSocket(0))
    , last_indexed_(std::nullopt)
{
}

auto Index::Imp::do_startup() noexcept -> void
{
    last_indexed_ = parent_.db_.SubchainLastIndexed(parent_.db_key_);
    do_work();
    log_(OT_PRETTY_CLASS())(parent_.name_)(" notifying scan task to begin work")
        .Flush();
}

auto Index::Imp::done(
    const node::internal::WalletDatabase::ElementMap& elements) noexcept -> void
{
    auto& db = parent_.db_;
    const auto& index = parent_.db_key_;
    db.SubchainAddElements(index, elements);
    last_indexed_ = parent_.db_.SubchainLastIndexed(index);
}

auto Index::Imp::ProcessReorg(const block::Position& parent) noexcept -> void
{
    // NOTE no action required
}

auto Index::Imp::process_key(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4u < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != parent_.chain_) { return; }

    const auto owner = parent_.api_.Factory().NymID(body.at(2));

    if (owner != parent_.owner_) { return; }

    const auto id = parent_.api_.Factory().Identifier(body.at(3));

    if (id != parent_.id_) { return; }

    const auto subchain = body.at(4).as<crypto::Subchain>();

    if (subchain != parent_.subchain_) { return; }

    do_work();
}

auto Index::Imp::process_update(Message&& msg) noexcept -> void
{
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(parent_.api_, msg, clean, dirty);

    for (const auto& [type, position] : clean) {
        if (ScanState::processed == type) {
            log_(OT_PRETTY_CLASS())(parent_.name_)(" re-indexing ")(
                parent_.name_)(" due to processed block ")(
                opentxs::print(position))
                .Flush();
        }
    }

    do_work();
    to_rescan_.SendDeferred(std::move(msg));
}

auto Index::Imp::work() noexcept -> bool
{
    const auto need = need_index(last_indexed_);

    if (need.has_value()) { process(last_indexed_, need.value()); }

    return false;
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Index::Index(boost::shared_ptr<Imp>&& imp) noexcept
    : imp_(std::move(imp))
{
    imp_->Init(imp_);
}

Index::Index(Index&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
}

auto Index::ChangeState(const State state) noexcept -> bool
{
    return imp_->ChangeState(state);
}

auto Index::ProcessReorg(const block::Position& parent) noexcept -> void
{
    imp_->ProcessReorg(parent);
}

Index::~Index()
{
    if (imp_) { imp_->Shutdown(); }
}
}  // namespace opentxs::blockchain::node::wallet
