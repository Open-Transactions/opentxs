// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Types.hpp"

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT Request final : public Base
{
public:
    class Imp;

    auto State() const noexcept -> const StateData&;

    OPENTXS_NO_EXPORT Request(Imp* imp) noexcept;
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;

    ~Request() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
