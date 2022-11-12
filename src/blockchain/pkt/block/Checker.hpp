// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"

#pragma once

#include "blockchain/bitcoin/block/Checker.hpp"

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::pkt::block
{
class Checker final : public bitcoin::block::Checker
{
public:
    Checker() = delete;
    Checker(const api::Crypto& crypto, blockchain::Type type) noexcept;
    Checker(const Checker&) noexcept;
    Checker(Checker&&) noexcept;
    auto operator=(const Checker&) -> Checker& = delete;
    auto operator=(Checker&&) -> Checker& = delete;

    ~Checker() override = default;

protected:
    auto find_payload() noexcept -> bool final;
};
}  // namespace opentxs::blockchain::pkt::block
