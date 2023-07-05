// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/core/contract/peer/request/Base.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
namespace internal
{
class Faucet;
}  // namespace internal
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTFaucetRequest = SharedPimpl<contract::peer::request::internal::Faucet>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::internal
{
class Faucet : virtual public internal::Request
{
public:
    virtual auto Address() const -> std::string_view = 0;
    virtual auto Currency() const -> opentxs::UnitType = 0;

    Faucet(const Faucet&) = delete;
    Faucet(Faucet&&) = delete;
    auto operator=(const Faucet&) -> Faucet& = delete;
    auto operator=(Faucet&&) -> Faucet& = delete;

    ~Faucet() override = default;

protected:
    Faucet() noexcept = default;

private:
    friend OTFaucetRequest;
};
}  // namespace opentxs::contract::peer::request::internal
