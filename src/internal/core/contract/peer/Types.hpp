// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ConnectionInfoType
// IWYU pragma: no_forward_declare opentxs::proto::PairEventType

#pragma once

#include <PeerEnums.pb.h>
#include <ZMQEnums.pb.h>
#include <cstdint>

#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::internal
{
enum class PairEventType : std::uint32_t;  // IWYU pragma: export

auto translate(PairEventType) noexcept -> proto::PairEventType;
}  // namespace opentxs::contract::peer::internal

namespace opentxs::contract::peer
{
auto translate(ConnectionInfoType) noexcept -> proto::ConnectionInfoType;
auto translate(internal::PairEventType) noexcept -> proto::PairEventType;
auto translate(ObjectType) noexcept -> proto::PeerObjectType;
auto translate(RequestType) noexcept -> proto::PeerRequestType;
auto translate(SecretType) noexcept -> proto::SecretType;
}  // namespace opentxs::contract::peer

namespace opentxs::proto
{
auto translate(ConnectionInfoType) noexcept
    -> contract::peer::ConnectionInfoType;
auto translate(PairEventType) noexcept
    -> contract::peer::internal::PairEventType;
auto translate(PeerObjectType) noexcept -> contract::peer::ObjectType;
auto translate(PeerRequestType) noexcept -> contract::peer::RequestType;
auto translate(SecretType) noexcept -> contract::peer::SecretType;
}  // namespace opentxs::proto
