// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class Ripemd160
{
public:
    virtual auto RIPEMD160(const ReadView data, Writer&& destination)
        const noexcept -> bool = 0;

    Ripemd160(const Ripemd160&) = delete;
    Ripemd160(Ripemd160&&) = delete;
    auto operator=(const Ripemd160&) -> Ripemd160& = delete;
    auto operator=(Ripemd160&&) -> Ripemd160& = delete;

    virtual ~Ripemd160() = default;

protected:
    Ripemd160() = default;
};
}  // namespace opentxs::crypto
