// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <future>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace crypto
{
class Element;
}  // namespace crypto

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
class Manager;
class Spend;
}  // namespace node

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Script;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
namespace be = boost::endian;

class BitcoinTransactionBuilderPrivate
{
public:
    auto BuildNormalTransaction() noexcept -> BuildResult;
    auto BuildSweepTransaction() noexcept -> BuildResult;

    auto operator()() noexcept -> BuildResult;

    BitcoinTransactionBuilderPrivate(
        const api::Session& api,
        const node::Manager& node,
        const identifier::Generic& id,
        const Type chain,
        database::Wallet& db,
        node::Spend& proposal,
        std::promise<SendOutcome>& promise) noexcept;

private:
    using Bip143 = std::optional<protocol::bitcoin::base::Bip143Hashes>;
    using Hash = std::array<std::byte, 32>;
    using Input = protocol::bitcoin::base::block::Input;
    using KeyID = crypto::Key;
    using Output = protocol::bitcoin::base::block::Output;
    using Proposal = node::Spend;
    using Transaction = protocol::bitcoin::base::block::Transaction;

    enum class Match : bool { ByValue, ByHash };

    static constexpr auto p2pkh_output_bytes_ = 34_uz;

    const api::Session& api_;
    const node::Manager& node_;
    const identifier::Generic& id_;
    const Type chain_;
    database::Wallet& db_;
    Proposal& proposal_;
    std::promise<SendOutcome>& promise_;
    const Nym_p sender_;
    const identifier::Generic self_contact_;
    const Amount fee_rate_;
    const be::little_int32_buf_t version_;
    const be::little_uint32_buf_t lock_time_;
    mutable bool segwit_;
    Vector<Output> outputs_;
    Vector<Output> change_;
    Vector<std::pair<Input, Amount>> inputs_;
    const std::size_t fixed_overhead_;
    network::blockchain::bitcoin::CompactSize input_count_;
    network::blockchain::bitcoin::CompactSize output_count_;
    std::size_t input_total_;
    std::size_t witness_total_;
    std::size_t output_total_;
    Amount input_value_;
    Amount output_value_;
    Amount notification_value_;
    Set<KeyID> change_keys_;
    Set<KeyID> outgoing_keys_;

    static auto is_segwit(const Input& input) noexcept -> bool;

    auto add_signatures(
        const ReadView preimage,
        const blockchain::protocol::bitcoin::base::SigHash& sigHash,
        Input& input) const noexcept -> bool;
    auto add_signatures_p2ms(
        const ReadView preimage,
        const blockchain::protocol::bitcoin::base::SigHash& sigHash,
        const PasswordPrompt& reason,
        const Output& spends,
        Input& input) const noexcept -> bool;
    auto add_signatures_p2pk(
        const ReadView preimage,
        const blockchain::protocol::bitcoin::base::SigHash& sigHash,
        const PasswordPrompt& reason,
        const Output& spends,
        Input& input) const noexcept -> bool;
    auto add_signatures_p2pkh(
        const ReadView preimage,
        const blockchain::protocol::bitcoin::base::SigHash& sigHash,
        const PasswordPrompt& reason,
        const Output& spends,
        Input& input) const noexcept -> bool;
    auto bytes() const noexcept -> std::size_t;
    auto dust() const noexcept -> std::uint64_t;
    auto excess_value() const noexcept -> Amount;
    auto get_private_key(
        const opentxs::crypto::asymmetric::key::EllipticCurve& pubkey,
        const blockchain::crypto::Element& element,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;
    auto has_notification() const noexcept -> bool;
    auto has_output() const noexcept -> bool;
    auto hash_type() const noexcept -> opentxs::crypto::HashType;
    auto init_bip143(Bip143& bip143) const noexcept -> bool;
    auto init_txcopy(Transaction& txcopy) const noexcept -> bool;
    auto is_funded() const noexcept -> bool;
    auto make_notification(
        const crypto::Element& element,
        const PaymentCode& recipient,
        const PasswordPrompt& reason) const noexcept(false)
        -> protocol::bitcoin::base::block::ScriptElements;
    auto make_p2pkh_change(const crypto::Element& element) const noexcept(false)
        -> protocol::bitcoin::base::block::ScriptElements;
    auto make_script(crypto::AddressStyle type, std::string_view bytes) const
        noexcept(false) -> protocol::bitcoin::base::block::ScriptElements;
    auto print() const noexcept -> UnallocatedCString;
    auto required_fee() const noexcept -> Amount;
    auto sign_input(
        const int index,
        Input& input,
        Transaction& txcopy,
        Bip143& bip143) const noexcept -> bool;
    auto sign_input_bch(const int index, Input& input, Bip143& bip143)
        const noexcept -> bool;
    auto sign_input_btc(const int index, Input& input, Transaction& txcopy)
        const noexcept -> bool;
    auto sign_input_segwit(const int index, Input& input, Bip143& bip143)
        const noexcept -> bool;
    auto spender() const noexcept -> const identifier::Nym&;
    auto validate(
        const Match match,
        const blockchain::crypto::Element& element,
        const block::Outpoint& outpoint,
        const Output& output) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;

    auto add_change() noexcept -> bool;
    auto add_change_output(Output output) noexcept(false) -> void;
    auto add_input(const UTXO& utxo) noexcept -> bool;
    auto add_output(
        const identifier::Generic& contact,
        const Amount& value,
        crypto::AddressStyle type,
        std::string_view bytes,
        std::int32_t& index) noexcept(false) -> void;
    auto add_output(
        const identifier::Generic& contact,
        const Amount& value,
        std::string_view pubkey,
        std::int32_t& index) noexcept(false) -> void;
    auto add_output(
        const identifier::Generic& contact,
        const Amount& value,
        protocol::bitcoin::base::block::Script&& script,
        std::int32_t& index) noexcept(false) -> void;
    auto add_sweep_inputs(BuildResult& output, SendResult& rc) noexcept -> bool;
    auto bip_69() noexcept -> void;
    auto create_outputs() noexcept -> bool;
    auto create_notifications(std::uint64_t value) noexcept -> bool;
    auto distribute_change_amounts() noexcept -> void;
    auto drop_unnecessary_change() noexcept -> void;
    auto finalize(SendResult& rc, block::TransactionHash& txid) noexcept
        -> BuildResult;
    auto finalize_outputs() noexcept -> void;
    auto finalize_transaction() noexcept -> Transaction;
    template <typename CB>
    auto make_change_output(
        const crypto::Element& element,
        CB get_output,
        std::optional<TxoTag> tag = std::nullopt) noexcept(false) -> void;
    auto next_change_element(const PasswordPrompt& reason) noexcept(false)
        -> const crypto::Element&;
    auto release_keys() noexcept -> void;
    auto sign_inputs() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::wallet
