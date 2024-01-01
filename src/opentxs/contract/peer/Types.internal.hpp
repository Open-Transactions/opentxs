// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ConnectionInfoType
// IWYU pragma: no_forward_declare opentxs::protobuf::PairEventType
// IWYU pragma: no_include "opentxs/core/contract/peer/ObjectType.hpp"
// IWYU pragma: no_include "opentxs/core/contract/peer/RequestType.hpp"

#pragma once

#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/ZMQEnums.pb.h>
#include <cstdint>

#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::internal
{
enum class PairEventType : std::uint32_t;  // IWYU pragma: export

auto translate(PairEventType) noexcept -> protobuf::PairEventType;
}  // namespace opentxs::contract::peer::internal

namespace opentxs::contract::peer
{
auto translate(ConnectionInfoType) noexcept -> protobuf::ConnectionInfoType;
auto translate(internal::PairEventType) noexcept -> protobuf::PairEventType;
auto translate(ObjectType) noexcept -> protobuf::PeerObjectType;
auto translate(RequestType) noexcept -> protobuf::PeerRequestType;
auto translate(SecretType) noexcept -> protobuf::SecretType;
}  // namespace opentxs::contract::peer

namespace opentxs::protobuf
{
auto translate(ConnectionInfoType) noexcept
    -> contract::peer::ConnectionInfoType;
auto translate(PairEventType) noexcept
    -> contract::peer::internal::PairEventType;
auto translate(PeerObjectType) noexcept -> contract::peer::ObjectType;
auto translate(PeerRequestType) noexcept -> contract::peer::RequestType;
auto translate(SecretType) noexcept -> contract::peer::SecretType;
}  // namespace opentxs::protobuf
