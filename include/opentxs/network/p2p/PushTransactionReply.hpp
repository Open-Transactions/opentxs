// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/p2p/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT PushTransactionReply final : public Base
{
public:
    class Imp;

    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto ID() const noexcept -> const opentxs::blockchain::block::Txid&;
    auto Success() const noexcept -> bool;

    OPENTXS_NO_EXPORT PushTransactionReply(Imp* imp) noexcept;

    ~PushTransactionReply() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    PushTransactionReply(const PushTransactionReply&) = delete;
    PushTransactionReply(PushTransactionReply&&) = delete;
    auto operator=(const PushTransactionReply&)
        -> PushTransactionReply& = delete;
    auto operator=(PushTransactionReply&&) -> PushTransactionReply& = delete;
};
}  // namespace opentxs::network::p2p
