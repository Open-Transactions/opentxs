// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Header.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
using namespace std::literals;

Header::Header(std::optional<Data> data) noexcept
    : data_(std::move(data))
{
    static_assert(sizeof(Data) == size_);
}

Header::Header(
    opentxs::blockchain::Type chain,
    message::Command command,
    std::size_t payload,
    ChecksumField checksum) noexcept
    : Header([&]() -> std::optional<Data> {
        try {
            using namespace opentxs::blockchain::params;

            return Data{
                MagicField{get(chain).P2PMagicBits()},
                SerializeCommand(command),
                PayloadSizeField{shorten(payload)},
                std::move(checksum)};
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();

            return std::nullopt;
        }
    }())
{
}

Header::Header(
    opentxs::blockchain::Type chain,
    message::Command command,
    const ReadView payload,
    const ReadView checksum) noexcept(false)
    : Header(chain, command, payload.size(), [&] {
        auto out = ChecksumField{};
        auto buf = preallocated(sizeof(out), std::addressof(out));

        if (false == copy(checksum, std::move(buf))) {

            throw std::runtime_error{"invalid checksum"};
        }

        return out;
    }())
{
}

Header::Header(
    const api::Session& api,
    opentxs::blockchain::Type chain,
    message::Command command,
    const ReadView payload) noexcept
    : Header(
          chain,
          command,
          payload.size(),
          calculate_checksum(api, chain, payload))
{
}

Header::Header(ReadView in) noexcept
    : Header([&]() -> std::optional<Data> {
        if (in.size() == size_) {
            auto out = Data{};
            copy(in, preallocated(size_, std::addressof(out)));

            return out;
        } else {

            return std::nullopt;
        }
    }())
{
}

Header::Header(Header&& rhs) noexcept
    : data_(std::move(rhs.data_))
{
}

auto Header::calculate_checksum(
    const api::Session& api,
    opentxs::blockchain::Type chain,
    const ReadView payload) noexcept -> ChecksumField
{
    if (payload.empty()) {
        static constexpr auto blank = ChecksumField{
            std::byte{0x5d},
            std::byte{0xf6},
            std::byte{0xe0},
            std::byte{0xe2},
        };

        return blank;
    } else {
        auto out = ChecksumField{};
        P2PMessageHash(
            api.Crypto(),
            chain,
            payload,
            preallocated(sizeof(out), std::addressof(out)));

        return out;
    }
}

auto Header::Checksum() const noexcept -> ReadView
{
    if (data_.has_value()) {

        return checksum(*data_);
    } else {

        return {};
    }
}

auto Header::checksum(const Data& data) noexcept -> ReadView
{
    const auto& cs = data.checksum_;

    return {reinterpret_cast<const char*>(std::addressof(cs)), sizeof(cs)};
}

auto Header::Command() const noexcept -> message::Command
{
    if (data_.has_value()) {

        return GetCommand(data_->command_);
    } else {

        return message::Command::unknown;
    }
}

auto Header::Describe() const noexcept -> ReadView
{
    if (data_.has_value()) {

        return reader(data_->command_);
    } else {

        return "invalid message"sv;
    }
}

auto Header::IsValid(opentxs::blockchain::Type chain) const noexcept -> bool
{
    if (data_.has_value()) {
        using namespace opentxs::blockchain::params;

        return data_->magic_.value() == get(chain).P2PMagicBits();
    } else {

        return false;
    }
}

auto Header::PayloadSize() const noexcept -> std::size_t
{
    if (data_.has_value()) {
        static_assert(sizeof(PayloadSizeField) <= sizeof(std::size_t));

        return data_->length_.value();
    } else {

        return 0_uz;
    }
}

auto Header::reader(const Data& data) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(std::addressof(data)), size_};
}

auto Header::reader(const ChecksumField& data) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(data.data()), data.size()};
}

auto Header::reader(const CommandField& data) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(data.data()), data.size()};
}

auto Header::Serialize(Writer&& out) const noexcept -> bool
{
    if (data_.has_value()) {

        return copy(reader(*data_), std::move(out));
    } else {

        return false;
    }
}

auto Header::Verify(
    const api::Session& api,
    opentxs::blockchain::Type chain,
    const ReadView payload) const noexcept -> bool
{
    if (data_.has_value()) {
        const auto calculated = calculate_checksum(api, chain, payload);

        return checksum(*data_) == reader(calculated);
    } else {

        return false;
    }
}
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
