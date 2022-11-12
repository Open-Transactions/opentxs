// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class Checker
{
public:
    [[nodiscard]] static auto Check(
        const api::Crypto& crypto,
        const blockchain::Type type,
        const Hash& expected,
        const ReadView bytes) noexcept -> bool;

    [[nodiscard]] virtual auto operator()(
        const Hash& expected,
        ReadView bytes) noexcept -> bool = 0;

    virtual ~Checker() = default;
};
}  // namespace opentxs::blockchain::block
