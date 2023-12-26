// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/otx/Types.internal.hpp"  // IWYU pragma: associated

#include <ConsensusEnums.pb.h>
#include <OTXEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/otx/ConsensusType.hpp"      // IWYU pragma: keep
#include "opentxs/otx/LastReplyStatus.hpp"    // IWYU pragma: keep
#include "opentxs/otx/PushType.hpp"           // IWYU pragma: keep
#include "opentxs/otx/ServerReplyType.hpp"    // IWYU pragma: keep
#include "opentxs/otx/ServerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"

namespace opentxs::otx
{
using ConsensusTypeMap =
    frozen::unordered_map<ConsensusType, proto::ConsensusType, 4>;
using ConsensusTypeReverseMap =
    frozen::unordered_map<proto::ConsensusType, ConsensusType, 4>;
using LastReplyStatusMap =
    frozen::unordered_map<LastReplyStatus, proto::LastReplyStatus, 6>;
using LastReplyStatusReverseMap =
    frozen::unordered_map<proto::LastReplyStatus, LastReplyStatus, 6>;
using PushTypeMap = frozen::unordered_map<PushType, proto::OTXPushType, 4>;
using PushTypeReverseMap =
    frozen::unordered_map<proto::OTXPushType, PushType, 4>;
using ServerReplyTypeMap =
    frozen::unordered_map<ServerReplyType, proto::ServerReplyType, 3>;
using ServerReplyTypeReverseMap =
    frozen::unordered_map<proto::ServerReplyType, ServerReplyType, 3>;
using ServerRequestTypeMap =
    frozen::unordered_map<ServerRequestType, proto::ServerRequestType, 2>;
using ServerRequestTypeReverseMap =
    frozen::unordered_map<proto::ServerRequestType, ServerRequestType, 2>;

auto consensustype_map() noexcept -> const ConsensusTypeMap&;
auto lastreplystatus_map() noexcept -> const LastReplyStatusMap&;
auto otxpushtype_map() noexcept -> const PushTypeMap&;
auto serverreplytype_map() noexcept -> const ServerReplyTypeMap&;
auto serverrequesttype_map() noexcept -> const ServerRequestTypeMap&;
}  // namespace opentxs::otx

namespace opentxs::otx
{
auto consensustype_map() noexcept -> const ConsensusTypeMap&
{
    using enum ConsensusType;
    using enum proto::ConsensusType;
    static constexpr auto map = ConsensusTypeMap{
        {Error, CONSENSUSTYPE_ERROR},
        {Server, CONSENSUSTYPE_SERVER},
        {Client, CONSENSUSTYPE_CLIENT},
        {Peer, CONSENSUSTYPE_PEER},
    };

    return map;
}

auto lastreplystatus_map() noexcept -> const LastReplyStatusMap&
{
    using enum LastReplyStatus;
    using enum proto::LastReplyStatus;
    static constexpr auto map = LastReplyStatusMap{
        {Invalid, LASTREPLYSTATUS_INVALID},
        {None, LASTREPLYSTATUS_NONE},
        {MessageSuccess, LASTREPLYSTATUS_MESSAGESUCCESS},
        {MessageFailed, LASTREPLYSTATUS_MESSAGEFAILED},
        {Unknown, LASTREPLYSTATUS_UNKNOWN},
        {NotSent, LASTREPLYSTATUS_NOTSENT},
    };

    return map;
}

auto otxpushtype_map() noexcept -> const PushTypeMap&
{
    using enum PushType;
    using enum proto::OTXPushType;
    static constexpr auto map = PushTypeMap{
        {Error, OTXPUSH_ERROR},
        {Nymbox, OTXPUSH_NYMBOX},
        {Inbox, OTXPUSH_INBOX},
        {Outbox, OTXPUSH_OUTBOX},
    };

    return map;
}

auto serverreplytype_map() noexcept -> const ServerReplyTypeMap&
{
    using enum ServerReplyType;
    using enum proto::ServerReplyType;
    static constexpr auto map = ServerReplyTypeMap{
        {Error, SERVERREPLY_ERROR},
        {Activate, SERVERREPLY_ACTIVATE},
        {Push, SERVERREPLY_PUSH},
    };

    return map;
}

auto serverrequesttype_map() noexcept -> const ServerRequestTypeMap&
{
    using enum ServerRequestType;
    using enum proto::ServerRequestType;
    static constexpr auto map = ServerRequestTypeMap{
        {Error, SERVERREQUEST_ERROR},
        {Activate, SERVERREQUEST_ACTIVATE},
    };

    return map;
}
}  // namespace opentxs::otx

namespace opentxs::otx
{
auto translate(ConsensusType in) noexcept -> proto::ConsensusType
{
    try {
        return consensustype_map().at(in);
    } catch (...) {
        return proto::CONSENSUSTYPE_ERROR;
    }
}

auto translate(LastReplyStatus in) noexcept -> proto::LastReplyStatus
{
    try {
        return lastreplystatus_map().at(in);
    } catch (...) {
        return proto::LASTREPLYSTATUS_INVALID;
    }
}

auto translate(PushType in) noexcept -> proto::OTXPushType
{
    try {
        return otxpushtype_map().at(in);
    } catch (...) {
        return proto::OTXPUSH_ERROR;
    }
}

auto translate(ServerReplyType in) noexcept -> proto::ServerReplyType
{
    try {
        return serverreplytype_map().at(in);
    } catch (...) {
        return proto::SERVERREPLY_ERROR;
    }
}

auto translate(ServerRequestType in) noexcept -> proto::ServerRequestType
{
    try {
        return serverrequesttype_map().at(in);
    } catch (...) {
        return proto::SERVERREQUEST_ERROR;
    }
}

auto translate(const proto::ConsensusType in) noexcept -> ConsensusType
{
    static const auto map = frozen::invert_unordered_map(consensustype_map());

    try {
        return map.at(in);
    } catch (...) {
        return ConsensusType::Error;
    }
}

auto translate(const proto::LastReplyStatus in) noexcept -> LastReplyStatus
{
    static const auto map = frozen::invert_unordered_map(lastreplystatus_map());

    try {
        return map.at(in);
    } catch (...) {
        return LastReplyStatus::Invalid;
    }
}

auto translate(const proto::OTXPushType in) noexcept -> PushType
{
    static const auto map = frozen::invert_unordered_map(otxpushtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return PushType::Error;
    }
}

auto translate(const proto::ServerReplyType in) noexcept -> ServerReplyType
{
    static const auto map = frozen::invert_unordered_map(serverreplytype_map());

    try {
        return map.at(in);
    } catch (...) {
        return ServerReplyType::Error;
    }
}

auto translate(const proto::ServerRequestType in) noexcept -> ServerRequestType
{
    static const auto map =
        frozen::invert_unordered_map(serverrequesttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return ServerRequestType::Error;
    }
}
}  // namespace opentxs::otx
