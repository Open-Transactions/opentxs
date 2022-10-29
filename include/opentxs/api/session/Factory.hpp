// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"
// IWYU pragma: no_include "opentxs/otx/blind/CashType.hpp"

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
class Script;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Hash;
class Header;
}  // namespace block

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key

class Key;
}  // namespace asymmetric

class Parameters;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identifier
{
class Generic;
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace otdht
{
class Base;
}  // namespace otdht

namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace blind
{
class Mint;
class Purse;
}  // namespace blind

namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class ByteArray;
class Item;
class Secret;
class PaymentCode;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
/**
 The Factory API for opentxs sessions, used for instantiating many different
 object types native to opentxs.
 */
class OPENTXS_EXPORT Factory : virtual public api::Factory
{
public:
    virtual auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto BitcoinBlock(
        const blockchain::Type chain,
        const ReadView bytes) const noexcept
        -> std::shared_ptr<const blockchain::bitcoin::block::Block> = 0;
    using Transaction_p =
        std::shared_ptr<const blockchain::bitcoin::block::Transaction>;
    using AbortFunction = std::function<bool()>;
    virtual auto BitcoinBlock(
        const blockchain::block::Header& previous,
        const Transaction_p generationTransaction,
        const std::uint32_t nBits,
        const UnallocatedVector<Transaction_p>& extraTransactions = {},
        const std::int32_t version = 2,
        const AbortFunction abort = {}) const noexcept
        -> std::shared_ptr<const blockchain::bitcoin::block::Block> = 0;
    virtual auto BitcoinGenerationTransaction(
        const blockchain::Type chain,
        const blockchain::block::Height height,
        UnallocatedVector<blockchain::OutputBuilder>&& outputs,
        const UnallocatedCString& coinbase = {},
        const std::int32_t version = 1) const noexcept -> Transaction_p = 0;
    virtual auto BitcoinScriptNullData(
        const blockchain::Type chain,
        const UnallocatedVector<ReadView>& data) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2MS(
        const blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        const UnallocatedVector<
            const opentxs::crypto::asymmetric::key::EllipticCurve*>& publicKeys)
        const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2PK(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& publicKey)
        const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2PKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& publicKey)
        const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2SH(
        const blockchain::Type chain,
        const blockchain::bitcoin::block::Script& script) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2WPKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& publicKey)
        const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2WSH(
        const blockchain::Type chain,
        const blockchain::bitcoin::block::Script& script) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinTransaction(
        const blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time& time = Clock::now()) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Transaction> = 0;
    virtual auto BlockchainAddress(
        const blockchain::p2p::Protocol protocol,
        const blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const UnallocatedSet<blockchain::p2p::Service>& services,
        const bool incoming = false) const -> blockchain::p2p::Address = 0;
    virtual auto BlockchainSyncMessage(
        const opentxs::network::zeromq::Message& in) const noexcept
        -> std::unique_ptr<opentxs::network::otdht::Base> = 0;
    using BlockHeaderP = std::unique_ptr<blockchain::block::Header>;
    virtual auto BlockHeader(const ReadView protobuf) const -> BlockHeaderP = 0;
    virtual auto BlockHeader(const blockchain::Type type, const ReadView native)
        const -> BlockHeaderP = 0;
    virtual auto BlockHeader(const blockchain::block::Block& block) const
        -> BlockHeaderP = 0;
    virtual auto Data() const -> ByteArray = 0;
    virtual auto Data(const opentxs::Armored& input) const -> ByteArray = 0;
    virtual auto Data(const opentxs::network::zeromq::Frame& input) const
        -> ByteArray = 0;
    virtual auto Data(const std::uint8_t input) const -> ByteArray = 0;
    virtual auto Data(const std::uint32_t input) const -> ByteArray = 0;
    virtual auto Data(const UnallocatedVector<unsigned char>& input) const
        -> ByteArray = 0;
    virtual auto Data(const UnallocatedVector<std::byte>& input) const
        -> ByteArray = 0;
    virtual auto DataFromBytes(const ReadView input) const -> ByteArray = 0;
    virtual auto DataFromHex(const ReadView input) const -> ByteArray = 0;
    OPENTXS_NO_EXPORT virtual auto InternalSession() const noexcept
        -> const internal::Factory& = 0;
    virtual auto Mint() const noexcept -> otx::blind::Mint = 0;
    virtual auto Mint(const otx::blind::CashType type) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto NymIDFromPaymentCode(
        const UnallocatedCString& serialized) const -> identifier::Nym = 0;
    virtual auto PasswordPrompt(std::string_view text) const
        -> opentxs::PasswordPrompt = 0;
    virtual auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> opentxs::PasswordPrompt = 0;
    virtual auto PaymentCode(const UnallocatedCString& base58) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(const ReadView& serialized) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(
        const UnallocatedCString& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;

    OPENTXS_NO_EXPORT virtual auto InternalSession() noexcept
        -> internal::Factory& = 0;

    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    OPENTXS_NO_EXPORT ~Factory() override = default;

protected:
    Factory() = default;
};
}  // namespace opentxs::api::session
