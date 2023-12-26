// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
class Inventory
{
public:
    using Hash = opentxs::blockchain::block::Hash;

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

    Type type_;
    FixedByteArray<32> hash_;

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
    auto operator=(const Inventory&) noexcept -> Inventory&;
    auto operator=(Inventory&&) noexcept -> Inventory&;

    ~Inventory() = default;

private:
    struct BitcoinFormat {
        network::blockchain::bitcoin::message::InventoryTypeField type_;
        network::blockchain::bitcoin::message::HashField hash_;

        BitcoinFormat(const Type type, const Hash& hash) noexcept(false);
    };

    static auto decode_hash(
        const void* payload,
        const std::size_t size) noexcept(false) -> FixedByteArray<32>;
    static auto decode_type(
        const void* payload,
        const std::size_t size) noexcept(false) -> Type;
    static auto encode_type(const Type type) noexcept -> std::uint32_t;
    static auto encode_hash(const Hash& hash) noexcept(false)
        -> network::blockchain::bitcoin::message::HashField;
};
}  // namespace opentxs::network::blockchain::bitcoin
