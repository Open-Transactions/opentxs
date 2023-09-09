// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Types.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/node/Types.hpp"   // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>

#include "internal/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Funding.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/node/SendResult.hpp"     // IWYU pragma: keep
#include "opentxs/blockchain/node/TxoState.hpp"       // IWYU pragma: keep
#include "opentxs/blockchain/node/TxoTag.hpp"         // IWYU pragma: keep

namespace opentxs::blockchain::node
{
auto print(Funding in) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum Funding;
    static constexpr auto map =
        frozen::make_unordered_map<Funding, std::string_view>({
            {Default, "default"sv},
            {SweepAccount, "sweep account"sv},
            {SweepSubaccount, "sweep subaccount"sv},
            {SweepKey, "sweep key"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown Funding"sv;
    }
}

auto print(SendResult code) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum SendResult;
    static constexpr auto map =
        frozen::make_unordered_map<SendResult, std::string_view>({
            {InvalidSenderNym, "invalid sender nym"sv},
            {MissingRecipients, "no recipients specified in spend"sv},
            {SerializationError, "internal serialization error"sv},
            {InsufficientConfirmedFunds,
             "try again after pending transactions are confirmed"sv},
            {DatabaseError, "database error"sv},
            {DuplicateProposal, "duplicate spend proposal"sv},
            {OutputCreationError, "failed to create transaction outputs"sv},
            {ChangeError, "failed to create change output"sv},
            {InsufficientFunds, "insufficient funds"sv},
            {InputCreationError, "failed to create transaction inputs"sv},
            {SignatureError, "error signing transaction"sv},
            {SendFailed, "failed to broadcast transaction"sv},
            {InvalidSweep, "invalid sweep parameters"sv},
            {Sent, "successfully broadcast transaction"sv},
        });

    if (const auto* i = map.find(code); map.end() != i) {

        return i->second;
    } else {

        return "unknown SendResult"sv;
    }
}

auto print(TxoState in) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum TxoState;
    // WARNING these strings are used as blockchain wallet database keys. Never
    // change their values.
    static constexpr auto map =
        frozen::make_unordered_map<TxoState, std::string_view>({
            {Error, "error"sv},
            {UnconfirmedNew, "unspent (unconfirmed)"sv},
            {UnconfirmedSpend, "spent (unconfirmed)"sv},
            {ConfirmedNew, "unspent"sv},
            {ConfirmedSpend, "spent"sv},
            {OrphanedNew, "orphaned"sv},
            {OrphanedSpend, "orphaned"sv},
            {Immature, "newly generated"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown TxoState"sv;
    }
}

auto print(TxoTag in) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum TxoTag;
    static constexpr auto map =
        frozen::make_unordered_map<TxoTag, std::string_view>({
            {Normal, "normal"sv},
            {Generation, "generated"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown TxoTag"sv;
    }
}
}  // namespace opentxs::blockchain::node
