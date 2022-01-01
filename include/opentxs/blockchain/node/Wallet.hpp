// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>
#include <tuple>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Output;
}  // namespace bitcoin

struct Outpoint;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Identifier;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace node
{
class OPENTXS_EXPORT Wallet
{
public:
    using UTXO =
        std::pair<block::Outpoint, std::unique_ptr<block::bitcoin::Output>>;

    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetBalance(
        const identifier::Nym& owner,
        const Identifier& subaccount) const noexcept -> Balance = 0;
    virtual auto GetBalance(const crypto::Key& key) const noexcept
        -> Balance = 0;
    virtual auto GetOutputs() const noexcept -> std::pmr::vector<UTXO> = 0;
    virtual auto GetOutputs(TxoState type) const noexcept
        -> std::pmr::vector<UTXO> = 0;
    virtual auto GetOutputs(const identifier::Nym& owner) const noexcept
        -> std::pmr::vector<UTXO> = 0;
    virtual auto GetOutputs(const identifier::Nym& owner, TxoState type)
        const noexcept -> std::pmr::vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& subaccount) const noexcept
        -> std::pmr::vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& subaccount,
        TxoState type) const noexcept -> std::pmr::vector<UTXO> = 0;
    virtual auto GetOutputs(const crypto::Key& key, TxoState type)
        const noexcept -> std::pmr::vector<UTXO> = 0;
    virtual auto GetTags(const block::Outpoint& output) const noexcept
        -> std::pmr::set<TxoTag> = 0;
    virtual auto Height() const noexcept -> block::Height = 0;

    virtual ~Wallet() = default;

protected:
    Wallet() noexcept = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
