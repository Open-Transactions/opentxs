// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: associated
#include "internal/blockchain/Params.hpp"      // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <robin_hood.h>
#include <iosfwd>
#include <memory>
#include <sstream>
#include <type_traits>

#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/SendResult.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto BlockchainToUnit(const blockchain::Type type) noexcept -> UnitType
{
    try {
        return blockchain::params::Data::Chains().at(type).itemtype_;
    } catch (...) {
        return UnitType::Unknown;
    }
}

auto UnitToBlockchain(const UnitType type) noexcept -> blockchain::Type
{
    using Map = UnallocatedMap<opentxs::UnitType, opentxs::blockchain::Type>;

    static const auto build = []() -> auto
    {
        auto output = Map{};

        for (const auto& [chain, data] : blockchain::params::Data::Chains()) {
            output.emplace(data.itemtype_, chain);
        }

        return output;
    };
    static const auto map{build()};

    try {
        return map.at(type);
    } catch (...) {
        return blockchain::Type::Unknown;
    }
}

using Code = blockchain::SendResult;

auto print(Code code) noexcept -> UnallocatedCString
{
    static const auto map =
        robin_hood::unordered_flat_map<Code, UnallocatedCString>{
            {Code::InvalidSenderNym, "invalid sender nym"},
            {Code::AddressNotValidforChain,
             "provided address is not valid for specified blockchain"},
            {Code::UnsupportedAddressFormat, "address format is not supported"},
            {Code::SenderMissingPaymentCode,
             "sender nym does not contain a valid payment code"},
            {Code::UnsupportedRecipientPaymentCode,
             "recipient payment code version is not supported"},
            {Code::HDDerivationFailure, "key derivation error"},
            {Code::DatabaseError, "database error"},
            {Code::DuplicateProposal, "duplicate spend proposal"},
            {Code::OutputCreationError, "failed to create transaction outputs"},
            {Code::ChangeError, "failed to create change output"},
            {Code::InsufficientFunds, "insufficient funds"},
            {Code::InputCreationError, "failed to create transaction inputs"},
            {Code::SignatureError, "error signing transaction"},
            {Code::SendFailed, "failed to broadcast transaction"},
            {Code::Sent, "successfully broadcast transaction"},
        };

    try {

        return map.at(code);
    } catch (...) {

        return "unspecified error";
    }
}

auto print(blockchain::Type type) noexcept -> UnallocatedCString
{
    return blockchain::DisplayString(type);
}

auto print(const blockchain::block::Position& in) noexcept -> UnallocatedCString
{
    const auto& [height, hash] = in;
    auto out = std::stringstream{};
    out << hash->asHex();
    out << " at height ";
    out << std::to_string(height);

    return out.str();
}
}  // namespace opentxs

namespace opentxs::blockchain
{
auto BlockHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                opentxs::crypto::HashType::Sha256D, input, output);
        }
    }
}

auto DefinedChains() noexcept -> const UnallocatedSet<Type>&
{
    static const auto output = [] {
        auto output = UnallocatedSet<Type>{};

        for (const auto& [chain, data] : params::Data::Chains()) {
            output.emplace(chain);
        }

        return output;
    }();

    return output;
}

auto DisplayString(const Type type) noexcept -> UnallocatedCString
{
    return opentxs::print(BlockchainToUnit(type));
}

auto FilterHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto HasSegwit(const Type type) noexcept -> bool
{
    try {

        return params::Data::Chains().at(type).segwit_;
    } catch (...) {

        return false;
    }
}

auto IsTestnet(const Type type) noexcept -> bool
{
    try {

        return params::Data::Chains().at(type).testnet_;
    } catch (...) {

        return false;
    }
}

auto MerkleHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto P2PMessageHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                opentxs::crypto::HashType::Sha256DC, input, output);
        }
    }
}

auto ProofOfWorkHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Litecoin:
        case Type::Litecoin_testnet4: {
            return api.Crypto().Hash().Scrypt(
                input, input, 1024, 1, 1, 32, output);
        }
        case Type::UnitTest:
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::PKT:
        case Type::PKT_testnet:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto PubkeyHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                opentxs::crypto::HashType::Bitcoin, input, output);
        }
    }
}

auto ScriptHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                opentxs::crypto::HashType::Bitcoin, input, output);
        }
    }
}

auto ScriptHashSegwit(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                opentxs::crypto::HashType::Sha256, input, output);
        }
    }
}

auto SupportedChains() noexcept -> const UnallocatedSet<Type>&
{
    static const auto output = [] {
        auto output = UnallocatedSet<Type>{};

        for (const auto& [chain, data] : params::Data::Chains()) {
            if (data.supported_) { output.emplace(chain); }
        }

        return output;
    }();

    return output;
}

auto TickerSymbol(const Type type) noexcept -> UnallocatedCString
{
    return UnallocatedCString{
        display::GetDefinition(BlockchainToUnit(type)).ShortName()};
}

auto TransactionHash(
    const api::Session& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::block
{
auto BlankHash() noexcept -> pHash
{
    return Data::Factory(
        "0x0000000000000000000000000000000000000000000000000000000000000000",
        Data::Mode::Hex);
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::internal
{
auto Format(const Type chain, const opentxs::Amount& amount) noexcept
    -> UnallocatedCString
{
    try {
        const auto& definition =
            display::GetDefinition(BlockchainToUnit(chain));

        return definition.Format(amount);
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs::blockchain::internal

namespace opentxs::blockchain::params
{
#if OT_BLOCKCHAIN
auto Data::Bip158() noexcept -> const FilterTypes&
{
    static const auto data = FilterTypes{
        {Type::Bitcoin,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::Bitcoin_testnet3,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::BitcoinCash,
         {
             {filter::Type::Basic_BCHVariant, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::BitcoinCash_testnet3,
         {
             {filter::Type::Basic_BCHVariant, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::Ethereum_frontier, {}},
        {Type::Ethereum_ropsten, {}},
        {Type::Litecoin,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::Litecoin_testnet4,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::PKT,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::PKT_testnet,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
        {Type::UnitTest,
         {
             {filter::Type::Basic_BIP158, 0x0},
             {filter::Type::ES, 0x58},
         }},
    };

    return data;
}
#endif  // OT_BLOCKCHAIN

auto Data::Chains() noexcept -> const ChainData&
{
    static const auto data = ChainData{
        {blockchain::Type::Bitcoin,
         {
             true,
             false,
             true,
             4u,
             opentxs::UnitType::Btc,
             Bip44Type::BITCOIN,
             486604799,  // 0x1d00ffff
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4a29ab5f49ffff001d1dac2b7c",
             "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4a29ab5f49ffff001d1dac2b7c01010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4d04ffff001d0104"
             "455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f7"
             "2206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f7220"
             "62616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a"
             "67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e5"
             "1ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
             {688000,
              "d305263e3bc034a3395437e316437c5a5347c18e6f520100000000000000000"
              "0",
              "dd08dde8736a06996ab3fc3d884d84a5d369246183c40900000000000000000"
              "0",
              "2b9004f975a42b8b911731ff2f703ed12f0bafdcbcbf71fc12d2cfef9b0ce0b"
              "6"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             3652501241,
             8333,
             {
                 "seed.bitcoin.sipa.be",
                 "dnsseed.bluematt.me",
                 "dnsseed.bitcoin.dashjr.org",
                 "seed.bitcoinstats.com",
                 "seed.bitcoin.jonasschnelli.ch",
                 "seed.btc.petertodd.org",
                 "seed.bitcoin.sprovoost.nl",
                 "dnsseed.emzy.de",
             },
             25000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::Bitcoin_testnet3,
         {
             true,
             true,
             true,
             4u,
             opentxs::UnitType::Tnbtc,
             Bip44Type::TESTNET,
             486604799,  // 0x1d00ffff
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4adae5494dffff001d1aa4ae18",
             "43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4adae5494dffff001d1aa4ae1801010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4d04ffff001d0104"
             "455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f7"
             "2206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f7220"
             "62616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a"
             "67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e5"
             "1ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
             {2005000,
              "15ff88d0b983325bb99a6375538b7c0e60427373754c591dccfb5c5c0000000"
              "0",
              "b5f3c6cab9d4fadc9c679ad19e46da9be74d33ec640d5de0070000000000000"
              "0",
              "072fffbbd1723b1796b92743fb8af34f8e62173985dd3249b663c2e62da6e8a"
              "9"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             118034699,
             18333,
             {
                 "testnet-seed.bitcoin.jonasschnelli.ch",
                 "seed.tbtc.petertodd.org",
                 "seed.testnet.bitcoin.sprovoost.nl",
                 "testnet-seed.bluematt.me",
                 "testnet-seed.bitcoin.schildbach.de",
             },
             4000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::BitcoinCash,
         {
             true,
             false,
             false,
             0,
             opentxs::UnitType::Bch,
             Bip44Type::BITCOINCASH,
             486604799,  // 0x1d00ffff
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4a29ab5f49ffff001d1dac2b7c",
             "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4a29ab5f49ffff001d1dac2b7c01010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4d04ffff001d0104"
             "455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f7"
             "2206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f7220"
             "62616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a"
             "67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e5"
             "1ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
             {693000,
              "ce0acf276d39e75741808afdde9c95b75c2fef935bc25401000000000000000"
              "0",
              "c83139029d4efddeda60ede13474276cdf1267fff6126900000000000000000"
              "0",
              "fb2e542d6e1aa15d763a3a1add8262fa7cb609063d8047372deae34118f8836"
              "a"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             3908297187,
             8333,
             {
                 "seed.bitcoinabc.org",
                 "seed-abc.bitcoinforks.org",
                 "btccash-seeder.bitcoinunlimited.info",
                 "seed.deadalnix.me",
                 "seed.bchd.cash",
                 "dnsseed.electroncash.de",
             },
             1000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, false},
                 {Style::P2WSH, false},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::BitcoinCash_testnet3,
         {
             true,
             true,
             false,
             0,
             opentxs::UnitType::Tnbch,
             Bip44Type::TESTNET,
             486604799,  // 0x1d00ffff
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4adae5494dffff001d1aa4ae18",
             "43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4adae5494dffff001d1aa4ae1801010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4d04ffff001d0104"
             "455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f7"
             "2206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f7220"
             "62616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a"
             "67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e5"
             "1ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
             {1452000,
              "5b9a0ecfcb3c18484bb5ce4d39e6bb29c204f482bcda89947a0000000000000"
              "0",
              "b7c4df50f1c631aaf16baf75167c27a8ed755766ac67acef750000000000000"
              "0",
              "fb2e542d6e1aa15d763a3a1add8262fa7cb609063d8047372deae34118f8836"
              "a"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             4109624820,
             18333,
             {
                 "testnet-seed.bitcoinabc.org",
                 "testnet-seed-abc.bitcoinforks.org",
                 "testnet-seed.bitprim.org",
                 "testnet-seed.deadalnix.me",
                 "testnet-seed.bchd.cash",
             },
             1000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, false},
                 {Style::P2WSH, false},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::Ethereum_frontier,
         {
             false,
             false,
             false,
             0,
             opentxs::UnitType::Eth,
             Bip44Type::ETHER,
             0,
             "",
             "d4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3",
             "",
             {0, "", "", ""},
             {},
             p2p::Protocol::ethereum,
             0,
             30303,
             {},
             0,
             0,
             {
                 {Style::P2PKH, false},
                 {Style::P2SH, false},
                 {Style::P2WPKH, false},
                 {Style::P2WSH, false},
                 {Style::P2TR, false},
             },
             {},
             {},
         }},
        {blockchain::Type::Ethereum_ropsten,
         {
             false,
             true,
             false,
             0,
             opentxs::UnitType::Ethereum_ropsten,
             Bip44Type::TESTNET,
             0,
             "",
             "41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d",
             "",
             {0, "", "", ""},
             {},
             p2p::Protocol::ethereum,
             0,
             30303,
             {},
             0,
             0,
             {
                 {Style::P2PKH, false},
                 {Style::P2SH, false},
                 {Style::P2WPKH, false},
                 {Style::P2WSH, false},
                 {Style::P2TR, false},
             },
             {},
             {},
         }},
        {blockchain::Type::Litecoin,
         {
             true,
             false,
             true,
             4u,
             opentxs::UnitType::Ltc,
             Bip44Type::LITECOIN,
             504365040,  // 0x1e0ffff0
             "01000000000000000000000000000000000000000000000000000000000000000"
             "0000000d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6ba"
             "fbdd97b9aa8e4ef0ff0f1ecd513f7c",
             "e2bf047e7e5a191aa4ef34d314979dc9986e0f19251edaba5940fd1fe365a712",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "0000000d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6ba"
             "fbdd97b9aa8e4ef0ff0f1ecd513f7c01010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4804ffff001d0104"
             "404e592054696d65732030352f4f63742f32303131205374657665204a6f62732"
             "c204170706c65e280997320566973696f6e6172792c2044696573206174203536"
             "ffffffff0100f2052a010000004341040184710fa689ad5023690c80f3a49c8f1"
             "3f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b"
             "51850b4acf21b179c45070ac7b03a9ac00000000",
             {2074000,
              "35602501052d8e861ee18040558ca6c7abf910512cc4e8e52eaaea137700b56"
              "2",
              "9d9491a5cc4d4f1aead008449a372cab7b96b5e0116b93336df0143e882d2a5"
              "6",
              "bfff6ef3acccc9d4d4e593864514eeaa8c78aa1b0e29efb3aa92228a4f97fea"
              "1"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             3686187259,
             9333,
             {
                 "seed-a.litecoin.loshan.co.uk",
                 "dnsseed.thrasher.io",
                 "dnsseed.litecointools.com",
                 "dnsseed.litecoinpool.org",
                 "dnsseed.koin-project.com",
             },
             2000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::Litecoin_testnet4,
         {
             true,
             true,
             true,
             4u,
             opentxs::UnitType::Tnltx,
             Bip44Type::TESTNET,
             504365040,  // 0x1e0ffff0
             "01000000000000000000000000000000000000000000000000000000000000000"
             "0000000d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6ba"
             "fbdd97f60ba158f0ff0f1ee1790400",
             "a0293e4eeb3da6e6f56f81ed595f57880d1a21569e13eefdd951284b5a626649",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "0000000d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6ba"
             "fbdd97f60ba158f0ff0f1ee179040001010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4804ffff001d0104"
             "404e592054696d65732030352f4f63742f32303131205374657665204a6f62732"
             "c204170706c65e280997320566973696f6e6172792c2044696573206174203536"
             "ffffffff0100f2052a010000004341040184710fa689ad5023690c80f3a49c8f1"
             "3f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b"
             "51850b4acf21b179c45070ac7b03a9ac00000000",
             {1941000,
              "461cb388a237e4da64bae7de29fdd34299e637aeeea2e61a874bbe562b85919"
              "5",
              "a88a1bff3883708a8ef2b1f9429c548866e3666ae0bcdc78f360af3b6fddc61"
              "b",
              "85ca75a1084b9ab7bd7dde196260e175d841064fccc7a247bf2766eda2b0194"
              "b"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             4056470269,
             19335,
             {
                 "testnet-seed.litecointools.com",
                 "seed-b.litecoin.loshan.co.uk",
                 "dnsseed-testnet.thrasher.io",
             },
             2000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::PKT,
         {
             true,
             false,
             true,
             4u,
             opentxs::UnitType::Pkt,
             Bip44Type::PKT,
             521142271,  // 0x1f0fffff
             "00000000000000000000000000000000000000000000000000000000000000000"
             "0000000df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36a"
             "c891fb00000000ffff0f1f00000000",
             "852d43936f4c9606a4a063cf356c454f2d9c43b07a41cf52e59461a41217dc0b",
             "00000000000000000000000000000000000000000000000000000000000000000"
             "0000000df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36a"
             "c891fb00000000ffff0f1f0000000001fd04160000000000000000df345ba23b1"
             "3467eec222a919d449dab6506abc555ef307794ecd3d36ac891fb00096e88ffff"
             "0f1f0300000000000000347607000098038000000000ffff0f200000000000000"
             "00000000000000000000000000000000000000000000000000000000000000000"
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000e79d06f72d778459a76a989dbdded6d45b5e4358943c9aab1eb4e42a"
             "9c67f9ac317b762fe60198c3861255552928a179a5e9a6b9b7b7f4b44e02fc351"
             "9f92964fbbfb576d1e9ff3c588c60fb2643602ae1f5695f89460608d3250e57a7"
             "755385aaa0de52409159387de4145d92533cd5f2a0d6d2a21b653311a40bd2556"
             "493171cf1beedf894a090626577d8042e72f9cdab8ab212b2d6ee5ca7b22169a0"
             "1bf903ab05b248fb8ed5de5a2bb0cd3901fc2e3270ffa524ed3adfc9d7fe109d0"
             "e2755f016386a09eda81bd9707bf681d75cef829f3f8ee0903bfdb2c86ff44628"
             "df573143ec832f41ae17e575e31848d9cf430930d81f41b0d803251b81f8181e5"
             "43cb25c7dca4f2454f8f8bb86987db019ceffe7f0a2be807767f85dc903d3b843"
             "af448d14d5214b6ad5812b4d82b8cbea25c69c6b87d667f9c18c2993d500ed902"
             "d4c539a7d06ab0ca95afd946fd3702554e4bf9f76a1f087dccf33356b7efa9149"
             "fa6b4949159d03cb6e7d13efe91134a9ed8adc7c7325d39201cb2c2c1e2191c5e"
             "9d3d71dc5d1232e4cfc603fc5aa994d7bb8d190ca3d7c0e2fb9abb68df80c2cdf"
             "d8d119aec1a9c62c0ef7af9375e56c0330263332c4c879bcda52de73fea26781e"
             "b3dfa19dd2399b605050198fca80467bdca4a50980a3a37aa552f65caf9634b18"
             "fca475551d0a37dceab5f78c1cfdb48917122137cb74e236800c0684936b9cc0c"
             "a563025cb68609be37869fa8e95bb6fdcd15320b3d5b2fabe9524f464dbfabe36"
             "ef958170b5d7f25c40938bd287a5540b00e06ccb40f558958b72541e8ca4f4f96"
             "5e4f78898085b34fdb6e33b1f588b6d0abc4cb119a8f54e0d949a08afb87979d4"
             "c69165ac6bd9e694369a3903ec24c1e3a52c401c88e035a9f6aed6909f3a2b6db"
             "e60e6fa842400c4164c21dc4c8b2325b70ad1829bed742717776ff28457b384f4"
             "bdd0bf48b2db2d18f89af671c58ecded320cf289b8fa9cfd53fcd7352de1cff3c"
             "41d2f7f8ec6f280d8a9d6933da42b66a6a3d30d46742e9cd793388a07e5e15b9b"
             "220b4209415537214447d386abce2c74a24b7dc60ff9ce04a7cad19ab679d0234"
             "ac95e535abd57d3ac91747b2f2cfe1f01bb944502b827fc8d2c5e8f920fb14518"
             "80271991e5f5db796ea8d392138cd18f602dc6deb3149c44e5085fbd77dc99757"
             "1e4652009b555253eefd215fb009b14e0e880f67d45e85a8252e457ddd0ace7cf"
             "dd5eec6cee070125b50307b7ab0f3983f32f58b75fb02133f3e0778c089484d07"
             "058e76025855909ff64b7c2ace114b6c302a087acc140be90679fe1d0a7530057"
             "3dc000000000ffff0f20000000000000000000000000000000000000000000000"
             "00000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000007594c5be146f727d7fb8151"
             "93044fb2596ceca3a9b62252e5259ed56b7fb63cd2fe906fac0f3ff2565899819"
             "8d9431a48a0be55a0a84333fbdabab0c318930b97d3bb1fa8a8ddeb1587f97c53"
             "1f81963c70784089465e2ef4f465b8d6bb9bbb27f36971c87b98ccae3f8d44518"
             "1b03c97a84ac8a12241b47d9845f966cedade1c31faa857bf2cafae9c71041dd2"
             "3124d4cd4d6dff24cf632e94dd68831639b0f3aba27219cd8a869936605760ae4"
             "08cadeef02c410fc2eeb412bdd7e411614e7830f54ebe0ea6eadae5fe226a67c0"
             "b310d4d4b5d10b47dfe2f165191e69c96e617ef8c3cf763fa49662deb82a2270b"
             "49816f11d56a3493c5e74b0eafbd9492e5fbaa0e0d6600c179a75c2c134e1d6a1"
             "c3721616b6241273b904aec0ef516c402649d032d5e4de8a1fb15bbeb250f5b69"
             "93b6bf5a39314e626d177578fedcc3f7935307321f8f25ae008855b1f19ddf26b"
             "cfa1636b3db132a737b4e1ec50ac9b223670f04a746be5c06e1de90115385c706"
             "af7eb947b9b712f9c14998d31b977ace19a1f2051799fe7aa47bc02f358f2d839"
             "891854825a7e7491e343eb5aa2d468e787989abf9961e21956c5ced5c6a5375e8"
             "09ad958235fc91989fa4141230c42ccbf6a50c6ca61e3780d65dbfc112a104cc1"
             "da5b1dd7ea024d2e37db0bb10ab6f06242589cb5383927ac5d130b189d32e4731"
             "ec1e8b675caf6c4da531db3c598c5da69aa8ddcecae67cefd633fd80f994cec4a"
             "d28c2f1421b316999c1043c749b14a645f785dd56e8fdbc959ff03648336b0c9c"
             "9ca3c86bb96738750b855dffa0b74c9c492580dcbbb892b91d76359aedc0a3d89"
             "a447b23f5449433bb7c4554eb6f0eb8ee63b9df12287f92eb23b3956d3933eecc"
             "f88ca9d9fe19a9a29a2821909f3a2b6dbe60e6fa842400c4164c21dc4c8b2325b"
             "70ad1829bed742717776ff28457b384f4bdd0bf48b2db2d18f89af671c58ecded"
             "320cf289b8fa9cfd53fcd7352de1cff3c41d2f7f8ec6f280d8a9d6933da42b66a"
             "6a3d30d46742e9cd793388a07e5e15b9b220b4209415537214447d386abce2c74"
             "a24b7dc60ff9ce04a7cad19ab679d0234ac95e535abd57d3ac91747b2f2cfe1f0"
             "1bb944502b827fc8d2c5e8f920fb1451880271991e5f5db796ea8d392138cd18f"
             "602dc6deb3149c44e5085fbd77dc997571e4652009b555253eefd215fb009b14e"
             "0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b50307b7ab0f3983"
             "f32f58b75fb021ace16c1a11a478a77f48ec8beda4f4912aa3337010343c14412"
             "cbc2f6d8ceb38dc88989cfee876ab00042a8000000000ffff0f20000000000000"
             "00000000000000000000000000000000000000000000000000000000000000000"
             "00000000000000000000000000000000000000000000000000000000000000000"
             "00000000009653aa497eb0bf1f7b9170967201419b6ced537def4363a0b2869d9"
             "74a91d4458b4099f8d9a5f8555219c9b6efd193e1c745636d42cd705557c48e47"
             "598648c42e1c94318744855d037b3de60b626de12f06be4ec366527100b35ea8d"
             "4626eac5c2461d733c072811aa87bb5a39edf46d13a318f948367fe7a130359cd"
             "2a1ed04a60ee497723623b258cecd2581a4d7cc3d7e9d05ae4d63ffcecdd16a19"
             "decb7dcffc9a9faccb2084177e736170f191b99446049304f95a2dad137670c09"
             "44a41dd36cd356ad70f65eaba46732e7976b4d252980db9e82ff554a599aae46d"
             "d27886e61a22adf51dbf26be34bbc766510ddebb15a9bef63ba3052fe7f712528"
             "07582e08fa1301fd78138917fec593f50758f103966bcf45c32071a279367c90d"
             "2728d9d13a90c3ee64682b86b80738f4ad1cc94e8d2c98d70bc99e72b45a68f47"
             "19465bd291177ef8675eb9ab2cca7599bb8470180137e6d0e92dcd13fd60dfa85"
             "69175055e76d0df50c79447df8a0d6c64d1d240aae79168de62becc24097a5da7"
             "7de3d860efbf3fbb7a737275944899df27a45b9a7203d813dad5c6ebd0986535a"
             "260589a51843ae43bf17902282439ce50ae75ab4ad8f994530750fc1b30d7dc36"
             "4828b76275e3536950834c0afeb17ad04a0a3090cd4e1165b65727b08c939e355"
             "a5c992d87bd80c3a41465bf1b41d304646fbbfb6b350208282945b68d3a0440bb"
             "8d2dabf1b3767ccc02174499f4084be56f7733052ac65bec5401b9e627bb4094c"
             "8c5fad47a0afb5ab1a7db4de6e318f535013c8db58d16e5455fb0d2aa32a4d8e4"
             "d403412db7ecc718e459e81f09fde3523436ef6104f96201f1fa8c4251033198d"
             "39d0c5a87eae9b9499eb2b3551d4e579103de55354c95b4c3b0cee177cb443e85"
             "e4936100efb659bb7356a52f5d51682673e9cf655c9cec51d100979ffbf74922d"
             "feaecf1bf1ac55933c73d5f3fe927674fd5afc5d5a85e5b8d9779d7352de1cff3"
             "c41d2f7f8ec6f280d8a9d6933da42b66a6a3d30d46742e9cd793388a07e5e15b9"
             "b220b4209415537214447d386abce2c74a24b7dc60ff9ce04a7cad19ab679d023"
             "4ac95e535abd57d3ac91747b2f2cfe1f01bb944502b827fc8d2c5e8f920fb1451"
             "880271991e5f5db796ea8d392138cd18f602dc6deb3149c44e5085fbd77dc9975"
             "71e4652009b555253eefd215fb009b14e0e880f67d45e85a8252e457ddd0ace7c"
             "fdd5eec6cee070125b50307b7ab0f3983f32f58b75fb0213ab54f4815c5fb0803"
             "d5ddd6d4278fc7105e5a15aff36d31ba05dd094c5d2b1f59974dd4d04c369300c"
             "b318000000000ffff0f2000000000000000000000000000000000000000000000"
             "00000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000d120d39a00a6aeb9703eaa"
             "6410db4990a504e21cdc0ccc4f913441b647104b4f0b8b87661db287ccaa443f2"
             "920759e0b9524babb4e227c7cc6a0ee765ff26b15ac81d3e764d6e4f8527edf23"
             "6288ca56196d55a51a8c2a7cb9f9fd7f235a459fb9f77454c0a0cfbd71605850d"
             "cb3ad5428614ef576b3cc358a2286bd7089a0459aea9c86741eb0e4e295ec976b"
             "94efcb4441e998c8e51758de78301ed490f799867355ecd7c57c1d6adfcb2f789"
             "f53f47ddd22fb6dad62b4d1b7315001c5b341a265587a38265e0e3ea811e53fbe"
             "e01786efedc6bab28d0ece33016c96a7a52cc1c77cb8eb932020b883222dbb8a3"
             "c9209b7a8e9ef54828b205a63ce185fa813409d4589c203b782fae087f59141ac"
             "a33b8a89af33314de4b215fb61821c03d76f0ac07d2d97e5cad8fe5864de4269d"
             "db23e0cbf4b53170a4b43da80e7d128f07a471f4ed7e81a9d4ab038cd4cb570c8"
             "10bd4386b882b29d965824d651fdade58fa18a231a2ad288ed5fb0a1716c45c24"
             "b80a332d5d8cd56d6f663b5b5bec1854bb2477b43bfa482d32577ebe6f775f134"
             "9c71fb98c49eccd2a6a984b29da8664e0715ce25b520e58622a207fd6f58b95a3"
             "7b095308e25672bca89d742faebbf8e397d5847a50266d4c8f76bdb9306d105a8"
             "a7d83d20ab07a8769fc1c64ae92233115a91352458a11f329b2b227b07e7aac54"
             "39354fd30e4c1ef22ed6061bdd65020347eb495e40f7ed2d5e5dd6e6cbd34dcdb"
             "1078f771c3c93c8e2f989fd4af8e4704acdae9f0a71e154bf6d0ada9efd1fc6a1"
             "76299a3ef71fa650484d1d7062835a92def53df596633bf39bf0383f30674ea81"
             "003187222c48d8d91989bfd41d40edde7b07c29f8da3e0446cc6f5c58f2941af4"
             "418658bc55c20dec60859c8e8f8545263179afdf5c1b48aedc0fb4b71bf00cd0e"
             "53e86d3af5350ba6ed0b283e2fbbe3333a2856b81f4db572f5193ef5c7561dd6c"
             "22e3c0b411fd711529e69bf05811b2e8ed4fcec0080b506394154245190535ebd"
             "f909fbaae9ced09b8f63f925e9170701598f9757e4db71546f4a4bbe4ad32be2f"
             "551f3841e3125881a4750ad6684076e0cf8a9565c3dfe5140b7b40f3578867a19"
             "cf652bef184f9ed2ad63bfa62e16bd8bb52232d76b171559acaa7c51d56103a83"
             "735f0d5b1ae3bc720e5085fbd77dc997571e4652009b555253eefd215fb009b14"
             "e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b50307b7ab0f398"
             "3f32f58b75fb0218228bfd8f3d022cd5a99786769f3a3e038e68fc7021fd54e87"
             "45ea09380d112f5846acb6b0b693a1ad015ae6d04e43116192dc9edcdcdf52b2e"
             "ce486afccac3a84da182bc48b69b3dec842c1d5f76abe2f9155a322a03808f708"
             "af8b589bdd206c338a2fefa693bc9dc232bdb3c03d1fa32b1da8a4514de4fccb2"
             "df8c0ffa2036dc15a92cd13bcb938f3d76853db406ece5f3bbfc6adb556855af8"
             "05acdf2b1784fba6e61c1288024f8609b9cee016f3b09c07b1e3257c03fc6f6a2"
             "bf40fd597d326d3eb2bb10c6a4412cb8e260153008a482f7315f2235a3ae044df"
             "7004944fddec3a3eba0095fcb7432c07752f662e57559217925a030083452f832"
             "2f71a201497ceb1aa8efea84504687932b1630f8440cd8b5b835424a99a6ba6ef"
             "531f0039c96dc9df6ddb1da17db6192d68265aa69fe8e7591d29f883799f4e853"
             "0085220cfe3d522c74c00ec447082de3f07f03e4cf6f427b0f2e54fa73d0ee631"
             "d7e632101d487173ab63a5a014250a34f900730eb4554c4fcaff9e11e9051a3d7"
             "142d74708aadc2e29e3dec6fa67563527027c92a77e85f39702b90f869548e8d2"
             "03f4b9166fd7ea1032e793228ea8ed223fa6d69ffef6c9ceca87df21a33bf16d0"
             "095ccd7de5c20364a71f63933bc5e9f3269497e6bdc1969d6f4e2106a5ed1adcd"
             "971f9af95e595d00953c1527674ba6b82b0f8f6ce97ded33774c8defd97c5ff1e"
             "fc54617984d68bde405e946062e16004f841e6d1cb21d25f844c947d9db391b63"
             "94537f0ee65b2670abcb51acb86515aa98155916420e00dadfa924a79604be007"
             "4b78bdba7439f6ac8a0b028c43947f32cf1bde6af3dc9ffc3b36837c2e2008396"
             "8aa01025b298c3f70f00028c0ed271ba1f8a425d46a81e480ad932dce9f46a84d"
             "6ccfe205403ad32dc1b571683788d29b2db5a793410d9a5843fb29d60ab294e0c"
             "cc2f35bfe1593e112a44dd3408760054899838af83022b08c6b224b92da9961cf"
             "8e5c518c082f07b037c87f56d1c711e4564c8c3061b57767b6ffd2cb2f782d8a0"
             "2db34ba0d94f6a0f8664af79fff0eac78b47b753df86cdb06ebe88017a391df96"
             "56bf69eac1536d4237d19b601b632f65c35b264d0b634d17e2d8882af7cf5859b"
             "752801210e474f50eb15a8e67cb2be55332de8c389d1beeddfc275a3efeeb25ef"
             "6eadc57f4ab65436f7600d93cd72a0ee92af81941141ba58b6e361510f10bf66f"
             "f61ca2a3b6e0c83114d96bf382431fa21c00c9d818dc76721ed0ed09838560630"
             "ce2e2fc3ff2796727f0ded2147f68c040bf0b06c99184f0b53b13e966dd46b622"
             "4663f591dcb06be2c15398ad79af6155478d888c0cec4d0f008f0469a084a21a0"
             "06ad610832938232cd672079fd672c29cfe44a9fe28029e4474b1d0efdf09ca6c"
             "99958969864e1a0483236c9a496f6753bd1dae2169f4a4a665d28907e5347aa30"
             "b181fa891a3d13c97612292424a7d21f89806e9ae3161be2e1067f7e5821c352c"
             "f985af08d990b2d5595dcf6aee29ba8f6a906990bb2407447e64dc31fdbb925db"
             "a728427683ef16e6fcde7b982390314a10cc5bd8c3a3fc9d4b1544a966301dbfd"
             "a478712ea9de748ed1120bd864dab49694680dfdf647cb5d263d0a591c737fd38"
             "15475cbf0006bf0b638870865f9118936e144b4e7315763a5e526450325e1966e"
             "d32af3ec4f5c07231e161f4f006d0b61cd3a747951d29a6af505a27264206786b"
             "8de5339ea1972c7e11027e77f90a5c9b11f5d2800490da63f1a94ffbb0bccc057"
             "f1be13eeae5cc8da783d3b84e2ae3aa424f54a663a4a9f9e67810f00b833ec015"
             "6377a6b96eb8b53e335f018af4b8be94118485b2d3b53652e890526d1a41bded7"
             "141400a8cc33116507392c3db3dddf3291d97543c77e9a2c616dfe130f23d0bc3"
             "733b0f2843d32c51d0e04e7932ad21ec5e9be6dd6b86e541e2323ccf8b209ad09"
             "40b7222d4aaa91d8837fe42cf46b785af711ea8c6600320be68fcd657241e8efb"
             "16dde17e25f5adcf601aed934acfb3a82a2245a46f8b224527eb3ca48beab1f05"
             "2a044b9a7ef7d12a11c7e81bc72b0d3fce26f522a6180a762742d1e0ea79950a0"
             "00f653cf348876d1b2a42b4c7524dc906089023d96eff593c6eb9f0f4ecbd3248"
             "00000101000000010000000000000000000000000000000000000000000000000"
             "000000000000000ffffffff0100ffffffff020000008011040000220020d5c100"
             "5c0d4012d3ae2672319e7f9eb15a57516aeefabbbc062265f67e308f2b0000000"
             "000000000326a3009f91102ffff0f20f935b3001ef51ba8f24921a404bc376a0c"
             "713274bd1cc68c2c57f66f5c0be7ca001000000000000000000000",
             {981000,
              "b9c7b3d95bdd5dbd5cc396ea64ced3011dee94d19a480790bd5288c81a8d732"
              "5",
              "9fbaa91731883143a066b8aa239e71c6095efe07caa4501af2ff664de9894b3"
              "4",
              "ad3f11fe94a2c7f6b18f0c206712c7bab44eaeb8a4a469daaa009043eec092e"
              "1"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             137298172,
             64764,
             {
                 "seed.cjd.li",
                 "seed.anode.co",
                 "pktdseed.pkt.world",
             },
             1000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::PKT_testnet,
         {
             false,
             true,
             true,
             4u,
             opentxs::UnitType::Tnpkt,
             Bip44Type::TESTNET,
             521142271,  // 0x1f0fffff
             "00000000000000000000000000000000000000000000000000000000000000000"
             "0000000df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36a"
             "c891fb00000000ffff0f1f00000000",
             "852d43936f4c9606a4a063cf356c454f2d9c43b07a41cf52e59461a41217dc0b",
             "TODO genesis block goes here",
             {0,
              "852d43936f4c9606a4a063cf356c454f2d9c43b07a41cf52e59461a41217dc0"
              "b",
              "0000000000000000000000000000000000000000000000000000000000000000"
              "526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd36"
              "c"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             118034940,
             64764,
             {
                 "testseed.cjd.li",
                 "testseed.anode.co",
                 "testseed.gridfinity.com",
             },
             1000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, false},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             100,
         }},
        {blockchain::Type::UnitTest,
         {
             false,
             true,
             false,
             0,
             opentxs::UnitType::Regtest,
             Bip44Type::TESTNET,
             545259519,  // 0x207fffff
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4adae5494dffff7f2002000000",
             "06226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf188910f",
             "01000000000000000000000000000000000000000000000000000000000000000"
             "00000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b"
             "1e5e4adae5494dffff7f200200000001010000000100000000000000000000000"
             "00000000000000000000000000000000000000000ffffffff4d04ffff001d0104"
             "455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f7"
             "2206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f7220"
             "62616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a"
             "67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e5"
             "1ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
             {0,
              "06226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf188910"
              "f",
              "000000000000000000000000000000000000000000000000000000000000000"
              "0",
              "5e0aa302450f931bc2e4fab27632231a06964277ea8dfcdd93c19149a24fe78"
              "8"},
             filter::Type::ES,
             p2p::Protocol::bitcoin,
             3669344250,
             18444,
             {},
             1000,
             100,
             {
                 {Style::P2PKH, true},
                 {Style::P2SH, true},
                 {Style::P2WPKH, true},
                 {Style::P2WSH, true},
                 {Style::P2TR, true},
             },
             {
                 {Style::P2WPKH, "P2WPKH"},
                 {Style::P2PKH, "P2PKH"},
             },
             10,
         }},
    };

    return data;
}

#if OT_BLOCKCHAIN
auto Data::Filters() noexcept -> const FilterData&
{
    static const auto data = FilterData{
        {blockchain::Type::Bitcoin,
         {
             {filter::Type::Basic_BIP158,
              {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c20"
               "2",
               "017fa880"}},
             {filter::Type::ES,
              {"fad52acc389a391c1d6d94e8984fe77323fbda24fb31299b88635d7bee0278e"
               "8",
               "049dc75e903561289b0029337bcf4e6720"}},
         }},
        {blockchain::Type::BitcoinCash,
         {
             {filter::Type::Basic_BCHVariant,
              {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c20"
               "2",
               "017fa880"}},
             {filter::Type::ES,
              {"fad52acc389a391c1d6d94e8984fe77323fbda24fb31299b88635d7bee0278e"
               "8",
               "049dc75e903561289b0029337bcf4e6720"}},
         }},
        {blockchain::Type::Litecoin,
         {
             {filter::Type::Basic_BIP158,
              {"8aa75530308cf8247a151c37c24e7aaa281ae3b5cecedb581aacb3a0d07c245"
               "1",
               "019e8738"}},
             {filter::Type::ES,
              {"23b8dae37cf04c8a278bd50bcbcf23a03051ea902f67c4760eb35be96d42832"
               "0",
               "049de896b2cc882671e81f336fdf119b00"}},
         }},
        {blockchain::Type::Bitcoin_testnet3,
         {
             {filter::Type::Basic_BIP158,
              {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb77945582"
               "1",
               "019dfca8"}},
             {filter::Type::ES,
              {"995cfe5d055c9158c5a388b71fb2ddbe292c9ca2d30dca91359d8cbbe4603e0"
               "2",
               "04e2f5880d851afd74c662d38d49e29130"}},
         }},
        {blockchain::Type::BitcoinCash_testnet3,
         {
             {filter::Type::Basic_BCHVariant,
              {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb77945582"
               "1",
               "019dfca8"}},
             {filter::Type::ES,
              {"995cfe5d055c9158c5a388b71fb2ddbe292c9ca2d30dca91359d8cbbe4603e0"
               "2",
               "04e2f5880d851afd74c662d38d49e29130"}},
         }},
        {blockchain::Type::Litecoin_testnet4,
         {
             {filter::Type::Basic_BIP158,
              {"02d023da9d271b849f717089aad7e03a515dac982c9fb2cfd952e2ce1c61879"
               "2",
               "014c8c60"}},
             {filter::Type::ES,
              {"ad242bb97aaf6a8f973dc2054d5356a4fcc87f575b29bbb3e0d953cfaedff8c"
               "6",
               "048b3d60cc5692c061eb30ca191005f1c0"}},
         }},
        {blockchain::Type::PKT,
         {
             {filter::Type::Basic_BIP158,
              {"526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd36"
               "c",
               "01902168"}},
             {filter::Type::ES,
              {"155e1700eff3f9019ba1716316295a8753ec44d2a7730eee1c1c73e2b511e13"
               "4",
               "02649a42b26e818d40"}},
         }},
        {blockchain::Type::PKT_testnet,
         {
             {filter::Type::Basic_BIP158,
              {"526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd36"
               "c",
               "01902168"}},
             {filter::Type::ES,
              {"155e1700eff3f9019ba1716316295a8753ec44d2a7730eee1c1c73e2b511e13"
               "4",
               "02649a42b26e818d40"}},
         }},
        {blockchain::Type::UnitTest,
         {
             {filter::Type::Basic_BIP158,
              {"2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e4"
               "8",
               "014756c0"}},
             {filter::Type::Basic_BCHVariant,
              {"2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e4"
               "8",
               "014756c0"}},
             {filter::Type::ES,
              {"5e0aa302450f931bc2e4fab27632231a06964277ea8dfcdd93c19149a24fe78"
               "8",
               "042547f61f786604db036044c4f7f36fe0"}},
         }},
    };

    return data;
}

auto Data::Services() noexcept -> const ServiceBits&
{
    static const auto data = ServiceBits{
        {blockchain::Type::Bitcoin,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::Bitcoin_testnet3,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::BitcoinCash,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::Graphene},
             {p2p::bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
             {p2p::bitcoin::Service::Bit9, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit10, p2p::Service::XThinner},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
             {p2p::bitcoin::Service::Bit25, p2p::Service::Avalanche},
         }},
        {blockchain::Type::BitcoinCash_testnet3,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::Graphene},
             {p2p::bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
             {p2p::bitcoin::Service::Bit9, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit10, p2p::Service::XThinner},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
             {p2p::bitcoin::Service::Bit25, p2p::Service::Avalanche},
         }},
        {blockchain::Type::Litecoin,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::Litecoin_testnet4,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::PKT,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::PKT_testnet,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::UnitTest,
         {
             {p2p::bitcoin::Service::None, p2p::Service::None},
             {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
             {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
             {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
             {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
             {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
             {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
    };

    return data;
}
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::params
