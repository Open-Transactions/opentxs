// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/qt/DestinationValidator.hpp"  // IWYU pragma: associated

#include <QMetaObject>
#include <sstream>
#include <string_view>
#include <utility>

#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Type.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui
{
using Super = DestinationValidator::Imp;

struct BlockchainDestinationValidator final : public Super {
    auto fixup(QString& input) const -> void final
    {
        strip_invalid(input, true);

        if (0 == input.size()) { reset(UnallocatedCString{}); }
    }
    auto getDetails() const -> QString final { return details_.c_str(); }
    auto validate(QString& input, int&) const -> QValidator::State final
    {
        fixup(input);

        if (0 == input.size()) {
            reset(UnallocatedCString{});

            return QValidator::State::Intermediate;
        }

        const auto candidate = input.toStdString();
        const auto code = api_.Factory().PaymentCodeFromBase58(candidate);
        auto text = std::stringstream{};
        text << "Address format: ";

        if (const auto ver = code.Version(); 0u < ver) {
            text << "version " << std::to_string(ver) << " payment code";
            reset(text.str());

            return QValidator::State::Acceptable;
        }

        const auto decoded =
            api_.Crypto().Blockchain().DecodeAddress(candidate);
        const auto& [data, style, chains, supported] = decoded;

        if (0 == data.size()) {
            text << "unknown";
            reset(text.str());

            return QValidator::State::Intermediate;
        }

        assert_true(0 < chains.size());

        const auto type = *chains.begin();
        const auto chain = print(type);
        const auto validChain = (chains.contains(chain_));

        switch (style) {
            using enum blockchain::crypto::AddressStyle;
            case p2pkh: {
                text << "P2PKH";
            } break;
            case p2sh: {
                text << "P2SH";
            } break;
            case p2wpkh: {
                text << "P2WPKH";
            } break;
            case p2wsh: {
                text << "P2WSH";
            } break;
            case p2tr: {
                text << "P2TR";
            } break;
            case ethereum_account: {
                text << "Ethereum";
            } break;
            case unknown_address_style:
            default: {
                text << "unsupported";
                reset(text.str());

                return QValidator::State::Intermediate;
            }
        }

        if (validChain) {
            if (supported) {
                reset(text.str());

                return QValidator::State::Acceptable;
            } else {
                text << " not supported on " << chain;

                return QValidator::State::Invalid;
            }
        } else {
            text = std::stringstream{};
            text << "This address is only valid on ";

            if (is_testnet(type)) {
                text << "testnet";
            } else {
                text << chain;
            }

            reset(text.str());

            return QValidator::State::Intermediate;
        }
    }

    BlockchainDestinationValidator(
        const api::session::Client& api,
        DestinationValidator& main,
        blockchain::Type chain,
        Parent& parent) noexcept
        : parent_(main)
        , api_(api)
        , chain_(chain)
        , details_()
    {
    }
    ~BlockchainDestinationValidator() final = default;

private:
    DestinationValidator& parent_;
    const api::session::Client& api_;
    const blockchain::Type chain_;
    mutable UnallocatedCString details_;

    auto reset(UnallocatedCString&& details) const noexcept -> void
    {
        details_ = std::move(details);

        Q_EMIT parent_.detailsChanged(details_.c_str());
    }
};

auto DestinationValidator::Imp::Blockchain(
    const api::session::Client& api,
    DestinationValidator& main,
    const identifier::Account& account,
    Parent& parent) noexcept -> std::unique_ptr<Imp>
{
    const auto [chain, owner] =
        api.Crypto().Blockchain().LookupAccount(account);

    if (blockchain::Type::UnknownBlockchain == chain) { return nullptr; }

    return std::make_unique<BlockchainDestinationValidator>(
        api, main, chain, parent);
}
}  // namespace opentxs::ui
