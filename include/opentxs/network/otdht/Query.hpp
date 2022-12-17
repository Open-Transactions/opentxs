// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/network/otdht/Base.hpp"

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT Query final : public Base
{
public:
    class Imp;

    OPENTXS_NO_EXPORT Query(Imp* imp) noexcept;
    Query(const Query&) = delete;
    Query(Query&&) = delete;
    auto operator=(const Query&) -> Query& = delete;
    auto operator=(Query&&) -> Query& = delete;

    ~Query() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
