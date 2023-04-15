// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/OTX.hpp"  // IWYU pragma: associated

#include <ConsensusEnums.pb.h>
#include <OTXEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/otx/ConsensusType.hpp"      // IWYU pragma: keep
#include "opentxs/otx/LastReplyStatus.hpp"    // IWYU pragma: keep
#include "opentxs/otx/OTXPushType.hpp"        // IWYU pragma: keep
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
using OTXPushTypeMap =
    frozen::unordered_map<OTXPushType, proto::OTXPushType, 4>;
using OTXPushTypeReverseMap =
    frozen::unordered_map<proto::OTXPushType, OTXPushType, 4>;
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
auto otxpushtype_map() noexcept -> const OTXPushTypeMap&;
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

auto otxpushtype_map() noexcept -> const OTXPushTypeMap&
{
    using enum OTXPushType;
    using enum proto::OTXPushType;
    static constexpr auto map = OTXPushTypeMap{
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
    static const auto map =
        frozen::invert_unordered_map(otx::consensustype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::ConsensusType::Error;
    }
}

auto translate(const proto::LastReplyStatus in) noexcept -> otx::LastReplyStatus
{
    static const auto map =
        frozen::invert_unordered_map(otx::lastreplystatus_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::LastReplyStatus::Invalid;
    }
}

auto translate(const proto::OTXPushType in) noexcept -> otx::OTXPushType
{
    static const auto map =
        frozen::invert_unordered_map(otx::otxpushtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::OTXPushType::Error;
    }
}

auto translate(const proto::ServerReplyType in) noexcept -> otx::ServerReplyType
{
    static const auto map =
        frozen::invert_unordered_map(otx::serverreplytype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::ServerReplyType::Error;
    }
}

auto translate(const proto::ServerRequestType in) noexcept
    -> otx::ServerRequestType
{
    static const auto map =
        frozen::invert_unordered_map(otx::serverrequesttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::ServerRequestType::Error;
    }
}
}  // namespace opentxs
