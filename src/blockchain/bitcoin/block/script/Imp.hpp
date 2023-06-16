// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>

#include "blockchain/bitcoin/block/script/ScriptPrivate.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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
class PaymentCode;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::implementation
{
class Script final : public ScriptPrivate
{
public:
    using value_type = script::Element;

    static auto decode(const std::byte in) noexcept(false) -> script::OP;
    static auto is_direct_push(const script::OP opcode) noexcept(false)
        -> std::optional<std::size_t>;
    static auto is_push(const script::OP opcode) noexcept(false)
        -> std::optional<std::size_t>;
    static auto validate(std::span<const value_type> elements) noexcept -> bool;

    auto CalculateHash160(const api::Crypto& crypto, Writer&& output)
        const noexcept -> bool final;
    auto CalculateSize() const noexcept -> std::size_t final;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> ScriptPrivate* final
    {
        return pmr::clone_as<ScriptPrivate>(this, {alloc});
    }
    auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void final;
    auto get() const noexcept -> std::span<const value_type> final
    {
        return elements_;
    }
    auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void final;
    auto IsNotification(
        const std::uint8_t version,
        const PaymentCode& recipient) const noexcept -> bool final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto LikelyPubkeyHashes(const api::Crypto& crypto) const noexcept
        -> UnallocatedVector<ByteArray> final;
    auto M() const noexcept -> std::optional<std::uint8_t> final;
    auto MultisigPubkey(const std::size_t position) const noexcept
        -> std::optional<ReadView> final;
    auto N() const noexcept -> std::optional<std::uint8_t> final;
    auto Print() const noexcept -> UnallocatedCString final;
    auto Print(allocator_type alloc) const noexcept -> CString final;
    auto Pubkey() const noexcept -> std::optional<ReadView> final;
    auto PubkeyHash() const noexcept -> std::optional<ReadView> final;
    auto RedeemScript(allocator_type alloc) const noexcept
        -> block::Script final;
    auto Role() const noexcept -> script::Position final { return role_; }
    auto ScriptHash() const noexcept -> std::optional<ReadView> final;
    auto Serialize(Writer&& destination) const noexcept -> bool final;
    auto SigningSubscript(const blockchain::Type chain, alloc::Default alloc)
        const noexcept -> block::Script final;
    auto Type() const noexcept -> script::Pattern final { return type_; }
    auto Value(const std::size_t position) const noexcept
        -> std::optional<ReadView> final;

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> final
    {
        return make_deleter(this);
    }

    Script(
        const blockchain::Type chain,
        const script::Position role,
        Vector<value_type> elements,
        std::optional<std::size_t> size,
        allocator_type alloc) noexcept;
    Script() = delete;
    Script(const Script&, allocator_type alloc) noexcept;
    Script(const Script&) noexcept = delete;
    Script(Script&&) = delete;
    auto operator=(const Script&) -> Script& = delete;
    auto operator=(Script&&) -> Script& = delete;

    ~Script() final;

private:
    const blockchain::Type chain_;
    const script::Position role_;
    const Vector<value_type> elements_;
    const script::Pattern type_;
    mutable std::optional<std::size_t> size_;

    static auto bytes(const value_type& element) noexcept -> std::size_t;
    static auto bytes(std::span<const value_type> script) noexcept
        -> std::size_t;
    static auto is_data_push(const value_type& element) noexcept -> bool;
    static auto is_hash160(const value_type& element) noexcept -> bool;
    static auto is_public_key(const value_type& element) noexcept -> bool;
    static auto evaluate_data(std::span<const value_type> script) noexcept
        -> script::Pattern;
    static auto evaluate_multisig(std::span<const value_type> script) noexcept
        -> script::Pattern;
    static auto evaluate_pubkey(std::span<const value_type> script) noexcept
        -> script::Pattern;
    static auto evaluate_pubkey_hash(
        std::span<const value_type> script) noexcept -> script::Pattern;
    static auto evaluate_script_hash(
        std::span<const value_type> script) noexcept -> script::Pattern;
    static auto evaluate_segwit(std::span<const value_type> script) noexcept
        -> script::Pattern;
    static auto first_opcode(std::span<const value_type> script) noexcept
        -> script::OP;
    static auto get_type(
        const script::Position role,
        std::span<const value_type> script) noexcept -> script::Pattern;
    static auto last_opcode(std::span<const value_type> script) noexcept
        -> script::OP;
    static auto potential_data(std::span<const value_type> script) noexcept
        -> bool;
    static auto potential_multisig(std::span<const value_type> script) noexcept
        -> bool;
    static auto potential_pubkey(std::span<const value_type> script) noexcept
        -> bool;
    static auto potential_pubkey_hash(
        std::span<const value_type> script) noexcept -> bool;
    static auto potential_script_hash(
        std::span<const value_type> script) noexcept -> bool;
    static auto potential_segwit(std::span<const value_type> script) noexcept
        -> bool;
    static auto to_number(const script::OP opcode) noexcept -> std::uint8_t;
    static auto validate(
        const value_type& element,
        const bool checkForData = false) noexcept -> bool;

    auto get_data(const std::size_t position) const noexcept(false) -> ReadView;
    auto get_opcode(const std::size_t position) const noexcept(false)
        -> script::OP;
};
}  // namespace opentxs::blockchain::bitcoin::block::implementation
