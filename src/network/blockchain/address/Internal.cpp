// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/Address.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::internal
{
auto Address::AddService(const bitcoin::Service) noexcept -> void {}

auto Address::Bytes() const noexcept -> ByteArray { return {}; }

auto Address::Chain() const noexcept -> opentxs::blockchain::Type { return {}; }

auto Address::Cookie() const noexcept -> ReadView { return {}; }

auto Address::Display() const noexcept -> UnallocatedCString { return {}; }

auto Address::ID() const noexcept -> const AddressID&
{
    static const auto blank = identifier::Generic{};

    return blank;
}

auto Address::Incoming() const noexcept -> bool { return {}; }

auto Address::IsValid() const noexcept -> bool { return {}; }

auto Address::Key() const noexcept -> ReadView { return {}; }

auto Address::LastConnected() const noexcept -> Time { return {}; }

auto Address::Port() const noexcept -> std::uint16_t { return {}; }

auto Address::PreviousLastConnected() const noexcept -> Time { return {}; }

auto Address::PreviousServices() const noexcept -> Set<bitcoin::Service>
{
    return {};
}

auto Address::RemoveService(const bitcoin::Service) noexcept -> void {}

auto Address::Serialize(proto::BlockchainPeerAddress&) const noexcept -> bool
{
    return {};
}

auto Address::Services() const noexcept -> Set<bitcoin::Service> { return {}; }

auto Address::SetIncoming(bool) noexcept -> void {}

auto Address::SetLastConnected(const Time&) noexcept -> void {}

auto Address::SetServices(const Set<bitcoin::Service>&) noexcept -> void {}

auto Address::Style() const noexcept -> Protocol { return {}; }

auto Address::Subtype() const noexcept -> Transport { return {}; }

auto Address::Type() const noexcept -> Transport { return {}; }
}  // namespace opentxs::network::blockchain::internal
