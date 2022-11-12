// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PaymentCode
// IWYU pragma: no_forward_declare opentxs::blockchain::SigHash
// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoState
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoTag

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Block.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Input.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Inputs.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Outputs.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Script.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: associated
#include "internal/blockchain/block/Block.hpp"   // IWYU pragma: associated
#include "internal/blockchain/block/Header.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::blockchain::bitcoin::block::blank
{
class Script final : public internal::Script
{
public:
    auto at(const std::size_t) const noexcept(false) -> const value_type& final
    {
        static const auto blank = value_type{};

        return blank;
    }
    auto begin() const noexcept -> const_iterator final { return {this}; }
    auto CalculateHash160(const api::Crypto&, Writer&&) const noexcept
        -> bool final
    {
        return {};
    }
    auto CalculateSize() const noexcept -> std::size_t final { return {}; }
    auto cbegin() const noexcept -> const_iterator final { return {this}; }
    auto cend() const noexcept -> const_iterator final { return {this}; }
    auto clone() const noexcept -> std::unique_ptr<internal::Script> final
    {
        return std::make_unique<blank::Script>();
    }
    auto end() const noexcept -> const_iterator final { return {this}; }
    auto ExtractElements(const cfilter::Type, Elements&) const noexcept
        -> void final
    {
    }
    auto IndexElements(const api::Session&, ElementHashes&) const noexcept
        -> void final
    {
    }
    auto IsNotification(const std::uint8_t, const PaymentCode&) const noexcept
        -> bool final
    {
        return {};
    }
    auto LikelyPubkeyHashes(const api::Crypto&) const noexcept
        -> UnallocatedVector<ByteArray> final
    {
        return {};
    }
    auto M() const noexcept -> std::optional<std::uint8_t> final { return {}; }
    auto MultisigPubkey(const std::size_t) const noexcept
        -> std::optional<ReadView> final
    {
        return {};
    }
    auto N() const noexcept -> std::optional<std::uint8_t> final { return {}; }
    auto Print() const noexcept -> UnallocatedCString final { return {}; }
    auto Pubkey() const noexcept -> std::optional<ReadView> final { return {}; }
    auto PubkeyHash() const noexcept -> std::optional<ReadView> final
    {
        return {};
    }
    auto RedeemScript() const noexcept -> std::unique_ptr<block::Script> final
    {
        return clone();
    }
    auto Role() const noexcept -> Position final { return {}; }
    auto ScriptHash() const noexcept -> std::optional<ReadView> final
    {
        return {};
    }
    auto SigningSubscript(const blockchain::Type) const noexcept
        -> std::unique_ptr<internal::Script> final
    {
        return clone();
    }
    auto Serialize(Writer&&) const noexcept -> bool final { return {}; }
    auto size() const noexcept -> std::size_t final { return {}; }
    auto Type() const noexcept -> Pattern final { return {}; }
    auto Value(const std::size_t) const noexcept
        -> std::optional<ReadView> final
    {
        return {};
    }

    ~Script() final = default;
};

class Output final : public internal::Output
{
public:
    auto AssociatedLocalNyms(
        const api::crypto::Blockchain&,
        UnallocatedVector<identifier::Nym>&) const noexcept -> void final
    {
    }
    auto AssociatedRemoteContacts(
        const api::session::Client&,
        UnallocatedVector<identifier::Generic>&) const noexcept -> void final
    {
    }
    auto CalculateSize() const noexcept -> std::size_t final { return {}; }
    auto clone() const noexcept -> std::unique_ptr<internal::Output> final
    {
        return std::make_unique<blank::Output>();
    }
    auto ExtractElements(const cfilter::Type, Elements&) const noexcept
        -> void final
    {
    }
    auto FindMatches(
        const api::Session&,
        const Txid&,
        const cfilter::Type,
        const ParsedPatterns&,
        const Log&,
        Matches&,
        alloc::Default) const noexcept -> void final
    {
    }
    auto IndexElements(const api::Session&, ElementHashes&) const noexcept
        -> void final
    {
    }
    auto MinedPosition() const noexcept -> const block::Position& final
    {
        static const auto blank = block::Position{};

        return blank;
    }
    auto NetBalanceChange(
        const api::crypto::Blockchain&,
        const identifier::Nym&,
        const Log&) const noexcept -> opentxs::Amount final
    {
        return {};
    }
    auto Note(const api::crypto::Blockchain&) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return {};
    }
    auto Payee() const noexcept -> ContactID final { return {}; }
    auto Payer() const noexcept -> ContactID final { return {}; }
    auto Print() const noexcept -> UnallocatedCString final { return {}; }
    auto Serialize(Writer&&) const noexcept -> std::optional<std::size_t> final
    {
        return {};
    }
    auto Serialize(const api::Session&, SerializeType&) const noexcept
        -> bool final
    {
        return {};
    }
    auto Script() const noexcept -> const block::Script& final
    {
        static const auto blank = blank::Script{};

        return blank;
    }
    auto SigningSubscript() const noexcept
        -> std::unique_ptr<internal::Script> final
    {
        return std::make_unique<blank::Script>();
    }
    auto State() const noexcept -> node::TxoState final { return {}; }
    auto Tags() const noexcept -> const UnallocatedSet<node::TxoTag> final
    {
        return {};
    }
    auto Value() const noexcept -> blockchain::Amount final { return {}; }

    auto AddTag(node::TxoTag) noexcept -> void final {}
    auto ForTestingOnlyAddKey(const crypto::Key&) noexcept -> void final {}
    auto MergeMetadata(const internal::Output&, const Log&) noexcept
        -> bool final
    {
        return {};
    }
    auto SetIndex(const std::uint32_t) noexcept -> void final {}
    auto SetKeyData(const KeyData&) noexcept -> void final {}
    auto SetMinedPosition(const block::Position&) noexcept -> void final {}
    auto SetPayee(const identifier::Generic&) noexcept -> void final {}
    auto SetPayer(const identifier::Generic&) noexcept -> void final {}
    auto SetState(node::TxoState) noexcept -> void final {}
    auto SetValue(const blockchain::Amount& value) noexcept -> void final {}

    ~Output() final = default;
};

class Input final : public internal::Input
{
public:
    auto AssociatedLocalNyms(
        const api::crypto::Blockchain&,
        UnallocatedVector<identifier::Nym>&) const noexcept -> void final
    {
    }
    auto AssociatedRemoteContacts(
        const api::session::Client& api,
        UnallocatedVector<identifier::Generic>&) const noexcept -> void final
    {
    }
    auto CalculateSize(const bool) const noexcept -> std::size_t final
    {
        return {};
    }
    auto clone() const noexcept -> std::unique_ptr<internal::Input> final
    {
        return std::make_unique<blank::Input>();
    }
    auto Coinbase() const noexcept -> Space final { return {}; }
    auto ExtractElements(const cfilter::Type, Elements&) const noexcept
        -> void final
    {
    }
    auto FindMatches(
        const api::Session&,
        const Txid&,
        const cfilter::Type,
        const Patterns&,
        const ParsedPatterns&,
        const std::size_t,
        const Log&,
        Matches&,
        alloc::Default) const noexcept -> void final
    {
    }
    auto GetBytes(std::size_t&, std::size_t&) const noexcept -> void final {}
    auto IndexElements(const api::Session&, ElementHashes&) const noexcept
        -> void final
    {
    }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return {};
    }
    auto NetBalanceChange(
        const api::crypto::Blockchain&,
        const identifier::Nym&,
        const std::size_t,
        const Log&) const noexcept -> opentxs::Amount final
    {
        return {};
    }
    auto PreviousOutput() const noexcept -> const Outpoint& final
    {
        static const auto blank = Outpoint{};

        return blank;
    }
    auto Print() const noexcept -> UnallocatedCString final { return {}; }
    auto Script() const noexcept -> const block::Script& final
    {
        static const auto blank = blank::Script{};

        return blank;
    }
    auto Sequence() const noexcept -> std::uint32_t final { return {}; }
    auto Serialize(Writer&&) const noexcept -> std::optional<std::size_t> final
    {
        return {};
    }
    auto Serialize(const api::Session&, const std::uint32_t, SerializeType&)
        const noexcept -> bool final
    {
        return {};
    }
    auto SerializeNormalized(Writer&&) const noexcept
        -> std::optional<std::size_t> final
    {
        return {};
    }
    auto SignatureVersion() const noexcept
        -> std::unique_ptr<internal::Input> final
    {
        return clone();
    }
    auto SignatureVersion(std::unique_ptr<internal::Script>) const noexcept
        -> std::unique_ptr<internal::Input> final
    {
        return clone();
    }
    auto Spends() const noexcept(false) -> const Output& final
    {
        static const auto blank = blank::Output{};

        return blank;
    }
    auto Witness() const noexcept -> const UnallocatedVector<Space>& final
    {
        static const auto blank = UnallocatedVector<Space>{};

        return blank;
    }

    auto AddMultisigSignatures(const Signatures&) noexcept -> bool final
    {
        return {};
    }
    auto AddSignatures(const Signatures&) noexcept -> bool final { return {}; }
    auto AssociatePreviousOutput(const internal::Output&) noexcept -> bool final
    {
        return {};
    }
    auto MergeMetadata(
        const internal::Input&,
        const std::size_t,
        const Log&) noexcept -> bool final
    {
        return {};
    }
    auto ReplaceScript() noexcept -> bool final { return {}; }
    auto SetKeyData(const KeyData&) noexcept -> void final {}

    ~Input() final = default;
};

class Outputs final : public internal::Outputs
{
public:
    auto AssociatedLocalNyms(
        const api::crypto::Blockchain&,
        UnallocatedVector<identifier::Nym>&) const noexcept -> void final
    {
    }
    auto AssociatedRemoteContacts(
        const api::session::Client&,
        UnallocatedVector<identifier::Generic>&) const noexcept -> void final
    {
    }
    auto at(const std::size_t) const noexcept(false) -> const value_type& final
    {
        static const auto blank = blank::Output{};

        return blank;
    }
    auto begin() const noexcept -> const_iterator final { return {this}; }
    auto CalculateSize() const noexcept -> std::size_t final { return {}; }
    auto cbegin() const noexcept -> const_iterator final { return {this}; }
    auto cend() const noexcept -> const_iterator final { return {this}; }
    auto clone() const noexcept -> std::unique_ptr<internal::Outputs> final
    {
        return std::make_unique<blank::Outputs>();
    }
    auto end() const noexcept -> const_iterator final { return {this}; }
    auto ExtractElements(const cfilter::Type, Elements&) const noexcept
        -> void final
    {
    }
    auto FindMatches(
        const api::Session&,
        const Txid&,
        const cfilter::Type,
        const ParsedPatterns&,
        const Log&,
        Matches&,
        alloc::Default) const noexcept -> void final
    {
    }
    auto IndexElements(const api::Session&, ElementHashes&) const noexcept
        -> void final
    {
    }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return {};
    }
    auto NetBalanceChange(
        const api::crypto::Blockchain&,
        const identifier::Nym&,
        const Log&) const noexcept -> opentxs::Amount final
    {
        return {};
    }
    auto Serialize(Writer&&) const noexcept -> std::optional<std::size_t> final
    {
        return {};
    }
    auto Serialize(const api::Session&, proto::BlockchainTransaction&)
        const noexcept -> bool final
    {
        return {};
    }
    auto size() const noexcept -> std::size_t final { return {}; }

    auto at(const std::size_t) noexcept(false) -> value_type& final
    {
        static auto blank = blank::Output{};

        return blank;
    }
    auto ForTestingOnlyAddKey(
        const std::size_t,
        const blockchain::crypto::Key&) noexcept -> bool final
    {
        return {};
    }
    auto MergeMetadata(const internal::Outputs&, const Log&) noexcept
        -> bool final
    {
        return {};
    }
    auto SetKeyData(const KeyData&) noexcept -> void final {}

    ~Outputs() final = default;
};

class Inputs final : public internal::Inputs
{
public:
    auto AssociatedLocalNyms(
        const api::crypto::Blockchain&,
        UnallocatedVector<identifier::Nym>&) const noexcept -> void final
    {
    }
    auto AssociatedRemoteContacts(
        const api::session::Client&,
        UnallocatedVector<identifier::Generic>&) const noexcept -> void final
    {
    }
    auto at(const std::size_t) const noexcept(false) -> const value_type& final
    {
        static const auto blank = blank::Input{};

        return blank;
    }
    auto begin() const noexcept -> const_iterator final { return {this}; }
    auto CalculateSize(const bool) const noexcept -> std::size_t final
    {
        return {};
    }
    auto cbegin() const noexcept -> const_iterator final { return {this}; }
    auto cend() const noexcept -> const_iterator final { return {this}; }
    auto clone() const noexcept -> std::unique_ptr<internal::Inputs> final
    {
        return std::make_unique<blank::Inputs>();
    }
    auto end() const noexcept -> const_iterator final { return {this}; }
    auto ExtractElements(const cfilter::Type, Elements&) const noexcept
        -> void final
    {
    }
    auto FindMatches(
        const api::Session&,
        const Txid&,
        const cfilter::Type,
        const Patterns&,
        const ParsedPatterns&,
        const Log&,
        Matches&,
        alloc::Default) const noexcept -> void final
    {
    }
    auto IndexElements(const api::Session&, ElementHashes&) const noexcept
        -> void final
    {
    }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return {};
    }
    auto NetBalanceChange(
        const api::crypto::Blockchain&,
        const identifier::Nym&,
        const Log&) const noexcept -> opentxs::Amount final
    {
        return {};
    }
    auto Serialize(Writer&&) const noexcept -> std::optional<std::size_t> final
    {
        return {};
    }
    auto Serialize(const api::Session&, proto::BlockchainTransaction&)
        const noexcept -> bool final
    {
        return {};
    }
    auto SerializeNormalized(Writer&&) const noexcept
        -> std::optional<std::size_t> final
    {
        return {};
    }
    auto size() const noexcept -> std::size_t final { return {}; }

    auto AnyoneCanPay(const std::size_t) noexcept -> bool final { return {}; }
    auto AssociatePreviousOutput(
        const std::size_t,
        const internal::Output&) noexcept -> bool final
    {
        return {};
    }
    auto at(const std::size_t) noexcept(false) -> value_type& final
    {
        static auto blank = blank::Input{};

        return blank;
    }
    auto MergeMetadata(const internal::Inputs&, const Log&) noexcept
        -> bool final
    {
        return {};
    }
    auto ReplaceScript(const std::size_t) noexcept -> bool final { return {}; }
    auto SetKeyData(const KeyData&) noexcept -> void final {}

    ~Inputs() final = default;
};

class Transaction final : public internal::Transaction
{
public:
    auto AssociatedLocalNyms(const api::crypto::Blockchain&) const noexcept
        -> UnallocatedVector<identifier::Nym> final
    {
        return {};
    }
    auto AssociatedRemoteContacts(
        const api::session::Client&,
        const identifier::Nym&) const noexcept
        -> UnallocatedVector<identifier::Generic> final
    {
        return {};
    }
    auto BlockPosition() const noexcept -> std::optional<std::size_t> final
    {
        return {};
    }
    auto Chains() const noexcept -> UnallocatedVector<blockchain::Type> final
    {
        return {};
    }
    auto clone() const noexcept -> std::unique_ptr<block::Transaction> final
    {
        return std::make_unique<blank::Transaction>();
    }
    auto ConfirmationHeight() const noexcept -> block::Height final
    {
        return {};
    }
    auto GetPreimageBTC(const std::size_t, const blockchain::bitcoin::SigHash&)
        const noexcept -> Space final
    {
        return {};
    }
    auto ID() const noexcept -> const Txid& final
    {
        static const auto blank = ByteArray{};

        return blank;
    }
    auto Inputs() const noexcept -> const block::Inputs& final
    {
        static const auto blank = blank::Inputs{};

        return blank;
    }
    auto IsGeneration() const noexcept -> bool final { return {}; }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return {};
    }
    auto Locktime() const noexcept -> std::uint32_t final { return {}; }
    auto Memo(const api::crypto::Blockchain&) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto MinedPosition() const noexcept -> const block::Position& final
    {
        static const auto blank = block::Position{};

        return blank;
    }
    auto NetBalanceChange(
        const api::crypto::Blockchain&,
        const identifier::Nym&) const noexcept -> opentxs::Amount final
    {
        return {};
    }
    auto Outputs() const noexcept -> const block::Outputs& final
    {
        static const auto blank = blank::Outputs{};

        return blank;
    }
    auto Print() const noexcept -> UnallocatedCString final { return {}; }
    auto SegwitFlag() const noexcept -> std::byte final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Version() const noexcept -> std::int32_t final { return {}; }
    auto vBytes(blockchain::Type) const noexcept -> std::size_t final
    {
        return {};
    }
    auto WTXID() const noexcept -> const Txid& final { return ID(); }

    auto AssociatePreviousOutput(
        const std::size_t,
        const internal::Output&) noexcept -> bool final
    {
        return {};
    }
    auto CalculateSize() const noexcept -> std::size_t final { return {}; }
    auto ExtractElements(const cfilter::Type, Elements&) const noexcept
        -> void final
    {
    }
    auto FindMatches(
        const api::Session&,
        const cfilter::Type,
        const Patterns&,
        const ParsedPatterns&,
        const Log&,
        alloc::Default,
        alloc::Default) const noexcept -> Matches final
    {
        return {};
    }
    auto FindMatches(
        const api::Session&,
        const cfilter::Type,
        const Patterns&,
        const ParsedPatterns&,
        const Log&,
        Matches&,
        alloc::Default) const noexcept -> void final
    {
    }
    auto ForTestingOnlyAddKey(
        const std::size_t,
        const blockchain::crypto::Key&) noexcept -> bool final
    {
        return {};
    }
    auto IDNormalized(const api::Factory&) const noexcept
        -> const identifier::Generic& final
    {
        static const auto blank = identifier::Generic{};

        return blank;
    }
    auto IndexElements(const api::Session&, alloc::Default) const noexcept
        -> ElementHashes final
    {
        return {};
    }
    auto MergeMetadata(
        const api::crypto::Blockchain&,
        const blockchain::Type,
        const internal::Transaction&,
        const Log&) noexcept -> void final
    {
    }
    auto Serialize(Writer&&) const noexcept -> std::optional<std::size_t> final
    {
        return {};
    }
    auto Serialize(const api::Session&) const noexcept
        -> std::optional<SerializeType> final
    {
        return {};
    }
    auto SetKeyData(const KeyData&) noexcept -> void final {}
    auto SetMemo(const UnallocatedCString&) noexcept -> void final {}
    auto SetMinedPosition(const block::Position&) noexcept -> void final {}
    auto SetPosition(std::size_t) noexcept -> void final {}

    ~Transaction() final = default;
};

class Block final : public blockchain::block::internal::Block,
                    public internal::Block
{
public:
    auto asBitcoin() const noexcept -> const bitcoin::block::Block& final
    {
        return *this;
    }
    auto at(const std::size_t) const noexcept -> const value_type& final
    {
        static const value_type blank = std::make_shared<blank::Transaction>();

        return blank;
    }
    auto at(const ReadView) const noexcept -> const value_type& final
    {
        static const value_type blank = std::make_shared<blank::Transaction>();

        return blank;
    }
    auto begin() const noexcept -> const_iterator final { return {this}; }
    auto CalculateSize() const noexcept -> std::size_t final { return {}; }
    auto cbegin() const noexcept -> const_iterator final { return {this}; }
    auto cend() const noexcept -> const_iterator final { return {this}; }
    auto clone_bitcoin() const noexcept
        -> std::unique_ptr<internal::Block> final
    {
        return std::make_unique<Block>();
    }
    auto end() const noexcept -> const_iterator final { return {this}; }
    auto ExtractElements(const cfilter::Type, alloc::Default) const noexcept
        -> Elements final
    {
        return {};
    }
    auto FindMatches(
        const api::Session&,
        const cfilter::Type,
        const Patterns&,
        const Patterns&,
        const Log&,
        alloc::Default,
        alloc::Default) const noexcept -> Matches final
    {
        return {};
    }
    auto Header() const noexcept -> const blockchain::block::Header& final
    {
        static const auto blank = blockchain::block::Header{};

        return blank;
    }
    auto ID() const noexcept -> const block::Hash& final
    {
        static const auto blank = block::Hash{};

        return blank;
    }
    auto Print() const noexcept -> UnallocatedCString final { return {}; }
    auto Serialize(Writer&&) const noexcept -> bool final { return {}; }
    auto size() const noexcept -> std::size_t final { return {}; }

    auto asBitcoin() noexcept -> bitcoin::block::Block& final { return *this; }

    ~Block() final = default;
};

class Header final : public internal::Header
{
public:
    auto as_Bitcoin() const noexcept
        -> const blockchain::bitcoin::block::internal::Header& final
    {
        return *this;
    }
    auto clone_bitcoin() const noexcept -> std::unique_ptr<block::Header> final
    {
        return std::make_unique<block::Header>();
    }
    auto EffectiveState() const noexcept -> Status final { return {}; }
    auto InheritedState() const noexcept -> Status final { return {}; }
    auto IsBlacklisted() const noexcept -> bool final { return {}; }
    auto IsDisconnected() const noexcept -> bool final { return {}; }
    auto LocalState() const noexcept -> Status final { return {}; }
    auto Serialize(SerializedType&) const noexcept -> bool final { return {}; }

    auto as_Bitcoin() noexcept
        -> blockchain::bitcoin::block::internal::Header& final
    {
        return *this;
    }
    auto CompareToCheckpoint(const block::Position&) noexcept -> void final {}
    auto InheritHeight(const blockchain::block::Header&) -> void final {}
    auto InheritState(const blockchain::block::Header&) -> void final {}
    auto InheritWork(const blockchain::Work&) noexcept -> void final {}
    auto RemoveBlacklistState() noexcept -> void final {}
    auto RemoveCheckpointState() noexcept -> void final {}
    auto SetDisconnectedState() noexcept -> void final {}
};
}  // namespace opentxs::blockchain::bitcoin::block::blank

namespace opentxs::blockchain::bitcoin::block::internal
{
auto Header::Blank() noexcept -> Header&
{
    static auto blank = blank::Header{};

    return blank;
}
}  // namespace opentxs::blockchain::bitcoin::block::internal

namespace opentxs::factory
{
auto BitcoinBlock() noexcept
    -> std::shared_ptr<blockchain::bitcoin::block::Block>
{
    return std::make_shared<blockchain::bitcoin::block::blank::Block>();
}

auto BitcoinScript() noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return std::make_unique<blockchain::bitcoin::block::blank::Script>();
}

auto BitcoinTransaction() noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    return std::unique_ptr<blockchain::bitcoin::block::blank::Transaction>();
}

auto BitcoinTransactionInput() noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    return std::unique_ptr<blockchain::bitcoin::block::blank::Input>();
}

auto BitcoinTransactionInputs() noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Inputs>
{
    return std::unique_ptr<blockchain::bitcoin::block::blank::Inputs>();
}

auto BitcoinTransactionOutput() noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    return std::unique_ptr<blockchain::bitcoin::block::blank::Output>();
}

auto BitcoinTransactionOutputs() noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Outputs>
{
    return std::unique_ptr<blockchain::bitcoin::block::blank::Outputs>();
}
}  // namespace opentxs::factory
