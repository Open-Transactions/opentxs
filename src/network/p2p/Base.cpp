// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/network/p2p/Base.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <robin_hood.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/network/p2p/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/p2p/Base.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"  // IWYU pragma: keep
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/Data.hpp"  // IWYU pragma: keep
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/PublishContract.hpp"       // IWYU pragma: keep
#include "opentxs/network/p2p/PublishContractReply.hpp"  // IWYU pragma: keep
#include "opentxs/network/p2p/PushTransaction.hpp"       // IWYU pragma: keep
#include "opentxs/network/p2p/PushTransactionReply.hpp"  // IWYU pragma: keep
#include "opentxs/network/p2p/Query.hpp"                 // IWYU pragma: keep
#include "opentxs/network/p2p/QueryContract.hpp"         // IWYU pragma: keep
#include "opentxs/network/p2p/QueryContractReply.hpp"    // IWYU pragma: keep
#include "opentxs/network/p2p/Request.hpp"               // IWYU pragma: keep
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainP2PHello.pb.h"
#include "serialization/protobuf/BlockchainP2PSync.pb.h"
#include "util/Container.hpp"

namespace opentxs::network::p2p
{
using LocalType = Base::Imp::LocalType;
using RemoteType = Base::Imp::RemoteType;
using ForwardMap = robin_hood::unordered_flat_map<LocalType, RemoteType>;
using ReverseMap = robin_hood::unordered_flat_map<RemoteType, LocalType>;

auto MessageToWork() noexcept -> const ForwardMap&;
auto MessageToWork() noexcept -> const ForwardMap&
{
    static const auto data = ForwardMap{
        {MessageType::sync_request, WorkType::P2PBlockchainSyncRequest},
        {MessageType::sync_ack, WorkType::P2PBlockchainSyncAck},
        {MessageType::sync_reply, WorkType::P2PBlockchainSyncReply},
        {MessageType::new_block_header, WorkType::P2PBlockchainNewBlock},
        {MessageType::query, WorkType::P2PBlockchainSyncQuery},
        {MessageType::publish_contract, WorkType::P2PPublishContract},
        {MessageType::publish_ack, WorkType::P2PResponse},
        {MessageType::contract_query, WorkType::P2PQueryContract},
        {MessageType::contract, WorkType::P2PResponse},
        {MessageType::pushtx, WorkType::P2PPushTransaction},
        {MessageType::pushtx_reply, WorkType::P2PResponse},
    };

    return data;
}

auto WorkToMessage() noexcept -> const ReverseMap&;
auto WorkToMessage() noexcept -> const ReverseMap&
{
    static const auto data = [] {
        auto out = reverse_arbitrary_map<LocalType, RemoteType, ReverseMap>(
            MessageToWork());
        out.erase(WorkType::P2PResponse);

        return out;
    }();

    return data;
}
}  // namespace opentxs::network::p2p

namespace opentxs::network::p2p
{
Base::Imp::Imp(
    VersionNumber version,
    MessageType type,
    UnallocatedVector<State> state,
    UnallocatedCString endpoint,
    UnallocatedVector<Block> blocks) noexcept
    : version_(version)
    , type_(type)
    , state_(std::move(state))
    , endpoint_(std::move(endpoint))
    , blocks_(std::move(blocks))
{
}

Base::Imp::Imp(MessageType type) noexcept
    : Imp(default_version_, type, {}, {}, {})
{
}

Base::Imp::Imp() noexcept
    : Imp(MessageType::error)
{
}

Base::Base(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Base::Base() noexcept
    : Base(std::make_unique<Imp>().release())
{
}

auto Base::Imp::asAcknowledgement() const noexcept -> const Acknowledgement&
{
    static const auto blank = factory::BlockchainSyncAcknowledgement();

    return blank;
}

auto Base::Imp::asData() const noexcept -> const Data&
{
    static const auto blank = factory::BlockchainSyncData();

    return blank;
}

auto Base::Imp::asPublishContract() const noexcept -> const PublishContract&
{
    static const auto blank = factory::BlockchainSyncPublishContract();

    return blank;
}

auto Base::Imp::asPublishContractReply() const noexcept
    -> const PublishContractReply&
{
    static const auto blank = factory::BlockchainSyncPublishContractReply();

    return blank;
}

auto Base::Imp::asPushTransaction() const noexcept -> const PushTransaction&
{
    static const auto blank = factory::BlockchainSyncPushTransaction();

    return blank;
}

auto Base::Imp::asPushTransactionReply() const noexcept
    -> const PushTransactionReply&
{
    static const auto blank = factory::BlockchainSyncPushTransactionReply();

    return blank;
}

auto Base::Imp::asQuery() const noexcept -> const Query&
{
    static const auto blank = factory::BlockchainSyncQuery();

    return blank;
}

auto Base::Imp::asQueryContract() const noexcept -> const QueryContract&
{
    static const auto blank = factory::BlockchainSyncQueryContract();

    return blank;
}

auto Base::Imp::asQueryContractReply() const noexcept
    -> const QueryContractReply&
{
    static const auto blank = factory::BlockchainSyncQueryContractReply();

    return blank;
}

auto Base::Imp::asRequest() const noexcept -> const Request&
{
    static const auto blank = factory::BlockchainSyncRequest();

    return blank;
}

auto Base::Imp::serialize(zeromq::Message& out) const noexcept -> bool
{
    if (false == serialize_type(out)) { return false; }

    try {
        const auto hello = [&] {
            auto out = proto::BlockchainP2PHello{};
            out.set_version(hello_version_);

            for (const auto& state : state_) {
                if (false == state.Serialize(*out.add_state())) {
                    throw std::runtime_error{""};
                }
            }

            return out;
        }();
        out.Internal().AddFrame(hello);
        out.AddFrame(endpoint_.data(), endpoint_.size());

        for (const auto& block : blocks_) {
            const auto data = [&] {
                auto out = proto::BlockchainP2PSync{};

                if (false == block.Serialize(out)) {
                    throw std::runtime_error{""};
                }

                return out;
            }();
            out.Internal().AddFrame(data);
        }
    } catch (...) {

        return false;
    }

    return true;
}

auto Base::Imp::serialize_type(zeromq::Message& out) const noexcept -> bool
{
    if (MessageType::error == type_) {
        LogError()(OT_PRETTY_CLASS())("Invalid type").Flush();

        return false;
    }

    if (0u == out.size()) {
        out.AddFrame();
    } else if (0u != out.at(out.size() - 1u).size()) {
        // NOTE supplied message should either be empty or else have header
        // frames followed by an empty delimiter frame.
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        return false;
    }

    using Buffer = boost::endian::little_uint16_buf_t;

    static_assert(sizeof(Buffer) == sizeof(decltype(translate(type_))));

    out.AddFrame(Buffer{static_cast<std::uint16_t>(translate(type_))});

    return true;
}

auto Base::Imp::translate(const LocalType in) noexcept -> RemoteType
{
    return MessageToWork().at(in);
}

auto Base::Imp::translate(const RemoteType in) noexcept -> LocalType
{
    return WorkToMessage().at(in);
}

auto Base::asAcknowledgement() const noexcept -> const Acknowledgement&
{
    return imp_->asAcknowledgement();
}

auto Base::asData() const noexcept -> const Data& { return imp_->asData(); }

auto Base::asPublishContract() const noexcept -> const PublishContract&
{
    return imp_->asPublishContract();
}

auto Base::asPublishContractReply() const noexcept
    -> const PublishContractReply&
{
    return imp_->asPublishContractReply();
}

auto Base::asPushTransaction() const noexcept -> const PushTransaction&
{
    return imp_->asPushTransaction();
}

auto Base::asPushTransactionReply() const noexcept
    -> const PushTransactionReply&
{
    return imp_->asPushTransactionReply();
}

auto Base::asQuery() const noexcept -> const Query& { return imp_->asQuery(); }

auto Base::asQueryContract() const noexcept -> const QueryContract&
{
    return imp_->asQueryContract();
}

auto Base::asQueryContractReply() const noexcept -> const QueryContractReply&
{
    return imp_->asQueryContractReply();
}

auto Base::asRequest() const noexcept -> const Request&
{
    return imp_->asRequest();
}

auto Base::Serialize(zeromq::Message& out) const noexcept -> bool
{
    return imp_->serialize(out);
}

auto Base::Type() const noexcept -> MessageType { return imp_->type_; }

auto Base::Version() const noexcept -> VersionNumber { return imp_->version_; }

Base::~Base()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::p2p
