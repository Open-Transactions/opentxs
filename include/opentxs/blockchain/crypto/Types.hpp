// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string_view>
#include <tuple>
#include <typeindex>
#include <utility>
#include <variant>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace crypto
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
enum class AddressStyle : std::uint16_t;
enum class HDProtocol : std::uint16_t;
enum class SubaccountType : std::uint16_t;
enum class Subchain : std::uint8_t;

/// transaction id, output index
using Coin = std::pair<UnallocatedCString, std::size_t>;
using ECKey = std::shared_ptr<const opentxs::crypto::key::EllipticCurve>;
using HDKey = std::shared_ptr<const opentxs::crypto::key::HD>;
/// account id, chain, index
using Key = std::tuple<identifier::Generic, Subchain, Bip32Index>;
using Activity = std::tuple<Coin, Key, Amount>;

OPENTXS_EXPORT auto is_notification(Subchain) noexcept -> bool;
OPENTXS_EXPORT auto print(HDProtocol) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SubaccountType) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Subchain) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(const Key& key) noexcept -> UnallocatedCString;
}  // namespace opentxs::blockchain::crypto

namespace std
{
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct hash<opentxs::blockchain::crypto::Key> {
    auto operator()(const opentxs::blockchain::crypto::Key& data) const noexcept
        -> std::size_t;
};
// NOLINTEND(cert-dcl58-cpp)
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto operator==(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool;
auto deserialize(const ReadView in) noexcept -> blockchain::crypto::Key;
auto serialize(const blockchain::crypto::Key& in) noexcept -> Space;
}  // namespace opentxs
