// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Armored;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
class Data : virtual public opentxs::Data
{
public:
    using Vector = UnallocatedVector<std::uint8_t>;

    auto operator==(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator!=(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator<(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator>(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator<=(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator>=(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator+=(const opentxs::Data& rhs) -> Data& final;
    auto operator+=(const std::uint8_t rhs) -> Data& final;
    auto operator+=(const std::uint16_t rhs) -> Data& final;
    auto operator+=(const std::uint32_t rhs) -> Data& final;
    auto operator+=(const std::uint64_t rhs) -> Data& final;

    auto asHex() const -> UnallocatedCString final;
    auto at(const std::size_t position) const -> const std::byte& final
    {
        return reinterpret_cast<const std::byte&>(data_.at(position));
    }
    auto begin() const -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto Bytes() const noexcept -> ReadView final
    {
        return ReadView{
            reinterpret_cast<const char*>(data_.data()), data_.size()};
    }
    auto cbegin() const -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const -> const_iterator final
    {
        return const_iterator(this, data_.size());
    }
    auto empty() const -> bool final { return data_.empty(); }
    auto data() const -> const void* final { return data_.data(); }
    auto end() const -> const_iterator final
    {
        return const_iterator(this, data_.size());
    }
    auto Extract(
        const std::size_t amount,
        opentxs::Data& output,
        const std::size_t pos) const -> bool final;
    auto Extract(std::uint8_t& output, const std::size_t pos) const
        -> bool final;
    auto Extract(std::uint16_t& output, const std::size_t pos) const
        -> bool final;
    auto Extract(std::uint32_t& output, const std::size_t pos) const
        -> bool final;
    auto Extract(std::uint64_t& output, const std::size_t pos) const
        -> bool final;
    auto IsEmpty() const -> bool final { return empty(); }
    auto IsNull() const -> bool final;
    auto GetPointer() const -> const void* final { return data_.data(); }
    auto GetSize() const -> std::size_t final { return size(); }
    auto size() const -> std::size_t final { return data_.size(); }

    auto Assign(const opentxs::Data& source) noexcept -> bool final
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(ReadView source) noexcept -> bool final
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(const void* data, const std::size_t size) noexcept
        -> bool override;
    auto at(const std::size_t position) -> std::byte& final
    {
        return reinterpret_cast<std::byte&>(data_.at(position));
    }
    auto begin() -> iterator final { return iterator(this, 0); }
    auto Concatenate(const ReadView data) noexcept -> bool final
    {
        return Concatenate(data.data(), data.size());
    }
    auto Concatenate(const void* data, const std::size_t size) noexcept
        -> bool override;
    auto data() -> void* final { return data_.data(); }
    auto DecodeHex(const UnallocatedCString& hex) -> bool final;
    auto end() -> iterator final { return iterator(this, data_.size()); }
    auto Randomize(const std::size_t size) -> bool override;
    void Release() final;
    void resize(const std::size_t size) final { data_.resize(size); }
    void SetSize(const std::size_t size) final;
    auto str() const -> UnallocatedCString override;
    void swap(opentxs::Data&& rhs) final;
    auto WriteInto() noexcept -> AllocateOutput final;
    void zeroMemory() final;

    ~Data() override = default;

protected:
    Vector data_;

    void Initialize();

    Data() noexcept;
    Data(const void* data, std::size_t size) noexcept;
    Data(const opentxs::Armored& source) noexcept;
    Data(const Vector& sourceVector) noexcept;
    Data(Vector&& data) noexcept;
    Data(const UnallocatedVector<std::byte>& sourceVector) noexcept;

private:
    friend opentxs::Data;

    auto clone() const -> Data* override { return new Data(this->data_); }

    auto check_sub(const std::size_t pos, const std::size_t target) const
        -> bool;
    auto concatenate(const Vector& data) -> void;
    auto spaceship(const opentxs::Data& rhs) const noexcept -> int;

    Data(const Data& rhs) = delete;
    Data(Data&& rhs) = delete;
    auto operator=(const Data& rhs) -> Data& = delete;
    auto operator=(Data&& rhs) -> Data& = delete;
};
}  // namespace opentxs::implementation
