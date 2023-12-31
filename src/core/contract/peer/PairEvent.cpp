// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/PairEvent.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PairEvent.pb.h>

#include "opentxs/protobuf/Types.internal.tpp"

namespace opentxs::contract::peer::internal
{
PairEvent::PairEvent(const ReadView view)
    : PairEvent(protobuf::Factory<protobuf::PairEvent>(view))
{
}

PairEvent::PairEvent(const protobuf::PairEvent& serialized)
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
