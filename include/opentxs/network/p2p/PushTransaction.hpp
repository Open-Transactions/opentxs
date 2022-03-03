// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/util/Bytes.hpp"

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
class OPENTXS_EXPORT PushTransaction final : public Base
{
public:
    class Imp;

    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto ID() const noexcept -> const opentxs::blockchain::block::Txid&;
    auto Payload() const noexcept -> ReadView;

    OPENTXS_NO_EXPORT PushTransaction(Imp* imp) noexcept;

    ~PushTransaction() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    PushTransaction(const PushTransaction&) = delete;
    PushTransaction(PushTransaction&&) = delete;
    auto operator=(const PushTransaction&) -> PushTransaction& = delete;
    auto operator=(PushTransaction&&) -> PushTransaction& = delete;
};
}  // namespace opentxs::network::p2p
