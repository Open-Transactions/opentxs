// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/FaucetRequest.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

class Factory;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class Faucet final : public request::Faucet,
                     public peer::implementation::Request
{
public:
    auto Address() const -> std::string_view final { return {}; }
    auto Currency() const -> opentxs::UnitType final { return {}; }

    Faucet(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        opentxs::UnitType unit,
        std::string_view address);
    Faucet(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    Faucet() = delete;
    Faucet(const Faucet&);
    Faucet(Faucet&&) = delete;
    auto operator=(const Faucet&) -> Faucet& = delete;
    auto operator=(Faucet&&) -> Faucet& = delete;

    ~Faucet() final = default;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const opentxs::UnitType unit_;
    const CString address_;

    auto clone() const noexcept -> Faucet* final { return new Faucet(*this); }
    auto IDVersion(const Lock& lock) const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::request::implementation
