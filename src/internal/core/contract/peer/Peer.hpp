// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
// IWYU pragma: no_include "opentxs/core/contract/peer/PeerObjectType.hpp"
// IWYU pragma: no_include "opentxs/core/contract/peer/PeerRequestType.hpp"
// IWYU pragma: no_include "opentxs/core/contract/peer/SecretType.hpp"

#pragma once

#include <cstdint>

#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/ContractEnums.pb.h"
#include "serialization/protobuf/PairEvent.pb.h"
#include "serialization/protobuf/PeerEnums.pb.h"
#include "serialization/protobuf/ZMQEnums.pb.h"
#include "util/Blank.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class PairEvent;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::internal
{
enum class PairEventType : std::uint8_t {
    Error = 0,
    Rename = 1,
    StoreSecret = 2,
};

struct PairEvent {
    std::uint32_t version_;
    PairEventType type_;
    UnallocatedCString issuer_;

    PairEvent(const ReadView);

private:
    PairEvent(
        const std::uint32_t,
        const PairEventType,
        const UnallocatedCString&);
    PairEvent(const proto::PairEvent&);

    PairEvent() = delete;
    PairEvent(const PairEvent&) = delete;
    PairEvent(PairEvent&&) = delete;
    auto operator=(const PairEvent&) -> PairEvent& = delete;
    auto operator=(PairEvent&&) -> PairEvent& = delete;
};
}  // namespace opentxs::contract::peer::internal

namespace opentxs
{
auto translate(const contract::peer::ConnectionInfoType in) noexcept
    -> proto::ConnectionInfoType;
auto translate(const contract::peer::internal::PairEventType in) noexcept
    -> proto::PairEventType;
auto translate(const contract::peer::PeerObjectType in) noexcept
    -> proto::PeerObjectType;
auto translate(const contract::peer::PeerRequestType in) noexcept
    -> proto::PeerRequestType;
auto translate(const contract::peer::SecretType in) noexcept
    -> proto::SecretType;
auto translate(const proto::ConnectionInfoType in) noexcept
    -> contract::peer::ConnectionInfoType;
auto translate(const proto::PairEventType in) noexcept
    -> contract::peer::internal::PairEventType;
auto translate(const proto::PeerObjectType in) noexcept
    -> contract::peer::PeerObjectType;
auto translate(const proto::PeerRequestType in) noexcept
    -> contract::peer::PeerRequestType;
auto translate(const proto::SecretType in) noexcept
    -> contract::peer::SecretType;
}  // namespace opentxs
