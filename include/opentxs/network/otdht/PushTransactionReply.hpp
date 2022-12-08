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
class OPENTXS_EXPORT PushTransactionReply final : public Base
{
public:
    class Imp;

    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto ID() const noexcept
        -> const opentxs::blockchain::block::TransactionHash&;
    auto Success() const noexcept -> bool;

    OPENTXS_NO_EXPORT PushTransactionReply(Imp* imp) noexcept;
    PushTransactionReply(const PushTransactionReply&) = delete;
    PushTransactionReply(PushTransactionReply&&) = delete;
    auto operator=(const PushTransactionReply&)
        -> PushTransactionReply& = delete;
    auto operator=(PushTransactionReply&&) -> PushTransactionReply& = delete;

    ~PushTransactionReply() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
