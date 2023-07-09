// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace identifier
{
namespace internal
{
class Identifier;
}  // namespace internal

class Generic;
class IdentifierPrivate;
}  // namespace identifier

class String;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identifier::Generic> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identifier::Generic& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto default_identifier_algorithm() noexcept
    -> identifier::Algorithm;
OPENTXS_EXPORT auto identifier_expected_hash_bytes(
    identifier::Algorithm type) noexcept -> std::size_t;
}  // namespace opentxs

namespace opentxs::identifier
{
/** An Identifier is basically a 256 bit hash value. This class makes it easy to
 * convert IDs back and forth to strings. */
class OPENTXS_EXPORT Generic : virtual public Allocated, virtual public Data
{
public:
    auto Algorithm() const noexcept -> identifier::Algorithm;
    auto asBase58(const api::Crypto& api) const -> UnallocatedCString;
    auto asBase58(const api::Crypto& api, alloc::Strategy alloc) const
        -> CString;
    auto asHex() const -> UnallocatedCString final;
    auto asHex(alloc::Strategy alloc) const -> CString final;
    auto at(const std::size_t position) const -> const std::byte& final;
    auto begin() const -> const_iterator final;
    auto Bytes() const noexcept -> ReadView final;
    auto cbegin() const -> const_iterator final;
    auto cend() const -> const_iterator final;
    auto data() const -> const void* final;
    auto empty() const -> bool final;
    auto end() const -> const_iterator final;
    auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const -> bool final;
    auto Extract(std::uint8_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto Extract(std::uint16_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto Extract(std::uint32_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto Extract(std::uint64_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto get_allocator() const noexcept -> allocator_type final;
    auto GetString(const api::Crypto& api, String& theStr) const noexcept
        -> void;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Identifier&;
    auto IsNull() const -> bool final;
    auto size() const -> std::size_t final;
    auto Type() const noexcept -> identifier::Type;

    auto Assign(const Data& source) noexcept -> bool final;
    auto Assign(const ReadView source) noexcept -> bool final;
    auto Assign(const void* data, const std::size_t size) noexcept
        -> bool final;
    auto at(const std::size_t position) -> std::byte& final;
    auto begin() -> iterator final;
    auto clear() noexcept -> void final;
    auto Concatenate(const ReadView) noexcept -> bool final;
    auto Concatenate(const void*, const std::size_t) noexcept -> bool final;
    auto data() -> void* final;
    auto DecodeHex(const ReadView hex) -> bool final;
    auto end() -> iterator final;
    auto Randomize(const std::size_t size) -> bool final;
    auto resize(const std::size_t) -> bool final;
    auto SetSize(const std::size_t) -> bool final;
    auto swap(Generic& rhs) noexcept -> void;
    auto WriteInto() noexcept -> Writer final;
    auto zeroMemory() -> void final;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Identifier&;

    OPENTXS_NO_EXPORT Generic(IdentifierPrivate* imp) noexcept;
    Generic(allocator_type alloc = {}) noexcept;
    Generic(const Generic& rhs, allocator_type alloc = {}) noexcept;
    Generic(Generic&& rhs) noexcept;
    Generic(Generic&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Generic& rhs) noexcept -> Generic&;
    auto operator=(Generic&& rhs) noexcept -> Generic&;

    ~Generic() override;

private:
    IdentifierPrivate* imp_;
};
}  // namespace opentxs::identifier
