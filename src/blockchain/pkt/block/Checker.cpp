// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "blockchain/pkt/block/Checker.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::pkt::block
{
Checker::Checker(const api::Crypto& crypto, blockchain::Type type) noexcept
    : bitcoin::block::Checker(crypto, type)
{
}

auto Checker::find_payload() noexcept -> bool
{
    using opentxs::network::blockchain::bitcoin::DecodeCompactSize;

    try {
        const auto check = [&](auto message, auto required = 1_uz) {
            const auto target = std::max(1_uz, required);

            if (data_.empty() || (data_.size() < target)) {
                const auto error = CString{"data_ too short: "}.append(message);

                throw std::runtime_error(error.c_str());
            }
        };
        const auto getSize = [&](auto message) {
            if (auto out = DecodeCompactSize(data_); out.has_value()) {

                return out.value();
            } else {
                const auto error =
                    CString{"failed to decode: "}.append(message);

                throw std::runtime_error(error.c_str());
            }
        };

        while (true) {
            // TODO presumably the generation transaction contains a commitment
            // of some type for this data that should be checked
            constexpr auto proofType = 1_uz;
            check("proof type", proofType);
            const auto type = static_cast<std::byte>(data_[0]);
            data_.remove_prefix(proofType);
            const auto proofBytes = getSize("proof size");
            check("proof", proofBytes);
            data_.remove_prefix(proofBytes);
            constexpr auto terminalType = std::byte{0x0};

            if (type == terminalType) { break; }
        }

        return bitcoin::block::Checker::find_payload();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::pkt::block
