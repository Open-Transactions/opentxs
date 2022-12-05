// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::contract::ProtocolVersion

#include "internal/core/contract/Contract.hpp"  // IWYU pragma: associated

#include <ContractEnums.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <ServerContract.pb.h>
#include <UnitDefinition.pb.h>
#include <robin_hood.h>

#include "opentxs/core/contract/ProtocolVersion.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/UnitType.hpp"  // IWYU pragma: keep
#include "util/Container.hpp"

namespace opentxs::contract::peer::blank
{
auto Reply::asAcknowledgement() const noexcept
    -> const peer::reply::Acknowledgement&
{
    static auto const blank = peer::reply::blank::Acknowledgement{api_};
    return blank;
}

auto Reply::asBailment() const noexcept -> const peer::reply::Bailment&
{
    static auto const blank = peer::reply::blank::Bailment{api_};
    return blank;
}

auto Reply::asConnection() const noexcept -> const peer::reply::Connection&
{
    static auto const blank = peer::reply::blank::Connection{api_};
    return blank;
}

auto Reply::asOutbailment() const noexcept -> const peer::reply::Outbailment&
{
    static auto const blank = peer::reply::blank::Outbailment{api_};
    return blank;
}

auto Reply::Serialize(SerializedType& output) const -> bool
{
    output = {};
    return true;
}

auto Request::asBailment() const noexcept -> const peer::request::Bailment&
{
    static auto const blank = peer::request::blank::Bailment{api_};
    return blank;
}

auto Request::asBailmentNotice() const noexcept
    -> const peer::request::BailmentNotice&
{
    static auto const blank = peer::request::blank::BailmentNotice{api_};
    return blank;
}

auto Request::asConnection() const noexcept -> const peer::request::Connection&
{
    static auto const blank = peer::request::blank::Connection{api_};
    return blank;
}

auto Request::asOutbailment() const noexcept
    -> const peer::request::Outbailment&
{
    static auto const blank = peer::request::blank::Outbailment{api_};
    return blank;
}

auto Request::asStoreSecret() const noexcept
    -> const peer::request::StoreSecret&
{
    static auto const blank = peer::request::blank::StoreSecret{api_};
    return blank;
}

auto Request::Serialize(SerializedType& output) const -> bool
{
    output = {};
    return true;
}
}  // namespace opentxs::contract::peer::blank

namespace opentxs::contract::blank
{
auto Server::Serialize(proto::ServerContract& output, bool includeNym) const
    -> bool
{
    output = {};
    return true;
}

auto Unit::Serialize(proto::UnitDefinition& output, bool includeNym) const
    -> bool
{
    output = {};
    return true;
}
}  // namespace opentxs::contract::blank

namespace opentxs::contract
{
using ProtocolVersionMap =
    robin_hood::unordered_flat_map<ProtocolVersion, proto::ProtocolVersion>;
using ProtocolVersionReverseMap =
    robin_hood::unordered_flat_map<proto::ProtocolVersion, ProtocolVersion>;
using UnitTypeMap = robin_hood::unordered_flat_map<UnitType, proto::UnitType>;
using UnitTypeReverseMap =
    robin_hood::unordered_flat_map<proto::UnitType, UnitType>;

auto protocolversion_map() noexcept -> const ProtocolVersionMap&;
auto unittype_map() noexcept -> const UnitTypeMap&;
}  // namespace opentxs::contract

namespace opentxs::contract
{
auto protocolversion_map() noexcept -> const ProtocolVersionMap&
{
    static const auto map = ProtocolVersionMap{
        {ProtocolVersion::Error, proto::PROTOCOLVERSION_ERROR},
        {ProtocolVersion::Legacy, proto::PROTOCOLVERSION_LEGACY},
        {ProtocolVersion::Notify, proto::PROTOCOLVERSION_NOTIFY},
    };

    return map;
}

auto unittype_map() noexcept -> const UnitTypeMap&
{
    static const auto map = UnitTypeMap{
        {UnitType::Error, proto::UNITTYPE_ERROR},
        {UnitType::Currency, proto::UNITTYPE_CURRENCY},
        {UnitType::Security, proto::UNITTYPE_SECURITY},
        {UnitType::Basket, proto::UNITTYPE_ERROR},
    };

    return map;
}
}  // namespace opentxs::contract

namespace opentxs
{
auto translate(const contract::ProtocolVersion in) noexcept
    -> proto::ProtocolVersion
{
    try {
        return contract::protocolversion_map().at(in);
    } catch (...) {
        return proto::PROTOCOLVERSION_ERROR;
    }
}

auto translate(const contract::UnitType in) noexcept -> proto::UnitType
{
    try {
        return contract::unittype_map().at(in);
    } catch (...) {
        return proto::UNITTYPE_ERROR;
    }
}

auto translate(const proto::ProtocolVersion in) noexcept
    -> contract::ProtocolVersion
{
    static const auto map = reverse_arbitrary_map<
        contract::ProtocolVersion,
        proto::ProtocolVersion,
        contract::ProtocolVersionReverseMap>(contract::protocolversion_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::ProtocolVersion::Error;
    }
}

auto translate(const proto::UnitType in) noexcept -> contract::UnitType
{
    static const auto map = reverse_arbitrary_map<
        contract::UnitType,
        proto::UnitType,
        contract::UnitTypeReverseMap>(contract::unittype_map());

    try {
        return map.at(in);
    } catch (...) {
        return contract::UnitType::Error;
    }
}
}  // namespace opentxs
