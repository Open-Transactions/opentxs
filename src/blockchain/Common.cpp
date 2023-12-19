// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/Blockchain.hpp"   // IWYU pragma: associated
#include "opentxs/blockchain/Types.hpp"        // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "internal/blockchain/params/ChainData.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Category.hpp"        // IWYU pragma: keep
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Hasher.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto unit_to_blockchain(const UnitType type) noexcept -> blockchain::Type
{
    static const auto map = [] {
        auto out = boost::unordered_flat_map<UnitType, blockchain::Type>{};

        for (const auto chain : blockchain::defined_chains()) {
            out.emplace(blockchain::params::get(chain).CurrencyType(), chain);
        }

        out.reserve(out.size());

        return out;
    }();

    if (auto i = map.find(type); map.end() != i) {

        return i->second;
    } else {

        return blockchain::Type::UnknownBlockchain;
    }
}
}  // namespace opentxs

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

auto associated_mainnet(Type type) noexcept -> Type
{
    try {
        return blockchain::params::get(type).AssociatedMainnet();
    } catch (...) {
        return Type::UnknownBlockchain;
    }
}

auto blockchain_to_unit(const blockchain::Type type) noexcept -> UnitType
{
    try {
        return blockchain::params::get(type).CurrencyType();
    } catch (...) {
        return UnitType::Unknown;
    }
}

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

auto category(Type type) noexcept -> Category
{
    try {
        return blockchain::params::get(type).Category();
    } catch (...) {
        return Category::unknown_category;
    }
}

auto defined_chains() noexcept -> std::span<const Type>
{
    static const auto data = [] {
        auto out = Vector<Type>{};
        const auto& chains = params::chains();
        out.reserve(chains.size());
        std::ranges::copy(chains, std::back_inserter(out));

        return out;
    }();

    return data;
}

auto display(Type type) noexcept -> const display::Definition&
{
    return display::GetDefinition(blockchain_to_unit(type));
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

auto has_segwit(const Type type) noexcept -> bool
{
    try {

        return params::get(type).SupportsSegwit();
    } catch (...) {

        return false;
    }
}

auto is_defined(Type in) noexcept -> bool
{
    return params::chains().contains(in);
}

auto is_descended_from(Type descendant, Type ancestor) noexcept -> bool
{
    static const auto map = [] {
        using Descendants = boost::container::flat_set<blockchain::Type>;
        using Map = boost::container::flat_map<blockchain::Type, Descendants>;
        auto out = Map{};

        for (const auto& chain : params::chains()) {
            out[chain].emplace(chain);
            auto parent = std::optional<blockchain::Type>{std::nullopt};
            auto current{chain};

            for (;;) {
                if (parent = params::get(current).ForkedFrom(); parent) {
                    current = *parent;
                    out[current].emplace(chain);
                } else {

                    break;
                }
            }
        }

        for (auto& [_, value] : out) { value.shrink_to_fit(); }

        out.shrink_to_fit();

        return out;
    }();

    if (auto i = map.find(ancestor); map.end() != i) {

        return i->second.contains(descendant);
    } else {

        return false;
    }
}

auto is_supported(Type in) noexcept -> bool
{
    return params::supported().contains(in);
}

auto is_testnet(const Type type) noexcept -> bool
{
    try {

        return params::get(type).IsTestnet();
    } catch (...) {

        return false;
    }
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

auto print(Type in) noexcept -> std::string_view
{
    return print(blockchain_to_unit(in));
}

auto print(Category in) noexcept -> std::string_view
{
    using enum Category;
    static constexpr auto map =
        frozen::make_unordered_map<Category, std::string_view>({
            {unknown_category, "unknown category"sv},
            {output_based, "bitcoin derived"sv},
            {balance_based, "ethereum derived"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid Category: ")(
            static_cast<TypeEnum>(in))
            .Abort();
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

auto supported_chains() noexcept -> std::span<const Type>
{
    static const auto output = [] {
        auto out = Vector<Type>{};
        const auto& chains = params::supported();
        out.reserve(chains.size());
        std::ranges::copy(chains, std::back_inserter(out));

        return out;
    }();

    return output;
}

auto ticker_symbol(const Type type) noexcept -> UnallocatedCString
{
    return UnallocatedCString{display(type).ShortName()};
}

auto ticker_symbol(const Type type, alloc::Strategy alloc) noexcept -> CString
{
    return CString{display(type).ShortName(), alloc.result_};
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
