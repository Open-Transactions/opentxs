// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/contract/peer/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/ZMQEnums.pb.h>
#include <functional>
#include <utility>

#include "internal/core/contract/peer/PairEventType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/ObjectType.hpp"   // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/SecretType.hpp"   // IWYU pragma: keep

namespace opentxs::contract::peer
{
constexpr auto connection_info_map_ = [] {
    using enum ConnectionInfoType;
    using enum protobuf::ConnectionInfoType;

    return frozen::
        make_unordered_map<ConnectionInfoType, protobuf::ConnectionInfoType>({
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
    using enum protobuf::PairEventType;

    return frozen::
        make_unordered_map<internal::PairEventType, protobuf::PairEventType>({
            {Error, PAIREVENT_ERROR},
            {Rename, PAIREVENT_RENAME},
            {StoreSecret, PAIREVENT_STORESECRET},
        });
}();
constexpr auto peer_object_type_map_ = [] {
    using enum ObjectType;
    using enum protobuf::PeerObjectType;

    return frozen::make_unordered_map<ObjectType, protobuf::PeerObjectType>({
        {Error, PEEROBJECT_ERROR},
        {Message, PEEROBJECT_MESSAGE},
        {Request, PEEROBJECT_REQUEST},
        {Response, PEEROBJECT_RESPONSE},
        {Payment, PEEROBJECT_PAYMENT},
        {Cash, PEEROBJECT_CASH},
    });
}();
constexpr auto peer_request_type_map_ = [] {
    using enum RequestType;
    using enum protobuf::PeerRequestType;

    return frozen::make_unordered_map<RequestType, protobuf::PeerRequestType>({
        {Error, PEERREQUEST_ERROR},
        {Bailment, PEERREQUEST_BAILMENT},
        {OutBailment, PEERREQUEST_OUTBAILMENT},
        {PendingBailment, PEERREQUEST_PENDINGBAILMENT},
        {ConnectionInfo, PEERREQUEST_CONNECTIONINFO},
        {StoreSecret, PEERREQUEST_STORESECRET},
        {VerifiedClaim, PEERREQUEST_VERIFIEDCLAIM},
        {Faucet, PEERREQUEST_FAUCET},
        {Verification, PEERREQUEST_VERIFICATION},
    });
}();
constexpr auto secret_type_map_ = [] {
    using enum SecretType;
    using enum protobuf::SecretType;

    return frozen::make_unordered_map<SecretType, protobuf::SecretType>({
        {SecretType::Error, protobuf::SECRETTYPE_ERROR},
        {SecretType::Bip39, protobuf::SECRETTYPE_BIP39},
    });
}();
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer::internal
{
auto translate(PairEventType in) noexcept -> protobuf::PairEventType
{
    static constexpr const auto& map = pair_event_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::PAIREVENT_ERROR;
    }
}
}  // namespace opentxs::contract::peer::internal

namespace opentxs::contract::peer
{
auto translate(ConnectionInfoType in) noexcept -> protobuf::ConnectionInfoType
{
    static constexpr const auto& map = connection_info_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::CONNECTIONINFO_ERROR;
    }
}

auto translate(ObjectType in) noexcept -> protobuf::PeerObjectType
{
    static constexpr const auto& map = peer_object_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::PEEROBJECT_ERROR;
    }
}

auto translate(RequestType in) noexcept -> protobuf::PeerRequestType
{
    static constexpr const auto& map = peer_request_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::PEERREQUEST_ERROR;
    }
}

auto translate(SecretType in) noexcept -> protobuf::SecretType
{
    static constexpr const auto& map = secret_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::SECRETTYPE_ERROR;
    }
}
}  // namespace opentxs::contract::peer

namespace opentxs::protobuf
{
auto translate(ConnectionInfoType in) noexcept
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

auto translate(PairEventType in) noexcept
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

auto translate(PeerObjectType in) noexcept -> contract::peer::ObjectType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::peer_object_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::ObjectType::Error;
    }
}

auto translate(PeerRequestType in) noexcept -> contract::peer::RequestType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::peer_request_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::RequestType::Error;
    }
}

auto translate(SecretType in) noexcept -> contract::peer::SecretType
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::peer::secret_type_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::peer::SecretType::Error;
    }
}
}  // namespace opentxs::protobuf
