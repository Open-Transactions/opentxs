// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "internal/blockchain/crypto/Crypto.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/HashType.hpp"

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

auto print(AddressStyle value) noexcept -> std::string_view
{
    using Type = AddressStyle;
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::Unknown, "Unknown"sv},
            {Type::P2PKH, "P2PKH"sv},
            {Type::P2SH, "P2SH"sv},
            {Type::P2WPKH, "P2WPKH"sv},
            {Type::P2WSH, "P2WSH"sv},
            {Type::P2TR, "P2TR"sv},
        };

    try {

        return map.at(value);
    } catch (...) {

        return map.at(Type::Unknown);
    }
}

auto print(HDProtocol value) noexcept -> std::string_view
{
    using Type = HDProtocol;
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::Error, "invalid"sv},
            {Type::BIP_32, "BIP-32"sv},
            {Type::BIP_44, "BIP-44"sv},
            {Type::BIP_49, "BIP-49"sv},
            {Type::BIP_84, "BIP-84"sv},
        };

    try {

        return map.at(value);
    } catch (...) {

        return map.at(Type::Error);
    }
}

auto print(SubaccountType type) noexcept -> std::string_view
{
    using Type = SubaccountType;
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::Error, "invalid"sv},
            {Type::HD, "HD"sv},
            {Type::PaymentCode, "payment code"sv},
            {Type::Imported, "single key"sv},
            {Type::Notification, "payment code notification"sv},
        };

    try {

        return map.at(type);
    } catch (...) {

        return map.at(Type::Error);
    }
}
auto print(Subchain value) noexcept -> std::string_view
{
    using Type = Subchain;
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::Error, "invalid"sv},
            {Type::Internal, "internal"sv},
            {Type::External, "external"sv},
            {Type::Incoming, "incoming"sv},
            {Type::Outgoing, "outgoing"sv},
            {Type::NotificationV3, "version 3"sv},
            {Type::NotificationV1, "version 1"sv},
            {Type::NotificationV2, "version 2"sv},
            {Type::NotificationV4, "version 4"sv},
            {Type::None, "none"sv},
        };

    try {

        return map.at(value);
    } catch (...) {

        return map.at(Type::Error);
    }
}

auto print(const Key& key) noexcept -> UnallocatedCString
{
    const auto& [account, subchain, index] = key;
    auto out = std::stringstream{};
    out << account.asBase58(Context().Crypto());
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
    const opentxs::blockchain::Type chain,
    const Data& txid) noexcept -> identifier::Generic
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
