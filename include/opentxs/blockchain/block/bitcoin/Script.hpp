// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Iterator.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class OPENTXS_EXPORT Script
{
public:
    using value_type = ScriptElement;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Script, const value_type>;

    enum class Pattern : std::uint8_t {
        Custom = 0,
        Coinbase,
        NullData,
        PayToMultisig,
        PayToPubkey,
        PayToPubkeyHash,
        PayToScriptHash,
        PayToWitnessPubkeyHash,
        PayToWitnessScriptHash,
        PayToTaproot,
        None = 252,
        Input = 253,
        Empty = 254,
        Malformed = 255,
    };

    enum class Position : std::uint8_t {
        Coinbase = 0,
        Input = 1,
        Output = 2,
        Redeem = 3,
    };

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto CalculateHash160(
        const api::Session& api,
        const AllocateOutput output) const noexcept -> bool = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> std::pmr::vector<Space> = 0;
    virtual auto ExtractPatterns(const api::Session& api) const noexcept
        -> std::pmr::vector<PatternID> = 0;
    virtual auto IsNotification(
        const std::uint8_t version,
        const PaymentCode& recipient) const noexcept -> bool = 0;
    /// Value only present for Multisig patterns
    virtual auto M() const noexcept -> std::optional<std::uint8_t> = 0;
    /// Value only present for Multisig patterns, 0 indexed
    virtual auto MultisigPubkey(const std::size_t position) const noexcept
        -> std::optional<ReadView> = 0;
    /// Value only present for Multisig patterns
    virtual auto N() const noexcept -> std::optional<std::uint8_t> = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    /// Value only present for PayToPubkey and PayToTaproot patterns
    virtual auto Pubkey() const noexcept -> std::optional<ReadView> = 0;
    /// Value only present for PayToPubkeyHash and PayToWitnessPubkeyHash
    /// patterns
    virtual auto PubkeyHash() const noexcept -> std::optional<ReadView> = 0;
    /// Value only present for input scripts which spend PayToScriptHash outputs
    virtual auto RedeemScript() const noexcept -> std::unique_ptr<Script> = 0;
    virtual auto Role() const noexcept -> Position = 0;
    /// Value only present for PayToScriptHash and PayToWitnessScriptHash
    /// patterns
    virtual auto ScriptHash() const noexcept -> std::optional<ReadView> = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> bool = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Type() const noexcept -> Pattern = 0;
    /// Value only present for NullData patterns, 0 indexed
    virtual auto Value(const std::size_t position) const noexcept
        -> std::optional<ReadView> = 0;

    virtual ~Script() = default;

protected:
    Script() noexcept = default;

private:
    Script(const Script&) = delete;
    Script(Script&&) = delete;
    auto operator=(const Script&) -> Script& = delete;
    auto operator=(Script&&) -> Script& = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
