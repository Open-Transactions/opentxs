// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "1_Internal.hpp"        // IWYU pragma: associated
#include "internal/otx/OTX.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/otx/ConsensusType.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/otx/OTXPushType.hpp"
#include "opentxs/otx/ServerReplyType.hpp"
#include "opentxs/otx/ServerRequestType.hpp"
#include "serialization/protobuf/ConsensusEnums.pb.h"
#include "serialization/protobuf/OTXEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::otx
{
using ConsensusTypeMap =
    robin_hood::unordered_flat_map<ConsensusType, proto::ConsensusType>;
using ConsensusTypeReverseMap =
    robin_hood::unordered_flat_map<proto::ConsensusType, ConsensusType>;
using LastReplyStatusMap =
    robin_hood::unordered_flat_map<LastReplyStatus, proto::LastReplyStatus>;
using LastReplyStatusReverseMap =
    robin_hood::unordered_flat_map<proto::LastReplyStatus, LastReplyStatus>;
using OTXPushTypeMap =
    robin_hood::unordered_flat_map<OTXPushType, proto::OTXPushType>;
using OTXPushTypeReverseMap =
    robin_hood::unordered_flat_map<proto::OTXPushType, OTXPushType>;
using ServerReplyTypeMap =
    robin_hood::unordered_flat_map<ServerReplyType, proto::ServerReplyType>;
using ServerReplyTypeReverseMap =
    robin_hood::unordered_flat_map<proto::ServerReplyType, ServerReplyType>;
using ServerRequestTypeMap =
    robin_hood::unordered_flat_map<ServerRequestType, proto::ServerRequestType>;
using ServerRequestTypeReverseMap =
    robin_hood::unordered_flat_map<proto::ServerRequestType, ServerRequestType>;

auto consensustype_map() noexcept -> const ConsensusTypeMap&;
auto lastreplystatus_map() noexcept -> const LastReplyStatusMap&;
auto otxpushtype_map() noexcept -> const OTXPushTypeMap&;
auto serverreplytype_map() noexcept -> const ServerReplyTypeMap&;
auto serverrequesttype_map() noexcept -> const ServerRequestTypeMap&;
}  // namespace opentxs::otx

namespace opentxs::otx
{
auto consensustype_map() noexcept -> const ConsensusTypeMap&
{
    static const auto map = ConsensusTypeMap{
        {ConsensusType::Error, proto::CONSENSUSTYPE_ERROR},
        {ConsensusType::Server, proto::CONSENSUSTYPE_SERVER},
        {ConsensusType::Client, proto::CONSENSUSTYPE_CLIENT},
        {ConsensusType::Peer, proto::CONSENSUSTYPE_PEER},
    };

    return map;
}

auto lastreplystatus_map() noexcept -> const LastReplyStatusMap&
{
    static const auto map = LastReplyStatusMap{
        {LastReplyStatus::Invalid, proto::LASTREPLYSTATUS_INVALID},
        {LastReplyStatus::None, proto::LASTREPLYSTATUS_NONE},
        {LastReplyStatus::MessageSuccess,
         proto::LASTREPLYSTATUS_MESSAGESUCCESS},
        {LastReplyStatus::MessageFailed, proto::LASTREPLYSTATUS_MESSAGEFAILED},
        {LastReplyStatus::Unknown, proto::LASTREPLYSTATUS_UNKNOWN},
        {LastReplyStatus::NotSent, proto::LASTREPLYSTATUS_NOTSENT},
    };

    return map;
}

auto otxpushtype_map() noexcept -> const OTXPushTypeMap&
{
    static const auto map = OTXPushTypeMap{
        {OTXPushType::Error, proto::OTXPUSH_ERROR},
        {OTXPushType::Nymbox, proto::OTXPUSH_NYMBOX},
        {OTXPushType::Inbox, proto::OTXPUSH_INBOX},
        {OTXPushType::Outbox, proto::OTXPUSH_OUTBOX},
    };

    return map;
}

auto serverreplytype_map() noexcept -> const ServerReplyTypeMap&
{
    static const auto map = ServerReplyTypeMap{
        {ServerReplyType::Error, proto::SERVERREPLY_ERROR},
        {ServerReplyType::Activate, proto::SERVERREPLY_ACTIVATE},
        {ServerReplyType::Push, proto::SERVERREPLY_PUSH},
    };

    return map;
}

auto serverrequesttype_map() noexcept -> const ServerRequestTypeMap&
{
    static const auto map = ServerRequestTypeMap{
        {ServerRequestType::Error, proto::SERVERREQUEST_ERROR},
        {ServerRequestType::Activate, proto::SERVERREQUEST_ACTIVATE},
    };

    return map;
}
}  // namespace opentxs::otx

namespace opentxs
{
auto translate(const otx::ConsensusType in) noexcept -> proto::ConsensusType
{
    try {
        return otx::consensustype_map().at(in);
    } catch (...) {
        return proto::CONSENSUSTYPE_ERROR;
    }
}

auto translate(const otx::LastReplyStatus in) noexcept -> proto::LastReplyStatus
{
    try {
        return otx::lastreplystatus_map().at(in);
    } catch (...) {
        return proto::LASTREPLYSTATUS_INVALID;
    }
}

auto translate(const otx::OTXPushType in) noexcept -> proto::OTXPushType
{
    try {
        return otx::otxpushtype_map().at(in);
    } catch (...) {
        return proto::OTXPUSH_ERROR;
    }
}

auto translate(const otx::ServerReplyType in) noexcept -> proto::ServerReplyType
{
    try {
        return otx::serverreplytype_map().at(in);
    } catch (...) {
        return proto::SERVERREPLY_ERROR;
    }
}

auto translate(const otx::ServerRequestType in) noexcept
    -> proto::ServerRequestType
{
    try {
        return otx::serverrequesttype_map().at(in);
    } catch (...) {
        return proto::SERVERREQUEST_ERROR;
    }
}

auto translate(const proto::ConsensusType in) noexcept -> otx::ConsensusType
{
    static const auto map = reverse_arbitrary_map<
        otx::ConsensusType,
        proto::ConsensusType,
        otx::ConsensusTypeReverseMap>(otx::consensustype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::ConsensusType::Error;
    }
}

auto translate(const proto::LastReplyStatus in) noexcept -> otx::LastReplyStatus
{
    static const auto map = reverse_arbitrary_map<
        otx::LastReplyStatus,
        proto::LastReplyStatus,
        otx::LastReplyStatusReverseMap>(otx::lastreplystatus_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::LastReplyStatus::Invalid;
    }
}

auto translate(const proto::OTXPushType in) noexcept -> otx::OTXPushType
{
    static const auto map = reverse_arbitrary_map<
        otx::OTXPushType,
        proto::OTXPushType,
        otx::OTXPushTypeReverseMap>(otx::otxpushtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::OTXPushType::Error;
    }
}

auto translate(const proto::ServerReplyType in) noexcept -> otx::ServerReplyType
{
    static const auto map = reverse_arbitrary_map<
        otx::ServerReplyType,
        proto::ServerReplyType,
        otx::ServerReplyTypeReverseMap>(otx::serverreplytype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::ServerReplyType::Error;
    }
}

auto translate(const proto::ServerRequestType in) noexcept
    -> otx::ServerRequestType
{
    static const auto map = reverse_arbitrary_map<
        otx::ServerRequestType,
        proto::ServerRequestType,
        otx::ServerRequestTypeReverseMap>(otx::serverrequesttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::ServerRequestType::Error;
    }
}
}  // namespace opentxs
