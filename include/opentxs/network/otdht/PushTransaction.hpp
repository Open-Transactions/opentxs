// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT PushTransaction final : public Base
{
public:
    class Imp;

    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto ID() const noexcept
        -> const opentxs::blockchain::block::TransactionHash&;
    auto Payload() const noexcept -> ReadView;

    OPENTXS_NO_EXPORT PushTransaction(Imp* imp) noexcept;
    PushTransaction(const PushTransaction&) = delete;
    PushTransaction(PushTransaction&&) = delete;
    auto operator=(const PushTransaction&) -> PushTransaction& = delete;
    auto operator=(PushTransaction&&) -> PushTransaction& = delete;

    ~PushTransaction() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
