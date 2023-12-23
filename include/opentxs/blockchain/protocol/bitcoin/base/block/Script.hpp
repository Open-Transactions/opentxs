// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
namespace internal
{
class Script;
}  // namespace internal

namespace script
{
struct Element;
}  // namespace script

class ScriptPrivate;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

class PaymentCode;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class OPENTXS_EXPORT Script : virtual public opentxs::Allocated
{
public:
    using value_type = script::Element;

    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Script&;

    operator bool() const noexcept { return IsValid(); }
    operator std::span<const value_type>() const noexcept { return get(); }

    auto CalculateHash160(const api::Crypto& crypto, Writer&& output)
        const noexcept -> bool;
    auto CalculateSize() const noexcept -> std::size_t;
    auto get() const noexcept -> std::span<const value_type>;
    auto get_allocator() const noexcept -> allocator_type final;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Script&;
    auto IsNotification(
        const std::uint8_t version,
        const PaymentCode& recipient) const noexcept -> bool;
    auto IsValid() const noexcept -> bool;
    /// Value only present for Multisig patterns
    auto M() const noexcept -> std::optional<std::uint8_t>;
    /// Value only present for Multisig patterns, 0 indexed
    auto MultisigPubkey(const std::size_t position) const noexcept
        -> std::optional<ReadView>;
    /// Value only present for Multisig patterns
    auto N() const noexcept -> std::optional<std::uint8_t>;
    auto Print() const noexcept -> UnallocatedCString;
    auto Print(allocator_type alloc) const noexcept -> CString;
    /// Value only present for PayToPubkey and PayToTaproot patterns
    auto Pubkey() const noexcept -> std::optional<ReadView>;
    /// Value only present for PayToPubkeyHash and PayToWitnessPubkeyHash
    /// patterns
    auto PubkeyHash() const noexcept -> std::optional<ReadView>;
    /// Value only present for input scripts which spend PayToScriptHash outputs
    auto RedeemScript(allocator_type alloc) const noexcept -> Script;
    auto Role() const noexcept -> script::Position;
    /// Value only present for PayToScriptHash and PayToWitnessScriptHash
    /// patterns
    auto ScriptHash() const noexcept -> std::optional<ReadView>;
    auto Serialize(Writer&& destination) const noexcept -> bool;
    auto Type() const noexcept -> script::Pattern;
    /// Value only present for NullData patterns, 0 indexed
    auto Value(const std::size_t position) const noexcept
        -> std::optional<ReadView>;

    auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Script&;
    auto swap(Script& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Script(ScriptPrivate* imp) noexcept;
    Script(allocator_type alloc = {}) noexcept;
    Script(const Script& rhs, allocator_type alloc = {}) noexcept;
    Script(Script&& rhs) noexcept;
    Script(Script&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Script& rhs) noexcept -> Script&;
    auto operator=(Script&& rhs) noexcept -> Script&;

    ~Script() override;

protected:
    friend ScriptPrivate;

    ScriptPrivate* imp_;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
