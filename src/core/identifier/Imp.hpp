// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/identifier/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Type.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace proto
{
class Identifier;
}  // namespace proto

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identifier
{
class Generic::Imp final : virtual public opentxs::Allocated
{
public:
    const identifier::Algorithm algorithm_;
    const identifier::Type type_;
    Generic* parent_;

    using Vector = opentxs::Vector<std::byte>;

    auto Algorithm() const noexcept -> identifier::Algorithm
    {
        return algorithm_;
    }
    auto asBase58(const api::Crypto& api) const -> UnallocatedCString;
    auto asBase58(const api::Crypto& api, alloc::Default alloc) const
        -> CString;
    auto asHex() const -> UnallocatedCString;
    auto asHex(alloc::Default alloc) const -> CString;
    auto at(const std::size_t position) const -> const std::byte&
    {
        return reinterpret_cast<const std::byte&>(data_.at(position));
    }
    auto begin() const -> const_iterator;
    auto Bytes() const noexcept -> ReadView
    {
        return ReadView{
            reinterpret_cast<const char*>(data_.data()), data_.size()};
    }
    auto cbegin() const -> const_iterator;
    auto cend() const -> const_iterator;
    auto empty() const -> bool { return data_.empty(); }
    auto data() const -> const void* { return data_.data(); }
    auto end() const -> const_iterator;
    auto Extract(
        const std::size_t amount,
        opentxs::Data& output,
        const std::size_t pos) const -> bool;
    auto Extract(std::uint8_t& output, const std::size_t pos) const -> bool;
    auto Extract(std::uint16_t& output, const std::size_t pos) const -> bool;
    auto Extract(std::uint32_t& output, const std::size_t pos) const -> bool;
    auto Extract(std::uint64_t& output, const std::size_t pos) const -> bool;
    auto get_allocator() const noexcept -> allocator_type final
    {
        return data_.get_allocator();
    }
    auto IsNull() const -> bool;
    auto size() const -> std::size_t { return data_.size(); }
    auto Type() const noexcept -> identifier::Type { return type_; }

    auto Assign(const opentxs::Data& source) noexcept -> bool
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(ReadView source) noexcept -> bool
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(const void* data, const std::size_t size) noexcept -> bool;
    auto at(const std::size_t position) -> std::byte&
    {
        return reinterpret_cast<std::byte&>(data_.at(position));
    }
    auto begin() -> iterator;
    auto clear() noexcept -> void { data_.clear(); }
    auto Concatenate(const ReadView data) noexcept -> bool
    {
        return Concatenate(data.data(), data.size());
    }
    auto Concatenate(const void* data, const std::size_t size) noexcept -> bool;
    auto data() -> void* { return data_.data(); }
    auto DecodeHex(const std::string_view hex) -> bool;
    auto end() -> iterator;
    auto operator+=(const opentxs::Data& rhs) noexcept -> Generic&;
    auto operator+=(const ReadView rhs) noexcept -> Generic&;
    auto operator+=(const std::uint8_t rhs) noexcept -> Generic&;
    auto operator+=(const std::uint16_t rhs) noexcept -> Generic&;
    auto operator+=(const std::uint32_t rhs) noexcept -> Generic&;
    auto operator+=(const std::uint64_t rhs) noexcept -> Generic&;
    auto Randomize(const std::size_t size) -> bool;
    auto resize(const std::size_t size) -> bool;
    auto Serialize(proto::Identifier& out) const noexcept -> bool;
    auto SetSize(const std::size_t size) -> bool;
    auto WriteInto() noexcept -> AllocateOutput;
    auto zeroMemory() -> void;

    Imp() = delete;
    Imp(const identifier::Algorithm algorithm,
        const identifier::Type type,
        const ReadView hash,
        allocator_type alloc = {}) noexcept;
    Imp(const Imp& rhs) = delete;
    Imp(Imp&& rhs) = delete;
    auto operator=(const Imp& rhs) -> Imp& = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    ~Imp() override = default;

private:
    static constexpr auto proto_version_ = VersionNumber{1};

    Vector data_;

    auto check_sub(const std::size_t pos, const std::size_t target) const
        -> bool;
    auto concatenate(const Vector& data) -> void;
};
}  // namespace opentxs::identifier
