// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Types.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/node/Types.hpp"   // IWYU pragma: associated

#include <robin_hood.h>

#include "internal/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/SendResult.hpp"     // IWYU pragma: keep
#include "opentxs/blockchain/node/TxoState.hpp"       // IWYU pragma: keep
#include "opentxs/blockchain/node/TxoTag.hpp"         // IWYU pragma: keep

namespace opentxs::blockchain::node
{
auto print(SendResult code) noexcept -> std::string_view
{
    using namespace std::literals;
    using Code = SendResult;
    static const auto map =
        robin_hood::unordered_flat_map<Code, std::string_view>{
            {Code::InvalidSenderNym, "invalid sender nym"sv},
            {Code::AddressNotValidforChain,
             "provided address is not valid for specified blockchain"sv},
            {Code::UnsupportedAddressFormat,
             "address format is not supported"sv},
            {Code::SenderMissingPaymentCode,
             "sender nym does not contain a valid payment code"sv},
            {Code::UnsupportedRecipientPaymentCode,
             "recipient payment code version is not supported"sv},
            {Code::HDDerivationFailure, "key derivation error"sv},
            {Code::DatabaseError, "database error"sv},
            {Code::DuplicateProposal, "duplicate spend proposal"sv},
            {Code::OutputCreationError,
             "failed to create transaction outputs"sv},
            {Code::ChangeError, "failed to create change output"sv},
            {Code::InsufficientFunds, "insufficient funds"sv},
            {Code::InputCreationError, "failed to create transaction inputs"sv},
            {Code::SignatureError, "error signing transaction"sv},
            {Code::SendFailed, "failed to broadcast transaction"sv},
            {Code::Sent, "successfully broadcast transaction"sv},
        };

    try {

        return map.at(code);
    } catch (...) {

        return "unspecified error"sv;
    }
}

auto print(TxoState in) noexcept -> std::string_view
{
    using namespace std::literals;
    using Type = TxoState;
    // WARNING these strings are used as blockchain wallet database keys. Never
    // change their values.
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::Error, "error"sv},
            {Type::UnconfirmedNew, "unspent (unconfirmed)"sv},
            {Type::UnconfirmedSpend, "spent (unconfirmed)"sv},
            {Type::ConfirmedNew, "unspent"sv},
            {Type::ConfirmedSpend, "spent"sv},
            {Type::OrphanedNew, "orphaned"sv},
            {Type::OrphanedSpend, "orphaned"sv},
            {Type::Immature, "newly generated"sv},
        };

    try {

        return map.at(in);
    } catch (...) {

        return {};
    }
}

auto print(TxoTag in) noexcept -> std::string_view
{
    using namespace std::literals;
    using Type = TxoTag;
    static const auto map =
        robin_hood::unordered_flat_map<Type, std::string_view>{
            {Type::Normal, "normal"sv},
            {Type::Generation, "generated"sv},
        };

    try {

        return map.at(in);
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs::blockchain::node
