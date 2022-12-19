// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin
{
class Inventory
{
public:
    enum class Type : std::uint8_t {
        None = 0,
        MsgTx = 1,
        MsgBlock = 2,
        MsgFilteredBlock = 3,
        MsgCmpctBlock = 4,
        MsgWitnessTx = 5,
        MsgWitnessBlock = 6,
        MsgFilteredWitnessBlock = 7
    };

    static const std::size_t EncodedSize;

    const Type type_;
    const FixedByteArray<32> hash_;

    static auto DisplayType(const Type type) noexcept -> UnallocatedCString;
    static constexpr auto size() noexcept -> std::size_t { return 36u; }

    auto DisplayType() const noexcept -> UnallocatedCString
    {
        return DisplayType(type_);
    }
    auto Serialize(Writer&& out) const noexcept -> bool;

    Inventory(const Type type, const Hash& hash) noexcept;
    Inventory(const void* payload, const std::size_t size) noexcept(false);
    Inventory(const ReadView payload) noexcept(false);
    Inventory() = delete;
    Inventory(const Inventory&) noexcept;
    Inventory(Inventory&&) noexcept;
    auto operator=(const Inventory&) -> Inventory& = delete;
    auto operator=(Inventory&&) -> Inventory& = delete;

    ~Inventory() = default;

private:
    using Map = UnallocatedMap<Type, std::uint32_t>;
    using ReverseMap = UnallocatedMap<std::uint32_t, Type>;

    struct BitcoinFormat {
        p2p::bitcoin::message::InventoryTypeField type_;
        p2p::bitcoin::message::HashField hash_;

        BitcoinFormat(const Type type, const Hash& hash) noexcept(false);
    };

    static const Map map_;
    static const ReverseMap reverse_map_;

    static auto decode_hash(
        const void* payload,
        const std::size_t size) noexcept(false) -> FixedByteArray<32>;
    static auto decode_type(
        const void* payload,
        const std::size_t size) noexcept(false) -> Type;
    static auto encode_type(const Type type) noexcept(false) -> std::uint32_t;
    static auto encode_hash(const Hash& hash) noexcept(false)
        -> p2p::bitcoin::message::HashField;
};
}  // namespace opentxs::blockchain::bitcoin
