// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "internal/core/contract/peer/Peer.hpp"  // IWYU pragma: associated

#include <PairEvent.pb.h>
#include <PeerEnums.pb.h>
#include <ZMQEnums.pb.h>
#include <robin_hood.h>

#include "Proto.tpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/contract/peer/PeerObjectType.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/SecretType.hpp"
#include "util/Container.hpp"

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
using ConnectionInfoTypeMap = robin_hood::
    unordered_flat_map<ConnectionInfoType, proto::ConnectionInfoType>;
using ConnectionInfoTypeReverseMap = robin_hood::
    unordered_flat_map<proto::ConnectionInfoType, ConnectionInfoType>;
using PairEventTypeMap = robin_hood::
    unordered_flat_map<internal::PairEventType, proto::PairEventType>;
using PairEventTypeReverseMap = robin_hood::
    unordered_flat_map<proto::PairEventType, internal::PairEventType>;
using PeerObjectTypeMap =
    robin_hood::unordered_flat_map<PeerObjectType, proto::PeerObjectType>;
using PeerObjectTypeReverseMap =
    robin_hood::unordered_flat_map<proto::PeerObjectType, PeerObjectType>;
using PeerRequestTypeMap =
    robin_hood::unordered_flat_map<PeerRequestType, proto::PeerRequestType>;
using PeerRequestTypeReverseMap =
    robin_hood::unordered_flat_map<proto::PeerRequestType, PeerRequestType>;
using SecretTypeMap =
    robin_hood::unordered_flat_map<SecretType, proto::SecretType>;
using SecretTypeReverseMap =
    robin_hood::unordered_flat_map<proto::SecretType, SecretType>;

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
    static const auto map = ConnectionInfoTypeMap{
        {ConnectionInfoType::Error, proto::CONNECTIONINFO_ERROR},
        {ConnectionInfoType::Bitcoin, proto::CONNECTIONINFO_BITCOIN},
        {ConnectionInfoType::BtcRpc, proto::CONNECTIONINFO_BTCRPC},
        {ConnectionInfoType::BitMessage, proto::CONNECTIONINFO_BITMESSAGE},
        {ConnectionInfoType::BitMessageRPC,
         proto::CONNECTIONINFO_BITMESSAGERPC},
        {ConnectionInfoType::SSH, proto::CONNECTIONINFO_SSH},
        {ConnectionInfoType::CJDNS, proto::CONNECTIONINFO_CJDNS},
    };

    return map;
}

auto paireventtype_map() noexcept -> const PairEventTypeMap&
{
    static const auto map = PairEventTypeMap{
        {internal::PairEventType::Error, proto::PAIREVENT_ERROR},
        {internal::PairEventType::Rename, proto::PAIREVENT_RENAME},
        {internal::PairEventType::StoreSecret, proto::PAIREVENT_STORESECRET},
    };

    return map;
}
auto peerobjecttype_map() noexcept -> const PeerObjectTypeMap&
{
    static const auto map = PeerObjectTypeMap{
        {PeerObjectType::Error, proto::PEEROBJECT_ERROR},
        {PeerObjectType::Message, proto::PEEROBJECT_MESSAGE},
        {PeerObjectType::Request, proto::PEEROBJECT_REQUEST},
        {PeerObjectType::Response, proto::PEEROBJECT_RESPONSE},
        {PeerObjectType::Payment, proto::PEEROBJECT_PAYMENT},
        {PeerObjectType::Cash, proto::PEEROBJECT_CASH},
    };

    return map;
}

auto peerrequesttype_map() noexcept -> const PeerRequestTypeMap&
{
    static const auto map = PeerRequestTypeMap{
        {PeerRequestType::Error, proto::PEERREQUEST_ERROR},
        {PeerRequestType::Bailment, proto::PEERREQUEST_BAILMENT},
        {PeerRequestType::OutBailment, proto::PEERREQUEST_OUTBAILMENT},
        {PeerRequestType::PendingBailment, proto::PEERREQUEST_PENDINGBAILMENT},
        {PeerRequestType::ConnectionInfo, proto::PEERREQUEST_CONNECTIONINFO},
        {PeerRequestType::StoreSecret, proto::PEERREQUEST_STORESECRET},
        {PeerRequestType::VerificationOffer,
         proto::PEERREQUEST_VERIFICATIONOFFER},
        {PeerRequestType::Faucet, proto::PEERREQUEST_FAUCET},
    };

    return map;
}

auto secrettype_map() noexcept -> const SecretTypeMap&
{
    static const auto map = SecretTypeMap{
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
    static const auto map = reverse_arbitrary_map<
        contract::peer::ConnectionInfoType,
        proto::ConnectionInfoType,
        contract::peer::ConnectionInfoTypeReverseMap>(
        contract::peer::connectioninfotype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::ConnectionInfoType::Error;
    }
}

auto translate(const proto::PairEventType in) noexcept
    -> contract::peer::internal::PairEventType
{
    static const auto map = reverse_arbitrary_map<
        contract::peer::internal::PairEventType,
        proto::PairEventType,
        contract::peer::PairEventTypeReverseMap>(
        contract::peer::paireventtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::internal::PairEventType::Error;
    }
}

auto translate(const proto::PeerObjectType in) noexcept
    -> contract::peer::PeerObjectType
{
    static const auto map = reverse_arbitrary_map<
        contract::peer::PeerObjectType,
        proto::PeerObjectType,
        contract::peer::PeerObjectTypeReverseMap>(
        contract::peer::peerobjecttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::PeerObjectType::Error;
    }
}

auto translate(const proto::PeerRequestType in) noexcept
    -> contract::peer::PeerRequestType
{
    static const auto map = reverse_arbitrary_map<
        contract::peer::PeerRequestType,
        proto::PeerRequestType,
        contract::peer::PeerRequestTypeReverseMap>(
        contract::peer::peerrequesttype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::PeerRequestType::Error;
    }
}

auto translate(const proto::SecretType in) noexcept
    -> contract::peer::SecretType
{
    static const auto map = reverse_arbitrary_map<
        contract::peer::SecretType,
        proto::SecretType,
        contract::peer::SecretTypeReverseMap>(contract::peer::secrettype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::peer::SecretType::Error;
    }
}
}  // namespace opentxs
