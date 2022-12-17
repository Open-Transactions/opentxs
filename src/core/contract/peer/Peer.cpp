// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/Peer.hpp"  // IWYU pragma: associated

#include <PairEvent.pb.h>
#include <PeerEnums.pb.h>
#include <ZMQEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "internal/serialization/protobuf/Proto.tpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/PeerObjectType.hpp"   // IWYU pragma: keep
#include "opentxs/core/contract/peer/PeerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/SecretType.hpp"       // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::internal
{
PairEvent::PairEvent(const ReadView view)
    : PairEvent(proto::Factory<proto::PairEvent>(view))
{
}

PairEvent::PairEvent(const proto::PairEvent& serialized)
    : PairEvent(
          serialized.version(),
          translate(serialized.type()),
          serialized.issuer())
{
}

PairEvent::PairEvent(
    const std::uint32_t version,
    const PairEventType type,
    const UnallocatedCString& issuer)
    : version_(version)
    , type_(type)
    , issuer_(issuer)
{
}
}  // namespace opentxs::contract::peer::internal

namespace opentxs::contract::peer
{
using ConnectionInfoTypeMap =
    frozen::unordered_map<ConnectionInfoType, proto::ConnectionInfoType, 7>;
using ConnectionInfoTypeReverseMap =
    frozen::unordered_map<proto::ConnectionInfoType, ConnectionInfoType, 7>;
using PairEventTypeMap =
    frozen::unordered_map<internal::PairEventType, proto::PairEventType, 3>;
using PairEventTypeReverseMap =
    frozen::unordered_map<proto::PairEventType, internal::PairEventType, 3>;
using PeerObjectTypeMap =
    frozen::unordered_map<PeerObjectType, proto::PeerObjectType, 6>;
using PeerObjectTypeReverseMap =
    frozen::unordered_map<proto::PeerObjectType, PeerObjectType, 6>;
using PeerRequestTypeMap =
    frozen::unordered_map<PeerRequestType, proto::PeerRequestType, 8>;
using PeerRequestTypeReverseMap =
    frozen::unordered_map<proto::PeerRequestType, PeerRequestType, 8>;
using SecretTypeMap = frozen::unordered_map<SecretType, proto::SecretType, 2>;
using SecretTypeReverseMap =
    frozen::unordered_map<proto::SecretType, SecretType, 2>;

auto connectioninfotype_map() noexcept -> const ConnectionInfoTypeMap&;
auto paireventtype_map() noexcept -> const PairEventTypeMap&;
auto peerobjecttype_map() noexcept -> const PeerObjectTypeMap&;
auto peerrequesttype_map() noexcept -> const PeerRequestTypeMap&;
auto secrettype_map() noexcept -> const SecretTypeMap&;
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
auto connectioninfotype_map() noexcept -> const ConnectionInfoTypeMap&
{
    using enum ConnectionInfoType;
    using enum proto::ConnectionInfoType;
    static constexpr auto map = ConnectionInfoTypeMap{
        {Error, CONNECTIONINFO_ERROR},
        {Bitcoin, CONNECTIONINFO_BITCOIN},
        {BtcRpc, CONNECTIONINFO_BTCRPC},
        {BitMessage, CONNECTIONINFO_BITMESSAGE},
        {BitMessageRPC, CONNECTIONINFO_BITMESSAGERPC},
        {SSH, CONNECTIONINFO_SSH},
        {CJDNS, CONNECTIONINFO_CJDNS},
    };

    return map;
}

auto paireventtype_map() noexcept -> const PairEventTypeMap&
{
    using enum internal::PairEventType;
    using enum proto::PairEventType;
    static constexpr auto map = PairEventTypeMap{
        {Error, PAIREVENT_ERROR},
        {Rename, PAIREVENT_RENAME},
        {StoreSecret, PAIREVENT_STORESECRET},
    };

    return map;
}
auto peerobjecttype_map() noexcept -> const PeerObjectTypeMap&
{
    using enum PeerObjectType;
    using enum proto::PeerObjectType;
    static constexpr auto map = PeerObjectTypeMap{
        {Error, PEEROBJECT_ERROR},
        {Message, PEEROBJECT_MESSAGE},
        {Request, PEEROBJECT_REQUEST},
        {Response, PEEROBJECT_RESPONSE},
        {Payment, PEEROBJECT_PAYMENT},
        {Cash, PEEROBJECT_CASH},
    };

    return map;
}

auto peerrequesttype_map() noexcept -> const PeerRequestTypeMap&
{
    using enum PeerRequestType;
    using enum proto::PeerRequestType;
    static constexpr auto map = PeerRequestTypeMap{
        {Error, PEERREQUEST_ERROR},
        {Bailment, PEERREQUEST_BAILMENT},
        {OutBailment, PEERREQUEST_OUTBAILMENT},
        {PendingBailment, PEERREQUEST_PENDINGBAILMENT},
        {ConnectionInfo, PEERREQUEST_CONNECTIONINFO},
        {StoreSecret, PEERREQUEST_STORESECRET},
        {VerificationOffer, PEERREQUEST_VERIFICATIONOFFER},
        {Faucet, PEERREQUEST_FAUCET},
    };

    return map;
}

auto secrettype_map() noexcept -> const SecretTypeMap&
{
    using enum SecretType;
    using enum proto::SecretType;
    static constexpr auto map = SecretTypeMap{
        {SecretType::Error, proto::SECRETTYPE_ERROR},
        {SecretType::Bip39, proto::SECRETTYPE_BIP39},
    };

    return map;
}
}  // namespace opentxs::contract::peer

namespace opentxs
{
auto translate(const contract::peer::ConnectionInfoType in) noexcept
    -> proto::ConnectionInfoType
{
    try {
        return contract::peer::connectioninfotype_map().at(in);
    } catch (...) {
        return proto::CONNECTIONINFO_ERROR;
    }
}

auto translate(const contract::peer::internal::PairEventType in) noexcept
    -> proto::PairEventType
{
    try {
        return contract::peer::paireventtype_map().at(in);
    } catch (...) {
        return proto::PAIREVENT_ERROR;
    }
}

auto translate(const contract::peer::PeerObjectType in) noexcept
    -> proto::PeerObjectType
{
    try {
        return contract::peer::peerobjecttype_map().at(in);
    } catch (...) {
        return proto::PEEROBJECT_ERROR;
    }
}

auto translate(const contract::peer::PeerRequestType in) noexcept
    -> proto::PeerRequestType
{
    try {
        return contract::peer::peerrequesttype_map().at(in);
    } catch (...) {
        return proto::PEERREQUEST_ERROR;
    }
}

auto translate(const contract::peer::SecretType in) noexcept
    -> proto::SecretType
{
    try {
        return contract::peer::secrettype_map().at(in);
    } catch (...) {
        return proto::SECRETTYPE_ERROR;
    }
}

auto translate(const proto::ConnectionInfoType in) noexcept
    -> contract::peer::ConnectionInfoType
{
    static const auto map =
        frozen::invert_unordered_map(contract::peer::connectioninfotype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::ConnectionInfoType::Error;
    }
}

auto translate(const proto::PairEventType in) noexcept
    -> contract::peer::internal::PairEventType
{
    static const auto map =
        frozen::invert_unordered_map(contract::peer::paireventtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::internal::PairEventType::Error;
    }
}

auto translate(const proto::PeerObjectType in) noexcept
    -> contract::peer::PeerObjectType
{
    static const auto map =
        frozen::invert_unordered_map(contract::peer::peerobjecttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::PeerObjectType::Error;
    }
}

auto translate(const proto::PeerRequestType in) noexcept
    -> contract::peer::PeerRequestType
{
    static const auto map =
        frozen::invert_unordered_map(contract::peer::peerrequesttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::PeerRequestType::Error;
    }
}

auto translate(const proto::SecretType in) noexcept
    -> contract::peer::SecretType
{
    static const auto map =
        frozen::invert_unordered_map(contract::peer::secrettype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::SecretType::Error;
    }
}
}  // namespace opentxs
