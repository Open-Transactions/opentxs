// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/identifier/Factory.hpp"  // IWYU pragma: associated
#include "opentxs/identifier/Generic.hpp"        // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "internal/core/String.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/contract/ContractType.hpp"      // IWYU pragma: keep
#include "opentxs/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Algorithm.hpp"       // IWYU pragma: keep
#include "opentxs/identifier/IdentifierPrivate.hpp"
#include "opentxs/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto Identifier(
    const identifier::Type type,
    identifier::AccountSubtype accountSubtype,
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*
{
    return Identifier(
        type,
        default_identifier_algorithm(),
        {},
        accountSubtype,
        std::move(alloc));
}

auto Identifier(
    const identifier::Type type,
    const identifier::Algorithm algorithm,
    const ReadView hash,
    identifier::AccountSubtype accountSubtype,
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*
{
    return pmr::construct<identifier::IdentifierPrivate>(
        alloc, algorithm, type, hash, accountSubtype);
}

auto IdentifierInvalid(identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*
{
    return Identifier(
        identifier::Type::invalid,
        identifier::Algorithm::invalid,
        {},
        identifier::AccountSubtype::invalid_subtype,
        std::move(alloc));
}
}  // namespace opentxs::factory

namespace opentxs
{
using namespace std::literals;

auto default_identifier_algorithm() noexcept -> identifier::Algorithm
{
    return identifier::Algorithm::blake2b256;
}

auto identifier_expected_hash_bytes(identifier::Algorithm type) noexcept
    -> std::size_t
{
    using enum identifier::Algorithm;
    static constexpr auto map =
        frozen::make_unordered_map<identifier::Algorithm, std::size_t>({
            {sha256, 32_uz},
            {blake2b160, 20_uz},
            {blake2b256, 32_uz},
        });

    try {

        return map.at(type);
    } catch (...) {

        return 0_uz;
    }
}
}  // namespace opentxs
namespace opentxs::identifier
{
Generic::Generic(IdentifierPrivate* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Generic::Generic(allocator_type alloc) noexcept
    : Generic(factory::Identifier(
          identifier::Type::generic,
          identifier::AccountSubtype::invalid_subtype,
          std::move(alloc)))
{
}

Generic::Generic(const Generic& rhs, allocator_type alloc) noexcept
    : Generic(rhs.imp_->clone(alloc))
{
}

Generic::Generic(Generic&& rhs) noexcept
    : Generic(std::exchange(rhs.imp_, nullptr))
{
}

Generic::Generic(Generic&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Generic::operator=(const Generic& rhs) noexcept -> Generic&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Generic::operator=(Generic&& rhs) noexcept -> Generic&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Generic::Algorithm() const noexcept -> identifier::Algorithm
{
    return imp_->Algorithm();
}

auto Generic::asBase58(const api::Crypto& api) const noexcept
    -> UnallocatedCString
{
    return imp_->asBase58(api);
}

auto Generic::asBase58(const api::Crypto& api, alloc::Default alloc) const
    -> CString
{
    return imp_->asBase58(api, alloc);
}

auto Generic::asHex() const noexcept -> UnallocatedCString
{
    return imp_->asHex();
}

auto Generic::asHex(alloc::Default alloc) const noexcept -> CString
{
    return imp_->asHex(alloc);
}

auto Generic::Assign(const Data& source) noexcept -> bool
{
    return imp_->Assign(source);
}

auto Generic::Assign(const ReadView source) noexcept -> bool
{
    return imp_->Assign(source);
}

auto Generic::Assign(const void* data, const std::size_t size) noexcept -> bool
{
    return imp_->Assign(data, size);
}

auto Generic::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto Generic::clear() noexcept -> void { imp_->clear(); }

auto Generic::Concatenate(const ReadView in) noexcept -> bool
{
    return imp_->Concatenate(in);
}

auto Generic::Concatenate(const void* data, const std::size_t size) noexcept
    -> bool
{
    return imp_->Concatenate(data, size);
}

auto Generic::data() noexcept -> void* { return imp_->data(); }

auto Generic::data() const noexcept -> const void* { return imp_->data(); }

auto Generic::DecodeHex(const ReadView hex) noexcept -> bool
{
    return imp_->DecodeHex(hex);
}

auto Generic::empty() const noexcept -> bool { return imp_->empty(); }

auto Generic::Extract(
    const std::size_t amount,
    Data& output,
    const std::size_t pos) const noexcept -> bool
{
    return imp_->Extract(amount, output, pos);
}

auto Generic::Extract(std::uint16_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::Extract(std::uint32_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::Extract(std::uint64_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::Extract(std::uint8_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::get() const noexcept -> std::span<const std::byte>
{
    return imp_->get();
}

auto Generic::get() noexcept -> std::span<std::byte> { return imp_->get(); }

auto Generic::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Generic::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Generic::GetString(const api::Crypto& api, String& out) const noexcept
    -> void
{
    out.Release();
    out.Concatenate(asBase58(api));
}

auto Generic::Internal() const noexcept -> const internal::Identifier&
{
    return *imp_;
}

auto Generic::Internal() noexcept -> internal::Identifier& { return *imp_; }

auto Generic::IsNull() const noexcept -> bool { return imp_->IsNull(); }

auto Generic::Randomize(const std::size_t size) noexcept -> bool
{
    return imp_->Randomize(size);
}

auto Generic::resize(const std::size_t size) noexcept -> bool
{
    return imp_->resize(size);
}

auto Generic::Serialize(network::zeromq::Message& out) const noexcept -> void
{
    imp_->Serialize(out);
}

auto Generic::size() const noexcept -> std::size_t { return imp_->size(); }

auto Generic::swap(Generic& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

auto Generic::Type() const noexcept -> identifier::Type { return imp_->Type(); }

auto Generic::WriteInto() noexcept -> Writer { return imp_->WriteInto(); }

Generic::~Generic() { pmr::destroy(imp_); }
}  // namespace opentxs::identifier
