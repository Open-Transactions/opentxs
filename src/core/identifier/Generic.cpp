// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "internal/core/identifier/Factory.hpp"  // IWYU pragma: associated
#include "opentxs/core/identifier/Generic.hpp"   // IWYU pragma: associated

#include <robin_hood.h>
#include <utility>

#include "core/identifier/IdentifierPrivate.hpp"
#include "internal/core/String.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/core/identifier/Types.hpp"

namespace opentxs::factory
{
auto Identifier(
    const identifier::Type type,
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*
{
    return Identifier(
        type, default_identifier_algorithm(), {}, std::move(alloc));
}

auto Identifier(
    const identifier::Type type,
    const identifier::Algorithm algorithm,
    const ReadView hash,
    identifier::Generic::allocator_type a) noexcept
    -> identifier::IdentifierPrivate*
{
    // TODO c++20
    auto alloc = alloc::PMR<identifier::IdentifierPrivate>{a};
    auto* imp = alloc.allocate(1_uz);
    alloc.construct(imp, algorithm, type, hash);

    return imp;
}

auto IdentifierInvalid(identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*
{
    return Identifier(
        identifier::Type::invalid,
        identifier::Algorithm::invalid,
        {},
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
    static const auto map =
        robin_hood::unordered_flat_map<identifier::Algorithm, std::size_t>{
            {identifier::Algorithm::sha256, 32_uz},
            {identifier::Algorithm::blake2b160, 20_uz},
            {identifier::Algorithm::blake2b256, 32_uz},
        };

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

auto print(Algorithm in) noexcept -> std::string_view
{
    static const auto map =
        robin_hood::unordered_flat_map<Algorithm, std::string_view>{
            {Algorithm::invalid, "invalid"sv},
            {Algorithm::sha256, "sha256"sv},
            {Algorithm::blake2b160, "blake2b160"sv},
            {Algorithm::blake2b256, "blake2b256"sv},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "unknown";
    }
}

auto print(Type in) noexcept -> std::string_view
{
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::invalid, "invalid"sv},
            {Type::generic, "generic"sv},
            {Type::nym, "nym"sv},
            {Type::notary, "notary"sv},
            {Type::unitdefinition, "unit definition"sv},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "unknown";
    }
}

auto translate(Type in) noexcept -> contract::Type
{
    static const auto map =
        robin_hood::unordered_flat_map<Type, contract::Type>{
            {Type::invalid, contract::Type::invalid},
            {Type::generic, contract::Type::invalid},
            {Type::nym, contract::Type::nym},
            {Type::notary, contract::Type::notary},
            {Type::unitdefinition, contract::Type::unit},
        };

    try {

        return map.at(in);
    } catch (...) {

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
    : Generic(factory::Identifier(identifier::Type::generic, std::move(alloc)))
{
}

Generic::Generic(const Generic& rhs, allocator_type alloc) noexcept
    : Generic(
          factory::Identifier(rhs.Type(), rhs.Algorithm(), rhs.Bytes(), alloc))
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
    imp_ = factory::Identifier(rhs.Type(), rhs.Algorithm(), rhs.Bytes(), alloc);

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
    OT_ASSERT(get_allocator() == rhs.get_allocator());

    std::swap(imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);
}

auto Generic::Type() const noexcept -> identifier::Type { return imp_->Type(); }

auto Generic::WriteInto() noexcept -> AllocateOutput
{
    return imp_->WriteInto();
}

auto Generic::zeroMemory() -> void { imp_->zeroMemory(); }

Generic::~Generic()
{
    if (nullptr != imp_) {
        // TODO c++20
        auto alloc = alloc::PMR<IdentifierPrivate>{get_allocator()};
        alloc.destroy(imp_);
        alloc.deallocate(imp_, 1);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::identifier
