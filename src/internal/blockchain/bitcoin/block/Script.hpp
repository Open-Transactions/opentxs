// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#pragma once

#include <memory>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Script : virtual public block::Script
{
public:
    static auto blank_signature(const blockchain::Type chain) noexcept
        -> const Space&;
    static auto blank_pubkey(
        const blockchain::Type chain,
        const bool compressed = true) noexcept -> const Space&;

    virtual auto clone() const noexcept -> std::unique_ptr<Script> = 0;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void = 0;
    virtual auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void = 0;
    auto Internal() const noexcept -> const internal::Script& final
    {
        return *this;
    }
    virtual auto LikelyPubkeyHashes(const api::Crypto& crypto) const noexcept
        -> UnallocatedVector<ByteArray> = 0;
    virtual auto SigningSubscript(const blockchain::Type chain) const noexcept
        -> std::unique_ptr<Script> = 0;

    auto Internal() noexcept -> internal::Script& final { return *this; }

    ~Script() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
