// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Checker.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class Checker : virtual public blockchain::block::Checker
{
public:
    [[nodiscard]] auto operator()(const Hash& expected, ReadView bytes) noexcept
        -> bool final;

    Checker() = delete;
    Checker(const api::Crypto& crypto, blockchain::Type type) noexcept;
    Checker(const Checker&) = delete;
    Checker(Checker&&) = delete;
    auto operator=(const Checker&) -> Checker& = delete;
    auto operator=(Checker&&) -> Checker& = delete;

    ~Checker() override = default;

protected:
    ReadView data_;

    virtual auto find_payload() noexcept -> bool;

private:
    const api::Crypto& crypto_;
    const blockchain::Type chain_;
    Hash block_hash_;
    Hash merkle_root_;
    Hash witness_reserved_value_;
    Hash segwit_commitment_;
    std::size_t transaction_count_;
    Vector<Hash> txids_;
    Vector<Hash> wtxids_;
    bool has_segwit_commitment_;
    bool has_segwit_transactions_;
    bool has_segwit_reserved_value_;

    auto calculate_committment() const noexcept -> Hash;
    auto calculate_merkle() const noexcept -> Hash;
    auto calculate_witness() const noexcept -> Hash;
    auto compare_header_to_hash(const Hash& expected) const noexcept -> bool;
    auto compare_merkle_to_header() const noexcept -> bool;
    auto compare_segwit_to_commitment() const noexcept -> bool;
    auto is_segwit_tx() const noexcept -> bool;

    auto calculate_hash(const ReadView header) noexcept -> bool;
    auto parse_header() noexcept -> bool;
    auto parse_legacy_transaction(const bool isGeneration) noexcept(false)
        -> bool;
    auto parse_next_transaction(const bool isGeneration) noexcept -> bool;
    auto parse_segwit_commitment(const ReadView script) noexcept -> bool;
    auto parse_segwit_transaction(const bool isGeneration) noexcept(false)
        -> bool;
    auto parse_transactions() noexcept -> bool;
};
}  // namespace opentxs::blockchain::bitcoin::block
