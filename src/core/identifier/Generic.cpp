// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/identifier/Factory.hpp"  // IWYU pragma: associated
#include "opentxs/core/identifier/Generic.hpp"   // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "core/identifier/IdentifierPrivate.hpp"
#include "internal/core/String.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/ContractType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Algorithm.hpp"       // IWYU pragma: keep
#include "opentxs/core/identifier/Type.hpp"            // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"
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
    identifier::Generic::allocator_type a) noexcept
    -> identifier::IdentifierPrivate*
{
    // TODO c++20
    auto alloc = alloc::PMR<identifier::IdentifierPrivate>{a};
    auto* imp = alloc.allocate(1_uz);
    alloc.construct(imp, algorithm, type, hash, accountSubtype);

    return imp;
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
using namespace std::literals;

auto print(AccountSubtype in) noexcept -> std::string_view
{
    using enum AccountSubtype;
    static constexpr auto map =
        frozen::make_unordered_map<AccountSubtype, std::string_view>({
            {invalid_subtype, "invalid_subtype"sv},
            {custodial_account, "custodial_account"sv},
            {blockchain_account, "blockchain_account"sv},
            {blockchain_subaccount, "blockchain_subaccount"sv},
            {blockchain_subchain, "blockchain_subchain"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "invalid_subtype";
    }
}

auto print(Algorithm in) noexcept -> std::string_view
{
    using enum Algorithm;
    static constexpr auto map =
        frozen::make_unordered_map<Algorithm, std::string_view>({
            {invalid, "invalid"sv},
            {sha256, "sha256"sv},
            {blake2b160, "blake2b160"sv},
            {blake2b256, "blake2b256"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown";
    }
}

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {invalid, "invalid"sv},
            {generic, "generic"sv},
            {nym, "nym"sv},
            {notary, "notary"sv},
            {unitdefinition, "unit definition"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown";
    }
}

auto translate(Type in) noexcept -> contract::Type
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, contract::Type>({
            {invalid, contract::Type::invalid},
            {generic, contract::Type::invalid},
            {nym, contract::Type::nym},
            {notary, contract::Type::notary},
            {unitdefinition, contract::Type::unit},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::Type::invalid;
    }
}
}  // namespace opentxs::identifier

namespace opentxs::identifier
{
Generic::Generic(IdentifierPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
}

Generic::Generic(allocator_type alloc) noexcept
    : Generic(factory::Identifier(
          identifier::Type::generic,
          identifier::AccountSubtype::invalid_subtype,
          std::move(alloc)))
{
}

Generic::Generic(const Generic& rhs, allocator_type alloc) noexcept
    : Generic(factory::Identifier(
          rhs.Type(),
          rhs.Algorithm(),
          rhs.Bytes(),
          rhs.imp_->account_subtype_,
          alloc))
{
}

Generic::Generic(Generic&& rhs) noexcept
    : Generic(rhs.get_allocator())
{
    swap(rhs);
}

Generic::Generic(Generic&& rhs, allocator_type alloc) noexcept
    : Generic(alloc)
{
    operator=(std::move(rhs));
}

auto Generic::operator=(const Generic& rhs) noexcept -> Generic&
{
    auto alloc = alloc::PMR<IdentifierPrivate>{get_allocator()};
    auto* old = imp_;
    imp_ = factory::Identifier(
        rhs.Type(),
        rhs.Algorithm(),
        rhs.Bytes(),
        rhs.imp_->account_subtype_,
        alloc);

    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
    // TODO c++20
    alloc.destroy(old);
    alloc.deallocate(old, 1);

    return *this;
}

auto Generic::operator=(Generic&& rhs) noexcept -> Generic&
{
    if (get_allocator() == rhs.get_allocator()) {
        swap(rhs);

        return *this;
    } else {

        return operator=(const_cast<const Generic&>(rhs));
    }
}

auto Generic::Algorithm() const noexcept -> identifier::Algorithm
{
    return imp_->Algorithm();
}

auto Generic::asBase58(const api::Crypto& api) const -> UnallocatedCString
{
    return imp_->asBase58(api);
}

auto Generic::asBase58(const api::Crypto& api, alloc::Default alloc) const
    -> CString
{
    return imp_->asBase58(api, alloc);
}

auto Generic::asHex() const -> UnallocatedCString { return imp_->asHex(); }

auto Generic::asHex(alloc::Default alloc) const -> CString
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

auto Generic::at(const std::size_t position) -> std::byte&
{
    return imp_->at(position);
}

auto Generic::at(const std::size_t position) const -> const std::byte&
{
    return imp_->at(position);
}

auto Generic::begin() -> iterator { return imp_->begin(); }

auto Generic::begin() const -> const_iterator { return cbegin(); }

auto Generic::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto Generic::cbegin() const -> const_iterator { return imp_->cbegin(); }

auto Generic::cend() const -> const_iterator { return imp_->cend(); }

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

auto Generic::data() -> void* { return imp_->data(); }

auto Generic::data() const -> const void* { return imp_->data(); }

auto Generic::DecodeHex(const ReadView hex) -> bool
{
    return imp_->DecodeHex(hex);
}

auto Generic::empty() const -> bool { return imp_->empty(); }

auto Generic::end() -> iterator { return imp_->end(); }

auto Generic::end() const -> const_iterator { return cend(); }

auto Generic::Extract(
    const std::size_t amount,
    Data& output,
    const std::size_t pos) const -> bool
{
    return imp_->Extract(amount, output, pos);
}

auto Generic::Extract(std::uint16_t& output, const std::size_t pos) const
    -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::Extract(std::uint32_t& output, const std::size_t pos) const
    -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::Extract(std::uint64_t& output, const std::size_t pos) const
    -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::Extract(std::uint8_t& output, const std::size_t pos) const -> bool
{
    return imp_->Extract(output, pos);
}

auto Generic::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Generic::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
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

auto Generic::IsNull() const -> bool { return imp_->IsNull(); }

auto Generic::Randomize(const std::size_t size) -> bool
{
    return imp_->Randomize(size);
}

auto Generic::resize(const std::size_t size) -> bool
{
    return imp_->resize(size);
}

auto Generic::SetSize(const std::size_t size) -> bool
{
    return imp_->SetSize(size);
}

auto Generic::size() const -> std::size_t { return imp_->size(); }

auto Generic::swap(Generic& rhs) noexcept -> void
{
    pmr_swap(*this, rhs, imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);
}

auto Generic::Type() const noexcept -> identifier::Type { return imp_->Type(); }

auto Generic::WriteInto() noexcept -> Writer { return imp_->WriteInto(); }

auto Generic::zeroMemory() -> void { imp_->zeroMemory(); }

Generic::~Generic() { pmr_delete(imp_); }
}  // namespace opentxs::identifier
