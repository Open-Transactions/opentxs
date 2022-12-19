// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace otdht
{
class State;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT Acknowledgement final : public Base
{
public:
    class Imp;

    auto Endpoint() const noexcept -> std::string_view;
    auto State() const noexcept -> const StateData&;
    /// throws std::out_of_range if specified chain is not present
    auto State(opentxs::blockchain::Type chain) const noexcept(false)
        -> const otdht::State&;

    OPENTXS_NO_EXPORT Acknowledgement(Imp* imp) noexcept;
    Acknowledgement(const Acknowledgement&) = delete;
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;

    ~Acknowledgement() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
