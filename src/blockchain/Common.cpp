// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/Blockchain.hpp"   // IWYU pragma: associated
#include "opentxs/blockchain/Types.hpp"        // IWYU pragma: associated

#include <stdexcept>
#include <string_view>

#include "opentxs/Types.hpp"
#include "opentxs/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Category.hpp"        // IWYU pragma: keep
#include "opentxs/crypto/HashType.hpp"            // IWYU pragma: keep
#include "opentxs/crypto/Hasher.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/display/Definition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain
{
static auto run_hasher(
    const ReadView input,
    Writer&& output,
    opentxs::crypto::Hasher hasher) noexcept -> bool
{
    if (false == hasher(input)) { return false; }

    return hasher(std::move(output));
}
}  // namespace opentxs::blockchain

namespace opentxs::blockchain
{
using namespace std::literals;

auto BlockHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    using enum Type;

    switch (chain) {
        case Dash:
        case Dash_testnet3: {

            return ProofOfWorkHash(crypto, chain, input, std::move(output));
        }
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Ethereum:
        case Ethereum_ropsten:
        case Ethereum_goerli:
        case Ethereum_sepolia:
        case Ethereum_holesovice:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Casper:
        case Casper_testnet:
        case UnitTest:
        default: {

            return run_hasher(
                input, std::move(output), BlockHasher(crypto, chain));
        }
    }
}

auto BlockHasher(const api::Crypto& crypto, const Type chain) noexcept
    -> opentxs::crypto::Hasher
{
    using enum opentxs::blockchain::Type;
    using opentxs::crypto::HashType;

    switch (chain) {
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Ethereum:
        case Ethereum_ropsten:
        case Ethereum_goerli:
        case Ethereum_sepolia:
        case Ethereum_holesovice:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Casper:
        case Casper_testnet:
        case Dash:
        case Dash_testnet3:
        case UnitTest:
        default: {

            return crypto.Hash().Hasher(HashType::Sha256D);
        }
    }
}

auto FilterHasher(const api::Crypto& crypto, const Type chain) noexcept
    -> opentxs::crypto::Hasher
{
    return BlockHasher(crypto, chain);
}

auto FilterHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    return run_hasher(input, std::move(output), FilterHasher(crypto, chain));
}

auto MerkleHasher(const api::Crypto& crypto, const Type chain) noexcept
    -> opentxs::crypto::Hasher
{
    return BlockHasher(crypto, chain);
}

auto MerkleHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    return run_hasher(input, std::move(output), MerkleHasher(crypto, chain));
}

auto P2PMessageHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    using enum opentxs::blockchain::Type;
    using opentxs::crypto::HashType;

    switch (chain) {
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Ethereum:
        case Ethereum_ropsten:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Casper:
        case Casper_testnet:
        case Dash:
        case Dash_testnet3:
        case UnitTest:
        default: {
            return crypto.Hash().Digest(
                HashType::Sha256DC, input, std::move(output));
        }
    }
}

auto ProofOfWorkHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    using enum opentxs::blockchain::Type;
    using opentxs::crypto::HashType;

    switch (chain) {
        case Litecoin:
        case Litecoin_testnet4: {
            return crypto.Hash().Scrypt(
                input, input, 1024, 1, 1, 32, std::move(output));
        }
        case Dash:
        case Dash_testnet3: {
            return crypto.Hash().Digest(
                HashType::X11, input, std::move(output));
        }
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Ethereum:
        case Ethereum_ropsten:
        case Ethereum_goerli:
        case Ethereum_sepolia:
        case Ethereum_holesovice:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Casper:
        case Casper_testnet:
        case UnitTest:
        default: {
            return BlockHash(crypto, chain, input, std::move(output));
        }
    }
}

auto PubkeyHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    using opentxs::crypto::HashType;

    switch (chain) {
        using enum opentxs::blockchain::Type;
        case Ethereum:
        case Ethereum_ropsten:
        case Ethereum_goerli:
        case Ethereum_sepolia:
        case Ethereum_holesovice:
        case Casper:
        case Casper_testnet: {

            return crypto.Hash().Digest(
                HashType::Ethereum, input, std::move(output));
        }
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Dash:
        case Dash_testnet3:
        case UnitTest:
        default: {

            return crypto.Hash().Digest(
                HashType::Bitcoin, input, std::move(output));
        }
    }
}

auto ScriptHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    return run_hasher(input, std::move(output), ScriptHasher(crypto, chain));
}

auto ScriptHasher(const api::Crypto& crypto, const Type chain) noexcept
    -> opentxs::crypto::Hasher
{
    using enum opentxs::blockchain::Type;
    using opentxs::crypto::HashType;

    switch (chain) {
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Ethereum:
        case Ethereum_ropsten:
        case Ethereum_goerli:
        case Ethereum_sepolia:
        case Ethereum_holesovice:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Casper:
        case Casper_testnet:
        case Dash:
        case Dash_testnet3:
        case UnitTest:
        default: {

            return crypto.Hash().Hasher(HashType::Bitcoin);
        }
    }
}

auto ScriptHashSegwit(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    return run_hasher(
        input, std::move(output), ScriptHasherSegwit(crypto, chain));
}

auto ScriptHasherSegwit(const api::Crypto& crypto, const Type chain) noexcept
    -> opentxs::crypto::Hasher
{
    using enum opentxs::blockchain::Type;
    using opentxs::crypto::HashType;

    switch (chain) {
        case UnknownBlockchain:
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinCash_testnet4:
        case Ethereum:
        case Ethereum_ropsten:
        case Ethereum_goerli:
        case Ethereum_sepolia:
        case Ethereum_holesovice:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case Casper:
        case Casper_testnet:
        case Dash:
        case Dash_testnet3:
        case UnitTest:
        default: {

            return crypto.Hash().Hasher(HashType::Sha256);
        }
    }
}

auto TransactionHash(
    const api::Crypto& crypto,
    const Type chain,
    const ReadView input,
    Writer&& output) noexcept -> bool
{
    return run_hasher(
        input, std::move(output), TransactionHasher(crypto, chain));
}

auto TransactionHasher(const api::Crypto& crypto, const Type chain) noexcept
    -> opentxs::crypto::Hasher
{
    return BlockHasher(crypto, chain);
}
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::internal
{
auto BlockHashToFilterKey(const ReadView hash) noexcept(false) -> ReadView
{
    if (16_uz > hash.size()) { throw std::runtime_error("Hash too short"); }

    return ReadView{hash.data(), 16_uz};
}

auto Format(const Type chain, const opentxs::Amount& amount) noexcept
    -> UnallocatedCString
{
    try {
        const auto& definition =
            display::GetDefinition(blockchain_to_unit(chain));

        return definition.Format(amount);
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs::blockchain::internal
