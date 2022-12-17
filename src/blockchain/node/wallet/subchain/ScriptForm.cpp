// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Pattern

#include "blockchain/node/wallet/subchain/ScriptForm.hpp"  // IWYU pragma: associated

#include <optional>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
// TODO allocator
ScriptForm::ScriptForm(
    const api::Session& api,
    const crypto::Element& input,
    blockchain::Type chain,
    Type primary,
    Type secondary) noexcept
    : segwit_(false)
    , primary_([&] {
        switch (primary) {
            case Type::PayToPubkey:
            case Type::PayToPubkeyHash:
            case Type::PayToScriptHash: {
            } break;
            case Type::PayToWitnessPubkeyHash:
            case Type::PayToWitnessScriptHash: {
                segwit_ = true;
            } break;
            case Type::Custom:
            case Type::Coinbase:
            case Type::NullData:
            case Type::PayToMultisig:
            case Type::PayToTaproot:
            case Type::None:
            case Type::Input:
            case Type::Empty:
            case Type::Malformed:
            default: {
                OT_FAIL;
            }
        }

        return primary;
    }())
    , secondary_([&] {
        switch (primary_) {
            case Type::PayToScriptHash:
            case Type::PayToWitnessScriptHash: {
            } break;
            case Type::Custom:
            case Type::Coinbase:
            case Type::NullData:
            case Type::PayToMultisig:
            case Type::PayToPubkey:
            case Type::PayToPubkeyHash:
            case Type::PayToWitnessPubkeyHash:
            case Type::PayToTaproot:
            case Type::None:
            case Type::Input:
            case Type::Empty:
            case Type::Malformed:
            default: {
                return Type::None;
            }
        }

        switch (secondary) {
            case Type::PayToPubkey:
            case Type::PayToPubkeyHash: {
            } break;
            case Type::Custom:
            case Type::Coinbase:
            case Type::NullData:
            case Type::PayToMultisig:
            case Type::PayToWitnessPubkeyHash:
            case Type::PayToScriptHash:
            case Type::PayToWitnessScriptHash:
            case Type::PayToTaproot:
            case Type::None:
            case Type::Input:
            case Type::Empty:
            case Type::Malformed:
            default: {
                OT_FAIL;
            }
        }

        return secondary;
    }())
    , element_()
    , script_([&]() -> bitcoin::block::Script {
        switch (primary_) {
            case Type::PayToPubkey: {
                auto out =
                    api.Factory().BitcoinScriptP2PK(chain, input.Key(), {});
                element_.emplace_back(out.Pubkey().value());

                return out;
            }
            case Type::PayToPubkeyHash: {
                auto out =
                    api.Factory().BitcoinScriptP2PKH(chain, input.Key(), {});
                element_.emplace_back(out.PubkeyHash().value());

                return out;
            }
            case Type::PayToWitnessPubkeyHash: {
                auto out =
                    api.Factory().BitcoinScriptP2WPKH(chain, input.Key(), {});
                element_.emplace_back(out.PubkeyHash().value());

                return out;
            }
            case Type::Custom:
            case Type::Coinbase:
            case Type::NullData:
            case Type::PayToMultisig:
            case Type::PayToScriptHash:
            case Type::PayToWitnessScriptHash:
            case Type::PayToTaproot:
            case Type::None:
            case Type::Input:
            case Type::Empty:
            case Type::Malformed:
            default: {
            }
        }

        const auto redeem = [&] {
            switch (secondary_) {
                case Type::PayToPubkey: {
                    return api.Factory().BitcoinScriptP2PK(
                        chain, input.Key(), {});
                }
                case Type::PayToPubkeyHash: {
                    return api.Factory().BitcoinScriptP2PKH(
                        chain, input.Key(), {});
                }
                case Type::Custom:
                case Type::Coinbase:
                case Type::NullData:
                case Type::PayToMultisig:
                case Type::PayToScriptHash:
                case Type::PayToWitnessPubkeyHash:
                case Type::PayToWitnessScriptHash:
                case Type::PayToTaproot:
                case Type::None:
                case Type::Input:
                case Type::Empty:
                case Type::Malformed:
                default: {
                    LogAbort()(OT_PRETTY_CLASS())("invalid script type")
                        .Abort();
                }
            }
        }();
        auto out = [&] {
            if (segwit_) {
                return api.Factory().BitcoinScriptP2WSH(chain, redeem, {});
            } else {
                return api.Factory().BitcoinScriptP2SH(chain, redeem, {});
            }
        }();
        element_.emplace_back(out.ScriptHash().value());

        return out;
    }())
{
    OT_ASSERT(script_.IsValid());
    OT_ASSERT(0 < element_.size());
}

ScriptForm::ScriptForm(
    const api::Session& api,
    const crypto::Element& input,
    blockchain::Type chain,
    Type primary) noexcept
    : ScriptForm(api, input, chain, primary, Type::None)
{
}

ScriptForm::ScriptForm(ScriptForm&& rhs) noexcept
    : segwit_(std::move(rhs.segwit_))
    , primary_(std::move(rhs.primary_))
    , secondary_(std::move(rhs.secondary_))
    , element_(std::move(rhs.element_))
    , script_(std::move(rhs.script_))
{
}

auto ScriptForm::operator=(ScriptForm&& rhs) noexcept -> ScriptForm&
{
    if (this != &rhs) {
        std::swap(segwit_, rhs.segwit_);
        std::swap(primary_, rhs.primary_);
        std::swap(secondary_, rhs.secondary_);
        std::swap(element_, rhs.element_);
        std::swap(script_, rhs.script_);
    }

    return *this;
}
}  // namespace opentxs::blockchain::node::wallet
