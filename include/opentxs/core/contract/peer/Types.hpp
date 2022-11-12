// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::contract::peer
{
enum class ConnectionInfoType : std::uint8_t;
enum class PeerObjectType : std::uint8_t;
enum class PeerRequestType : std::uint8_t;
enum class SecretType : std::uint8_t;
}  // namespace opentxs::contract::peer
