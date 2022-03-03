// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace p2p
{
class State;
}  // namespace p2p
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT Acknowledgement final : public Base
{
public:
    class Imp;

    auto Endpoint() const noexcept -> const UnallocatedCString&;
    auto State() const noexcept -> const StateData&;
    /// throws std::out_of_range if specified chain is not present
    auto State(opentxs::blockchain::Type chain) const noexcept(false)
        -> const p2p::State&;

    OPENTXS_NO_EXPORT Acknowledgement(Imp* imp) noexcept;

    ~Acknowledgement() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    Acknowledgement(const Acknowledgement&) = delete;
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;
};
}  // namespace opentxs::network::p2p
