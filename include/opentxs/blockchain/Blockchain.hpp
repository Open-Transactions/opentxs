// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>  // IWYU pragma: keep
#include <tuple>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
OPENTXS_EXPORT auto BlockHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto DefinedChains() noexcept -> const UnallocatedSet<Type>&;
OPENTXS_EXPORT auto FilterHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto HashToNumber(const api::Session& api, ReadView hex) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto HashToNumber(const Hash& hash) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto HasSegwit(const Type type) noexcept -> bool;
OPENTXS_EXPORT auto IsTestnet(const Type type) noexcept -> bool;
OPENTXS_EXPORT auto MerkleHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto NumberToHash(const api::Session& api, ReadView hex) noexcept
    -> pHash;
OPENTXS_EXPORT auto P2PMessageHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto ProofOfWorkHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto PubkeyHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto ScriptHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto ScriptHashSegwit(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto SupportedChains() noexcept -> const UnallocatedSet<Type>&;
OPENTXS_EXPORT auto TickerSymbol(const Type type) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto TransactionHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
}  // namespace opentxs::blockchain
