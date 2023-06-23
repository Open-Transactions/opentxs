// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/Peer.hpp"  // IWYU pragma: associated

#include <PeerEnums.pb.h>
#include <ZMQEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>
#include <utility>

#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/PeerObjectType.hpp"   // IWYU pragma: keep
#include "opentxs/core/contract/peer/PeerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/SecretType.hpp"       // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer
{
constexpr auto connection_info_map_ = [] {
    using enum ConnectionInfoType;
    using enum proto::ConnectionInfoType;

    return frozen::
        make_unordered_map<ConnectionInfoType, proto::ConnectionInfoType>({
            {Error, CONNECTIONINFO_ERROR},
            {Bitcoin, CONNECTIONINFO_BITCOIN},
            {BtcRpc, CONNECTIONINFO_BTCRPC},
            {BitMessage, CONNECTIONINFO_BITMESSAGE},
            {BitMessageRPC, CONNECTIONINFO_BITMESSAGERPC},
            {SSH, CONNECTIONINFO_SSH},
            {CJDNS, CONNECTIONINFO_CJDNS},
        });
}();
constexpr auto pair_event_type_map_ = [] {
    using enum internal::PairEventType;
    using enum proto::PairEventType;

    return frozen::
        make_unordered_map<internal::PairEventType, proto::PairEventType>({
            {Error, PAIREVENT_ERROR},
            {Rename, PAIREVENT_RENAME},
            {StoreSecret, PAIREVENT_STORESECRET},
        });
}();
constexpr auto peer_object_type_map_ = [] {
    using enum PeerObjectType;
    using enum proto::PeerObjectType;

    return frozen::make_unordered_map<PeerObjectType, proto::PeerObjectType>({
        {Error, PEEROBJECT_ERROR},
        {Message, PEEROBJECT_MESSAGE},
        {Request, PEEROBJECT_REQUEST},
        {Response, PEEROBJECT_RESPONSE},
        {Payment, PEEROBJECT_PAYMENT},
        {Cash, PEEROBJECT_CASH},
    });
}();
constexpr auto peer_request_type_map_ = [] {
    using enum PeerRequestType;
    using enum proto::PeerRequestType;

    return frozen::make_unordered_map<PeerRequestType, proto::PeerRequestType>({
        {Error, PEERREQUEST_ERROR},
        {Bailment, PEERREQUEST_BAILMENT},
        {OutBailment, PEERREQUEST_OUTBAILMENT},
        {PendingBailment, PEERREQUEST_PENDINGBAILMENT},
        {ConnectionInfo, PEERREQUEST_CONNECTIONINFO},
        {StoreSecret, PEERREQUEST_STORESECRET},
        {VerificationOffer, PEERREQUEST_VERIFICATIONOFFER},
        {Faucet, PEERREQUEST_FAUCET},
    });
}();
constexpr auto secret_type_map_ = [] {
    using enum SecretType;
    using enum proto::SecretType;

    return frozen::make_unordered_map<SecretType, proto::SecretType>({
        {SecretType::Error, proto::SECRETTYPE_ERROR},
        {SecretType::Bip39, proto::SECRETTYPE_BIP39},
    });
}();
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer::internal
{
auto translate(const PairEventType in) noexcept -> proto::PairEventType
{
    static constexpr const auto& map = pair_event_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::PAIREVENT_ERROR;
    }
}
}  // namespace opentxs::contract::peer::internal

namespace opentxs::contract::peer
{
auto translate(const ConnectionInfoType in) noexcept
    -> proto::ConnectionInfoType
{
    static constexpr const auto& map = connection_info_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::CONNECTIONINFO_ERROR;
    }
}

auto translate(const PeerObjectType in) noexcept -> proto::PeerObjectType
{
    static constexpr const auto& map = peer_object_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::PEEROBJECT_ERROR;
    }
}

auto translate(const PeerRequestType in) noexcept -> proto::PeerRequestType
{
    static constexpr const auto& map = peer_request_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::PEERREQUEST_ERROR;
    }
}

auto translate(const SecretType in) noexcept -> proto::SecretType
{
    static constexpr const auto& map = secret_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::SECRETTYPE_ERROR;
    }
}
}  // namespace opentxs::contract::peer

namespace opentxs::proto
{
auto translate(const ConnectionInfoType in) noexcept
    -> contract::peer::ConnectionInfoType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::connection_info_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::ConnectionInfoType::Error;
    }
}

auto translate(const PairEventType in) noexcept
    -> contract::peer::internal::PairEventType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::pair_event_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::internal::PairEventType::Error;
    }
}

auto translate(const PeerObjectType in) noexcept
    -> contract::peer::PeerObjectType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::peer_object_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::PeerObjectType::Error;
    }
}

auto translate(const PeerRequestType in) noexcept
    -> contract::peer::PeerRequestType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::peer_request_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::PeerRequestType::Error;
    }
}

auto translate(const SecretType in) noexcept -> contract::peer::SecretType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::secret_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::SecretType::Error;
    }
}
}  // namespace opentxs::proto