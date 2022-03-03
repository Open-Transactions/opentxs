// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "internal/blockchain/crypto/Crypto.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const opentxs::blockchain::Type chain,
    const Data& txid) noexcept -> OTIdentifier
{
    auto preimage = UnallocatedCString{};
    const auto hashed = crypto.Hash().HMAC(
        crypto::HashType::Sha256,
        ReadView{reinterpret_cast<const char*>(&chain), sizeof(chain)},
        txid.Bytes(),
        writer(preimage));

    OT_ASSERT(hashed);

    auto id = Identifier::Factory();
    id->CalculateDigest(preimage);

    return id;
}

auto operator==(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return false; }

    if (lSubchain != rSubchain) { return false; }

    return lIndex == rIndex;
}

auto operator!=(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return true; }

    if (lSubchain != rSubchain) { return true; }

    return lIndex != rIndex;
}

auto preimage(const blockchain::crypto::Key& in) noexcept -> Space
{
    const auto& [id, subchain, index] = in;
    auto out = space(id.size() + sizeof(subchain) + sizeof(index));
    auto i = out.data();
    std::memcpy(i, id.data(), id.size());
    std::advance(i, id.size());
    std::memcpy(i, &subchain, sizeof(subchain));
    std::advance(i, sizeof(subchain));
    std::memcpy(i, &index, sizeof(index));
    std::advance(i, sizeof(index));

    return out;
}
auto print(blockchain::crypto::HDProtocol value) noexcept -> UnallocatedCString
{
    using Proto = blockchain::crypto::HDProtocol;
    static const auto map =
        robin_hood::unordered_flat_map<Proto, UnallocatedCString>{
            {Proto::BIP_32, "BIP-32"},
            {Proto::BIP_44, "BIP-44"},
            {Proto::BIP_49, "BIP-49"},
            {Proto::BIP_84, "BIP-84"},
            {Proto::Error, "invalid"},
        };

    try {

        return map.at(value);
    } catch (...) {

        return map.at(Proto::Error);
    }
}

auto print(blockchain::crypto::Subchain value) noexcept -> UnallocatedCString
{
    using Subchain = blockchain::crypto::Subchain;
    static const auto map =
        robin_hood::unordered_flat_map<Subchain, UnallocatedCString>{
            {Subchain::Internal, "internal"},
            {Subchain::External, "external"},
            {Subchain::Incoming, "incoming"},
            {Subchain::Outgoing, "outgoing"},
            {Subchain::Notification, "notification"},
            {Subchain::None, "none"},
        };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(const blockchain::crypto::Key& key) noexcept -> UnallocatedCString
{
    const auto& [account, subchain, index] = key;

    return account + " / " + print(subchain) + " / " + std::to_string(index);
}
}  // namespace opentxs
