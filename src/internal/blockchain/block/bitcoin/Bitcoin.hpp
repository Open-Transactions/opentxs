// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
class Inventory;
struct EncodedInput;
struct EncodedOutpoint;
struct EncodedOutput;
struct EncodedTransaction;
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
namespace internal
{
struct Header;
struct Output;
struct Script;
}  // namespace internal

class Block;
class Header;
class Input;
class Inputs;
class Output;
class Outputs;
class Script;
class Transaction;
}  // namespace bitcoin

class Header;
class Outpoint;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network

namespace proto
{
class BlockchainBlockHeader;
class BlockchainTransaction;
class BlockchainTransactionInput;
class BlockchainTransactionOutput;
}  // namespace proto

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::bitcoin::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height;
auto EncodeBip34(block::Height height) noexcept -> Space;
auto Opcode(const OP opcode) noexcept(false) -> ScriptElement;
auto PushData(const ReadView data) noexcept(false) -> ScriptElement;

struct Header : virtual public bitcoin::Header {
};

struct Input : virtual public bitcoin::Input {
    using SerializeType = proto::BlockchainTransactionInput;
    using Signature = std::pair<ReadView, ReadView>;
    using Signatures = UnallocatedVector<Signature>;

    virtual auto AssociatedLocalNyms(
        UnallocatedVector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        UnallocatedVector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto CalculateSize(const bool normalized = false) const noexcept
        -> std::size_t = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto FindMatches(
        const ReadView txid,
        const filter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetBytes(std::size_t& base, std::size_t& witness)
        const noexcept -> void = 0;
    virtual auto GetPatterns() const noexcept
        -> UnallocatedVector<PatternID> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool = 0;
    virtual auto SerializeNormalized(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;
    virtual auto SignatureVersion() const noexcept
        -> std::unique_ptr<Input> = 0;
    virtual auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto Spends() const noexcept(false) -> const Output& = 0;

    virtual auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool = 0;
    virtual auto AddSignatures(const Signatures& signatures) noexcept
        -> bool = 0;
    virtual auto AssociatePreviousOutput(const Output& output) noexcept
        -> bool = 0;
    virtual auto MergeMetadata(const Input& rhs) noexcept -> bool = 0;
    virtual auto ReplaceScript() noexcept -> bool = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    ~Input() override = default;
};
struct Inputs : virtual public bitcoin::Inputs {
    virtual auto AssociatedLocalNyms(
        UnallocatedVector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        UnallocatedVector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto CalculateSize(const bool normalized = false) const noexcept
        -> std::size_t = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Inputs> = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto FindMatches(
        const ReadView txid,
        const filter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetPatterns() const noexcept
        -> UnallocatedVector<PatternID> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;
    virtual auto Serialize(
        proto::BlockchainTransaction& destination) const noexcept -> bool = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto SerializeNormalized(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;

    virtual auto AnyoneCanPay(const std::size_t index) noexcept -> bool = 0;
    virtual auto AssociatePreviousOutput(
        const std::size_t inputIndex,
        const Output& output) noexcept -> bool = 0;
    using bitcoin::Inputs::at;
    virtual auto at(const std::size_t position) noexcept(false)
        -> value_type& = 0;
    virtual auto MergeMetadata(const Inputs& rhs) noexcept -> bool = 0;
    virtual auto ReplaceScript(const std::size_t index) noexcept -> bool = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    ~Inputs() override = default;
};
struct Output : virtual public bitcoin::Output {
    using SerializeType = proto::BlockchainTransactionOutput;

    virtual auto AssociatedLocalNyms(
        UnallocatedVector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        UnallocatedVector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Output> = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto FindMatches(
        const ReadView txid,
        const filter::Type type,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetPatterns() const noexcept
        -> UnallocatedVector<PatternID> = 0;
    // WARNING do not call this function if another thread has a non-const
    // reference to this object
    virtual auto MinedPosition() const noexcept -> const block::Position& = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(SerializeType& destination) const noexcept
        -> bool = 0;
    virtual auto SigningSubscript() const noexcept
        -> std::unique_ptr<internal::Script> = 0;
    virtual auto State() const noexcept -> node::TxoState = 0;
    virtual auto Tags() const noexcept
        -> const UnallocatedSet<node::TxoTag> = 0;

    virtual auto AddTag(node::TxoTag tag) noexcept -> void = 0;
    virtual auto ForTestingOnlyAddKey(const crypto::Key& key) noexcept
        -> void = 0;
    virtual auto MergeMetadata(const Output& rhs) noexcept -> bool = 0;
    virtual auto SetIndex(const std::uint32_t index) noexcept -> void = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept
        -> void = 0;
    virtual auto SetPayee(const Identifier& contact) noexcept -> void = 0;
    virtual auto SetPayer(const Identifier& contact) noexcept -> void = 0;
    virtual auto SetState(node::TxoState state) noexcept -> void = 0;
    virtual auto SetValue(const blockchain::Amount& value) noexcept -> void = 0;

    ~Output() override = default;
};
struct Outputs : virtual public bitcoin::Outputs {
    virtual auto AssociatedLocalNyms(
        UnallocatedVector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        UnallocatedVector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Outputs> = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto FindMatches(
        const ReadView txid,
        const filter::Type type,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetPatterns() const noexcept
        -> UnallocatedVector<PatternID> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(
        proto::BlockchainTransaction& destination) const noexcept -> bool = 0;

    using bitcoin::Outputs::at;
    virtual auto at(const std::size_t position) noexcept(false)
        -> value_type& = 0;
    virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool = 0;
    virtual auto MergeMetadata(const Outputs& rhs) noexcept -> bool = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    ~Outputs() override = default;
};
struct Script : virtual public bitcoin::Script {
    static auto blank_signature(const blockchain::Type chain) noexcept
        -> const Space&;
    static auto blank_pubkey(
        const blockchain::Type chain,
        const bool compressed = true) noexcept -> const Space&;

    virtual auto clone() const noexcept -> std::unique_ptr<Script> = 0;
    virtual auto LikelyPubkeyHashes(const api::Session& api) const noexcept
        -> UnallocatedVector<OTData> = 0;
    virtual auto SigningSubscript(const blockchain::Type chain) const noexcept
        -> std::unique_ptr<Script> = 0;

    ~Script() override = default;
};
struct Transaction : virtual public bitcoin::Transaction {
    using SerializeType = proto::BlockchainTransaction;
    using SigHash = blockchain::bitcoin::SigOption;

    virtual auto ConfirmationHeight() const noexcept -> block::Height = 0;
    virtual auto GetPreimageBTC(
        const std::size_t index,
        const blockchain::bitcoin::SigHash& hashType) const noexcept
        -> Space = 0;
    // WARNING do not call this function if another thread has a non-const
    // reference to this object
    virtual auto MinedPosition() const noexcept -> const block::Position& = 0;

    virtual auto AssociatePreviousOutput(
        const std::size_t inputIndex,
        const Output& output) noexcept -> bool = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto FindMatches(
        const filter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetPatterns() const noexcept
        -> UnallocatedVector<PatternID> = 0;
    virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool = 0;
    virtual auto IDNormalized() const noexcept -> const Identifier& = 0;
    virtual auto MergeMetadata(
        const blockchain::Type chain,
        const Transaction& rhs) noexcept -> void = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize() const noexcept -> std::optional<SerializeType> = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;
    virtual auto SetMemo(const UnallocatedCString& memo) noexcept -> void = 0;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept
        -> void = 0;
    virtual auto SetPosition(std::size_t position) noexcept -> void = 0;

    ~Transaction() override = default;
};
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::factory
{
#if OT_BLOCKCHAIN
using UTXO = std::pair<
    blockchain::block::Outpoint,
    std::unique_ptr<blockchain::block::bitcoin::Output>>;
using Transaction_p =
    std::shared_ptr<const opentxs::blockchain::block::bitcoin::Transaction>;
using AbortFunction = std::function<bool()>;

auto BitcoinBlock(
    const api::Session& api,
    const opentxs::blockchain::block::Header& previous,
    const Transaction_p generationTransaction,
    const std::uint32_t nBits,
    const UnallocatedVector<Transaction_p>& extraTransactions,
    const std::int32_t version,
    const AbortFunction abort) noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>;
auto BitcoinBlock(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>;
auto BitcoinBlockHeader(
    const api::Session& api,
    const opentxs::blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    opentxs::blockchain::block::pHash&& merkle,
    const AbortFunction abort) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::Session& api,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView bytes) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinScript(
    const blockchain::Type chain,
    const ReadView bytes,
    const blockchain::block::bitcoin::Script::Position role,
    const bool allowInvalidOpcodes = true,
    const bool mute = false) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>;
auto BitcoinScript(
    const blockchain::Type chain,
    blockchain::block::bitcoin::ScriptElements&& elements,
    const blockchain::block::bitcoin::Script::Position role) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>;
auto BitcoinTransaction(
    const api::Session& api,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    std::unique_ptr<blockchain::block::bitcoin::internal::Inputs> inputs,
    std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>
        outputs) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
auto BitcoinTransaction(
    const api::Session& api,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
auto BitcoinTransaction(
    const api::Session& api,
    const proto::BlockchainTransaction& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
auto BitcoinTransactionInput(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool isGeneration,
    UnallocatedVector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInput(
    const api::Session& api,
    const blockchain::Type chain,
    const UTXO& spends,
    const std::optional<std::uint32_t> sequence = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInput(
    const api::Session& api,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput,
    const bool isGeneration) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInputs(
    UnallocatedVector<
        std::unique_ptr<blockchain::block::bitcoin::internal::Input>>&& inputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Inputs>;
auto BitcoinTransactionOutput(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const blockchain::block::bitcoin::internal::Script> script,
    const UnallocatedSet<blockchain::crypto::Key>& keys) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutput(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutput(
    const api::Session& api,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutputs(
    UnallocatedVector<std::unique_ptr<
        blockchain::block::bitcoin::internal::Output>>&& outputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory
