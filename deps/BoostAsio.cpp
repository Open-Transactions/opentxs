// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "BoostAsio.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_set.h>
#include <functional>

namespace opentxs
{
auto unexpected_asio_error(const boost::system::error_code& ec) noexcept -> bool
{
    using namespace boost::system::errc;
    static constexpr auto ignore =
        frozen::make_unordered_set<int>({operation_canceled, 995});

    if (ec) {

        return false == ignore.contains(ec.value());
    } else {

        return false;
    }
}
}  // namespace opentxs
