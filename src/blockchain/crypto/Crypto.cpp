// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <sstream>
#include <string_view>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"    // IWYU pragma: keep
#include "opentxs/blockchain/crypto/HDProtocol.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::crypto
{
using namespace std::literals;

auto is_notification(Subchain in) noexcept -> bool
{
    switch (in) {
        case Subchain::NotificationV1:
        case Subchain::NotificationV2:
        case Subchain::NotificationV3:
        case Subchain::NotificationV4: {

            return true;
        }
        default: {

            return false;
        }
    }
}

auto operator==(const Key& lhs, const Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return false; }

    if (lSubchain != rSubchain) { return false; }

    return lIndex == rIndex;
}

auto operator!=(const Key& lhs, const Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return true; }

    if (lSubchain != rSubchain) { return true; }

    return lIndex != rIndex;
}

auto print(AddressStyle value) noexcept -> std::string_view
{
    using enum AddressStyle;
    static constexpr auto map =
        frozen::make_unordered_map<AddressStyle, std::string_view>({
            {Unknown, "Unknown"sv},
            {P2PKH, "P2PKH"sv},
            {P2SH, "P2SH"sv},
            {P2WPKH, "P2WPKH"sv},
            {P2WSH, "P2WSH"sv},
            {P2TR, "P2TR"sv},
        });

    if (const auto* i = map.find(value); map.end() != i) {

        return i->second;
    } else {

        return map.at(Unknown);
    }
}

auto print(HDProtocol value) noexcept -> std::string_view
{
    using enum HDProtocol;
    static constexpr auto map =
        frozen::make_unordered_map<HDProtocol, std::string_view>({
            {Error, "invalid"sv},
            {BIP_32, "BIP-32"sv},
            {BIP_44, "BIP-44"sv},
            {BIP_49, "BIP-49"sv},
            {BIP_84, "BIP-84"sv},
        });

    if (const auto* i = map.find(value); map.end() != i) {

        return i->second;
    } else {

        return map.at(Error);
    }
}

auto print(SubaccountType type) noexcept -> std::string_view
{
    using enum SubaccountType;
    static constexpr auto map =
        frozen::make_unordered_map<SubaccountType, std::string_view>({
            {Error, "invalid"sv},
            {HD, "HD"sv},
            {PaymentCode, "payment code"sv},
            {Imported, "single key"sv},
            {Notification, "payment code notification"sv},
        });

    if (const auto* i = map.find(type); map.end() != i) {

        return i->second;
    } else {

        return map.at(Error);
    }
}
auto print(Subchain value) noexcept -> std::string_view
{
    using enum Subchain;
    static constexpr auto map =
        frozen::make_unordered_map<Subchain, std::string_view>({
            {Error, "invalid"sv},
            {Internal, "internal"sv},
            {External, "external"sv},
            {Incoming, "incoming"sv},
            {Outgoing, "outgoing"sv},
            {NotificationV3, "version 3"sv},
            {NotificationV1, "version 1"sv},
            {NotificationV2, "version 2"sv},
            {NotificationV4, "version 4"sv},
            {None, "none"sv},
        });

    if (const auto* i = map.find(value); map.end() != i) {

        return i->second;
    } else {

        return map.at(Error);
    }
}

auto print(const Key& key, const api::Crypto& api) noexcept
    -> UnallocatedCString
{
    const auto& [account, subchain, index] = key;
    auto out = std::stringstream{};
    out << account.asBase58(api);
    out << " / ";
    out << print(subchain);
    out << " / ";
    out << std::to_string(index);

    return out.str();
}
}  // namespace opentxs::blockchain::crypto

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const blockchain::block::TransactionHash& txid) noexcept
    -> identifier::Generic
{
    auto preimage = UnallocatedCString{};
    const auto hashed = crypto.Hash().HMAC(
        crypto::HashType::Sha256,
        ReadView{reinterpret_cast<const char*>(&chain), sizeof(chain)},
        txid.Bytes(),
        writer(preimage));

    OT_ASSERT(hashed);

    return factory.IdentifierFromPreimage(preimage);
}

auto deserialize(const ReadView in) noexcept -> blockchain::crypto::Key
{
    auto out = blockchain::crypto::Key{};
    auto& [id, subchain, index] = out;

    if (in.size() < (sizeof(subchain) + sizeof(index))) { return out; }

    const auto idbytes = in.size() - sizeof(subchain) - sizeof(index);

    const auto* i = in.data();
    id.Assign(ReadView{i, idbytes});
    std::advance(i, idbytes);
    std::memcpy(&subchain, i, sizeof(subchain));
    std::advance(i, sizeof(subchain));
    std::memcpy(&index, i, sizeof(index));
    std::advance(i, sizeof(index));

    return out;
}

auto serialize(const blockchain::crypto::Key& in) noexcept -> Space
{
    const auto& [id, subchain, index] = in;
    auto out = space(id.size() + sizeof(subchain) + sizeof(index));
    auto* i = out.data();
    std::memcpy(i, id.data(), id.size());
    std::advance(i, id.size());
    std::memcpy(i, &subchain, sizeof(subchain));
    std::advance(i, sizeof(subchain));
    std::memcpy(i, &index, sizeof(index));
    std::advance(i, sizeof(index));

    return out;
}
}  // namespace opentxs
