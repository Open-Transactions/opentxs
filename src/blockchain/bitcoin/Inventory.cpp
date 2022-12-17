// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/Inventory.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::bitcoin
{
const std::size_t Inventory::EncodedSize{sizeof(BitcoinFormat)};

const Inventory::Map Inventory::map_{
    {Type::None, 0},
    {Type::MsgTx, 1},
    {Type::MsgBlock, 2},
    {Type::MsgFilteredBlock, 3},
    {Type::MsgCmpctBlock, 4},
    {Type::MsgWitnessTx, 16777280},
    {Type::MsgWitnessBlock, 8388672},
    {Type::MsgFilteredWitnessBlock, 25165888},
};

const Inventory::ReverseMap Inventory::reverse_map_{reverse_map(map_)};

Inventory::Inventory(const Type type, const Hash& hash) noexcept
    : type_(type)
    , hash_(hash)
{
    static_assert(size() == sizeof(BitcoinFormat));
}

Inventory::Inventory(const void* payload, const std::size_t size) noexcept(
    false)
    : type_(decode_type(payload, size))
    , hash_(decode_hash(payload, size))
{
}

Inventory::Inventory(const ReadView payload) noexcept(false)
    : Inventory(payload.data(), payload.size())
{
}

Inventory::Inventory(const Inventory& rhs) noexcept
    : Inventory(rhs.type_, rhs.hash_.Bytes())
{
}

Inventory::Inventory(Inventory&& rhs) noexcept
    : type_(std::move(rhs.type_))
    , hash_(std::move(rhs.hash_))
{
}

Inventory::BitcoinFormat::BitcoinFormat(
    const Type type,
    const Hash& hash) noexcept(false)
    : type_(encode_type(type))
    , hash_(encode_hash(hash))
{
}

auto Inventory::decode_hash(
    const void* payload,
    const std::size_t size) noexcept(false) -> FixedByteArray<32>
{
    if (EncodedSize != size) { throw std::runtime_error("Invalid payload"); }

    const auto* it = std::next(
        static_cast<const char*>(payload), sizeof(BitcoinFormat::type_));

    return FixedByteArray<32>{{it, sizeof(BitcoinFormat::hash_)}};
}

auto Inventory::decode_type(
    const void* payload,
    const std::size_t size) noexcept(false) -> Inventory::Type
{
    p2p::bitcoin::message::InventoryTypeField type{};

    if (EncodedSize != size) { throw std::runtime_error("Invalid payload"); }

    std::memcpy(&type, payload, sizeof(type));
    auto inventorytype = Inventory::Type{};

    try {
        inventorytype = reverse_map_.at(type.value());
    } catch (...) {

        throw std::runtime_error{
            UnallocatedCString{"unknown payload type: "} +
            std::to_string(type.value())};
    }

    return inventorytype;
}

auto Inventory::DisplayType(const Type type) noexcept -> UnallocatedCString
{
    if (Type::None == type) {

        return "null";
    } else if (Type::MsgTx == type) {

        return "transaction";
    } else if (Type::MsgBlock == type) {

        return "block";
    } else if (Type::MsgFilteredBlock == type) {

        return "filtered block";
    } else if (Type::MsgCmpctBlock == type) {

        return "compact block";
    } else if (Type::MsgWitnessTx == type) {

        return "segwit transaction";
    } else if (Type::MsgWitnessBlock == type) {

        return "segwit block";
    } else if (Type::MsgFilteredWitnessBlock == type) {

        return "filtered segwit block";
    } else {

        return "unknown";
    }
}

auto Inventory::encode_hash(const Hash& hash) noexcept(false)
    -> p2p::bitcoin::message::HashField
{
    p2p::bitcoin::message::HashField output{};

    if (sizeof(output) != hash.size()) {
        throw std::runtime_error("Invalid hash");
    }

    std::memcpy(output.data(), hash.data(), output.size());

    return output;
}

auto Inventory::encode_type(const Type type) noexcept(false) -> std::uint32_t
{
    return map_.at(type);
}

auto Inventory::Serialize(Writer&& out) const noexcept -> bool
{
    try {
        const auto raw = BitcoinFormat{type_, hash_.Bytes()};

        return copy(reader(std::addressof(raw), sizeof(raw)), std::move(out));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::bitcoin
