// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class Header
{
public:
    static constexpr auto size_ = 24_uz;

    auto Checksum() const noexcept -> ReadView;
    auto Command() const noexcept -> message::Command;
    auto Describe() const noexcept -> ReadView;
    [[nodiscard]] auto IsValid(opentxs::blockchain::Type chain) const noexcept
        -> bool;
    auto PayloadSize() const noexcept -> std::size_t;
    auto Serialize(Writer&& out) const noexcept -> bool;
    auto Verify(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        const ReadView payload) const noexcept -> bool;

    Header(ReadView serialized) noexcept;
    Header(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        message::Command command,
        const ReadView payload) noexcept;
    Header(
        opentxs::blockchain::Type chain,
        message::Command command,
        const ReadView payload,
        const ReadView checksum) noexcept(false);
    Header(const Header&) = delete;
    Header(Header&& rhs) noexcept;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;

    ~Header() = default;

private:
    struct Data {
        MagicField magic_{};
        CommandField command_{};
        PayloadSizeField length_{};
        ChecksumField checksum_{};
    };

    std::optional<Data> data_;

    static auto calculate_checksum(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        const ReadView payload) noexcept -> ChecksumField;
    static auto checksum(const Data& data) noexcept -> ReadView;
    static auto reader(const Data& data) noexcept -> ReadView;
    static auto reader(const ChecksumField& data) noexcept -> ReadView;
    static auto reader(const CommandField& data) noexcept -> ReadView;

    Header(
        opentxs::blockchain::Type chain,
        message::Command command,
        std::size_t payload,
        ChecksumField checksum) noexcept;
    Header(std::optional<Data> data) noexcept;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
