// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>
#include <vector>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class Armored;
class Data;

using OTData = Pimpl<Data>;
}  // namespace opentxs

namespace opentxs
{
OPENTXS_EXPORT auto operator==(const OTData& lhs, const Data& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator!=(const OTData& lhs, const Data& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<(const OTData& lhs, const Data& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator>(const OTData& lhs, const Data& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=(const OTData& lhs, const Data& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator>=(const OTData& lhs, const Data& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator+=(OTData& lhs, const OTData& rhs) -> OTData&;
OPENTXS_EXPORT auto operator+=(OTData& lhs, const std::uint8_t rhs) -> OTData&;
OPENTXS_EXPORT auto operator+=(OTData& lhs, const std::uint16_t rhs) -> OTData&;
OPENTXS_EXPORT auto operator+=(OTData& lhs, const std::uint32_t rhs) -> OTData&;
OPENTXS_EXPORT auto operator+=(OTData& lhs, const std::uint64_t rhs) -> OTData&;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Data
{
public:
    enum class Mode : bool { Hex = true, Raw = false };

    using iterator = opentxs::iterator::Bidirectional<Data, std::byte>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Data, const std::byte>;

    static auto Factory() -> Pimpl<opentxs::Data>;
    static auto Factory(const Data& rhs) -> Pimpl<opentxs::Data>;
    static auto Factory(const void* data, std::size_t size)
        -> Pimpl<opentxs::Data>;
    static auto Factory(const Armored& source) -> OTData;
    static auto Factory(const std::pmr::vector<unsigned char>& source)
        -> OTData;
    static auto Factory(const std::pmr::vector<std::byte>& source) -> OTData;
    static auto Factory(const network::zeromq::Frame& message) -> OTData;
    static auto Factory(const std::uint8_t in) -> OTData;
    /// Bytes will be stored in big endian order
    static auto Factory(const std::uint16_t in) -> OTData;
    /// Bytes will be stored in big endian order
    static auto Factory(const std::uint32_t in) -> OTData;
    /// Bytes will be stored in big endian order
    static auto Factory(const std::uint64_t in) -> OTData;
    static auto Factory(const std::string in, const Mode mode) -> OTData;

    virtual auto operator==(const Data& rhs) const noexcept -> bool = 0;
    virtual auto operator!=(const Data& rhs) const noexcept -> bool = 0;
    virtual auto operator<(const Data& rhs) const noexcept -> bool = 0;
    virtual auto operator>(const Data& rhs) const noexcept -> bool = 0;
    virtual auto operator<=(const Data& rhs) const noexcept -> bool = 0;
    virtual auto operator>=(const Data& rhs) const noexcept -> bool = 0;
    virtual auto asHex() const -> std::string = 0;
    virtual auto at(const std::size_t position) const -> const std::byte& = 0;
    virtual auto begin() const -> const_iterator = 0;
    virtual auto Bytes() const noexcept -> ReadView = 0;
    virtual auto cbegin() const -> const_iterator = 0;
    virtual auto cend() const -> const_iterator = 0;
    virtual auto data() const -> const void* = 0;
    virtual auto empty() const -> bool = 0;
    virtual auto end() const -> const_iterator = 0;
    virtual auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const -> bool = 0;
    virtual auto Extract(std::uint8_t& output, const std::size_t pos = 0) const
        -> bool = 0;
    virtual auto Extract(std::uint16_t& output, const std::size_t pos = 0) const
        -> bool = 0;
    virtual auto Extract(std::uint32_t& output, const std::size_t pos = 0) const
        -> bool = 0;
    virtual auto Extract(std::uint64_t& output, const std::size_t pos = 0) const
        -> bool = 0;
    [[deprecated]] virtual auto GetPointer() const -> const void* = 0;
    [[deprecated]] virtual auto GetSize() const -> std::size_t = 0;
    [[deprecated]] virtual auto IsEmpty() const -> bool = 0;
    virtual auto IsNull() const -> bool = 0;
    virtual auto size() const -> std::size_t = 0;
    virtual auto str() const -> std::string = 0;

    virtual auto operator+=(const Data& rhs) -> Data& = 0;
    virtual auto operator+=(const std::uint8_t rhs) -> Data& = 0;
    /// Bytes will be stored in big endian order
    virtual auto operator+=(const std::uint16_t rhs) -> Data& = 0;
    /// Bytes will be stored in big endian order
    virtual auto operator+=(const std::uint32_t rhs) -> Data& = 0;
    /// Bytes will be stored in big endian order
    virtual auto operator+=(const std::uint64_t rhs) -> Data& = 0;
    virtual auto Assign(const Data& source) noexcept -> bool = 0;
    virtual auto Assign(const ReadView source) noexcept -> bool = 0;
    virtual auto Assign(const void* data, const std::size_t size) noexcept
        -> bool = 0;
    virtual auto at(const std::size_t position) -> std::byte& = 0;
    virtual auto begin() -> iterator = 0;
    virtual auto data() -> void* = 0;
    virtual auto DecodeHex(const std::string& hex) -> bool = 0;
    virtual auto Concatenate(const ReadView data) noexcept -> bool = 0;
    virtual auto Concatenate(const void* data, const std::size_t size) noexcept
        -> bool = 0;
    virtual auto end() -> iterator = 0;
    virtual auto Randomize(const std::size_t size) -> bool = 0;
    virtual void Release() = 0;
    virtual void resize(const std::size_t size) = 0;
    virtual void SetSize(const std::size_t size) = 0;
    virtual void swap(Data&& rhs) = 0;
    virtual auto WriteInto() noexcept -> AllocateOutput = 0;
    virtual void zeroMemory() = 0;

    virtual ~Data() = default;

protected:
    Data() = default;

private:
    friend OTData;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const -> Data* = 0;
#ifdef _WIN32
private:
#endif

    Data(const Data& rhs) = delete;
    Data(Data&& rhs) = delete;
    auto operator=(const Data& rhs) -> Data& = delete;
    auto operator=(Data&& rhs) -> Data& = delete;
};
}  // namespace opentxs

namespace std
{
template <>
struct OPENTXS_EXPORT less<opentxs::OTData> {
    auto operator()(const opentxs::OTData& lhs, const opentxs::OTData& rhs)
        const -> bool;
};
}  // namespace std
