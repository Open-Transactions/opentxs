// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::otdht::MessageType

#include "opentxs/network/otdht/Base.hpp"  // IWYU pragma: associated

#include <P2PBlockchainHello.pb.h>
#include <P2PBlockchainSync.pb.h>
#include <boost/endian/buffers.hpp>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/network/otdht/Acknowledgement.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/MessageType.hpp"
#include "opentxs/network/otdht/PublishContract.hpp"       // IWYU pragma: keep
#include "opentxs/network/otdht/PublishContractReply.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/PushTransaction.hpp"       // IWYU pragma: keep
#include "opentxs/network/otdht/PushTransactionReply.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Query.hpp"                 // IWYU pragma: keep
#include "opentxs/network/otdht/QueryContract.hpp"         // IWYU pragma: keep
#include "opentxs/network/otdht/QueryContractReply.hpp"    // IWYU pragma: keep
#include "opentxs/network/otdht/Request.hpp"               // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::network::otdht
{
using LocalType = Base::Imp::LocalType;
using RemoteType = Base::Imp::RemoteType;
using ForwardMap = frozen::unordered_map<LocalType, RemoteType, 11>;
using ReverseMap = frozen::unordered_map<RemoteType, LocalType, 8>;

auto MessageToWork() noexcept -> const ForwardMap&;
auto MessageToWork() noexcept -> const ForwardMap&
{
    using enum MessageType;
    using enum WorkType;
    static constexpr auto data = ForwardMap{
        {sync_request, P2PBlockchainSyncRequest},
        {sync_ack, P2PBlockchainSyncAck},
        {sync_reply, P2PBlockchainSyncReply},
        {new_block_header, P2PBlockchainNewBlock},
        {query, P2PBlockchainSyncQuery},
        {publish_contract, P2PPublishContract},
        {publish_ack, P2PResponse},
        {contract_query, P2PQueryContract},
        {contract, P2PResponse},
        {pushtx, P2PPushTransaction},
        {pushtx_reply, P2PResponse},
    };

    return data;
}

auto WorkToMessage() noexcept -> const ReverseMap&;
auto WorkToMessage() noexcept -> const ReverseMap&
{
    auto items = std::array<std::pair<RemoteType, LocalType>, 8_uz>{};

    auto i = 0_uz;
    for (const auto& [key, value] : MessageToWork()) {
        if (value != WorkType::P2PResponse) { items[i++] = {value, key}; }
    }

    static const auto map = ReverseMap(items);

    return map;
}
}  // namespace opentxs::network::otdht

namespace opentxs::network::otdht
{
Base::Imp::Imp(
    VersionNumber version,
    MessageType type,
    StateData state,
    std::string_view endpoint,
    SyncData blocks) noexcept
    : version_(version)
    , type_(type)
    , state_(std::move(state))
    , endpoint_(endpoint)
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
            auto output = proto::P2PBlockchainHello{};
            output.set_version(hello_version_);

            for (const auto& state : state_) {
                if (false == state.Serialize(*output.add_state())) {
                    throw std::runtime_error{""};
                }
            }

            return output;
        }();
        out.Internal().AddFrame(hello);
        out.AddFrame(endpoint_.data(), endpoint_.size());

        for (const auto& block : blocks_) {
            const auto data = [&] {
                auto output = proto::P2PBlockchainSync{};

                if (false == block.Serialize(output)) {
                    throw std::runtime_error{""};
                }

                return output;
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
}  // namespace opentxs::network::otdht
