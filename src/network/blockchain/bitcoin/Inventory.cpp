// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/Inventory.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain::bitcoin
{
static constexpr auto inv_type_to_int_ =
    frozen::make_unordered_map<Inventory::Type, std::uint32_t>({
        {Inventory::Type::None, 0},
        {Inventory::Type::MsgTx, 1},
        {Inventory::Type::MsgBlock, 2},
        {Inventory::Type::MsgFilteredBlock, 3},
        {Inventory::Type::MsgCmpctBlock, 4},
        {Inventory::Type::MsgWitnessTx, 16777280},
        {Inventory::Type::MsgWitnessBlock, 8388672},
        {Inventory::Type::MsgFilteredWitnessBlock, 25165888},
    });
static constexpr auto int_to_inv_type_ =
    frozen::invert_unordered_map(inv_type_to_int_);
}  // namespace opentxs::network::blockchain::bitcoin

namespace opentxs::network::blockchain::bitcoin
{
using namespace std::literals;

const std::size_t Inventory::EncodedSize{sizeof(BitcoinFormat)};

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
    network::blockchain::bitcoin::message::InventoryTypeField type{};

    if (EncodedSize != size) { throw std::runtime_error("Invalid payload"); }

    std::memcpy(&type, payload, sizeof(type));
    const auto& map = int_to_inv_type_;

    if (const auto* i = map.find(type.value()); map.end() != i) {

        return i->second;
    } else {

        return Type::None;
    }
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
    -> network::blockchain::bitcoin::message::HashField
{
    network::blockchain::bitcoin::message::HashField output{};

    if (sizeof(output) != hash.size()) {
        throw std::runtime_error("Invalid hash");
    }

    std::memcpy(output.data(), hash.data(), output.size());

    return output;
}

auto Inventory::encode_type(const Type type) noexcept -> std::uint32_t
{
    const auto& map = inv_type_to_int_;

    if (const auto* i = map.find(type); map.end() != i) {

        return i->second;
    } else {

        return 0u;
    }
}

auto Inventory::operator=(const Inventory& rhs) noexcept
    -> Inventory& = default;

auto Inventory::operator=(Inventory&& rhs) noexcept -> Inventory&
{
    using std::swap;
    swap(type_, rhs.type_);
    swap(hash_, rhs.hash_);

    return *this;
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
}  // namespace opentxs::network::blockchain::bitcoin
