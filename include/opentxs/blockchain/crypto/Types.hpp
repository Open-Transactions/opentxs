// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"
// IWYU pragma: no_include "opentxs/identifier/Account.hpp"
// IWYU pragma: no_include "opentxs/opentxs.hpp"
// IWYU pragma: no_include <coroutine>

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/token/Descriptor.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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
class Script;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Account;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
using opentxs::crypto::Bip32Index;

enum class AddressStyle : std::uint16_t;    // IWYU pragma: export
enum class Bip44Subchain : Bip32Index;      // IWYU pragma: export
enum class Bip44Type : Bip32Index;          // IWYU pragma: export
enum class HDProtocol : std::uint16_t;      // IWYU pragma: export
enum class SubaccountType : std::uint16_t;  // IWYU pragma: export
enum class Subchain : std::uint8_t;         // IWYU pragma: export

/// subaccount id, chain, index
using Key = std::tuple<identifier::Account, Subchain, Bip32Index>;
using Target = std::variant<blockchain::Type, token::Descriptor>;
using Batch = Vector<Bip32Index>;

OPENTXS_EXPORT auto base_chain(const Target&) noexcept -> blockchain::Type;
OPENTXS_EXPORT auto is_notification(Subchain) noexcept -> bool;
OPENTXS_EXPORT auto is_token(const Target&) noexcept -> bool;
OPENTXS_EXPORT auto operator==(const Key& lhs, const Key& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(const Key& lhs, const Key& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto operator==(const Target& lhs, const Target& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Target& lhs, const Target& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto print(AddressStyle) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(HDProtocol) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SubaccountType) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Subchain) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(const Key& key, const api::Crypto& api) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(
    const Key& key,
    const api::Crypto& api,
    alloc::Strategy alloc) noexcept -> CString;
OPENTXS_EXPORT auto print(const Target& target) noexcept -> UnallocatedCString;
OPENTXS_EXPORT auto print(const Target& target, alloc::Strategy alloc) noexcept
    -> CString;
OPENTXS_EXPORT auto target_to_unit(const Target&) noexcept -> opentxs::UnitType;

constexpr auto value(Bip44Subchain in) noexcept
{
    return static_cast<std::underlying_type_t<Bip44Subchain>>(in);
}
}  // namespace opentxs::blockchain::crypto

namespace opentxs::blockchain
{
using OutputBuilder = std::tuple<
    Amount,
    protocol::bitcoin::base::block::Script,
    UnallocatedSet<crypto::Key>>;
}  // namespace opentxs::blockchain

namespace opentxs
{
auto deserialize(const ReadView in) noexcept -> blockchain::crypto::Key;
auto serialize(const blockchain::crypto::Key& in) noexcept -> Space;
}  // namespace opentxs

// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::hash<opentxs::blockchain::crypto::Key> {
    auto operator()(const opentxs::blockchain::crypto::Key& data) const noexcept
        -> std::size_t;
};
template <>
struct std::hash<opentxs::blockchain::crypto::Target> {
    auto operator()(const opentxs::blockchain::crypto::Target& data)
        const noexcept -> std::size_t;
};
// NOLINTEND(cert-dcl58-cpp)
