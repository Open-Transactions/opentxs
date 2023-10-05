// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainTransaction.pb.h>  // IWYU pragma: keep
#include <HDPath.pb.h>                 // IWYU pragma: keep
#include <boost/container/flat_set.hpp>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include "blockchain/node/spend/SpendPrivate.hpp"
#include "internal/blockchain/node/SpendPolicy.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace crypto
{
class PaymentCode;
}  // namespace crypto
}  // namespace blockchain

namespace identity
{
class Nym;
}  // namespace identity

namespace proto
{
class BlockchainTransactionProposal;
class BlockchainTransactionProposedNotification;
class BlockchainTransactionProposedOutput;
class BlockchainTransactionProposedSweep;
}  // namespace proto

class ByteArray;
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class SpendPrivate final : public node::SpendPrivate
{
public:
    auto AddNotification(const block::TransactionHash& txid) const noexcept
        -> void final;
    auto AddressRecipients() const noexcept
        -> std::span<const AddressRecipient> final;
    auto Funding() const noexcept -> node::Funding final;
    auto ID() const noexcept -> const identifier::Generic& final;
    auto IsExpired() const noexcept -> bool final;
    auto Memo() const noexcept -> std::string_view final;
    auto Notifications() const noexcept -> std::span<const PaymentCode> final;
    auto OutgoingKeys() const noexcept -> Set<crypto::Key> final;
    auto PasswordPrompt() const noexcept -> std::string_view final;
    auto PaymentCodeRecipients() const noexcept
        -> std::span<const PaymentCodeRecipient> final;
    auto Policy() const noexcept -> internal::SpendPolicy final;
    auto SpendUnconfirmedChange() const noexcept -> bool final;
    auto SpendUnconfirmedIncoming() const noexcept -> bool final;
    auto Spender() const noexcept -> const identifier::Nym& final;
    [[nodiscard]] auto Serialize(
        proto::BlockchainTransactionProposal& out) const noexcept -> bool final;
    auto SweepFromAccount() const noexcept -> bool final;
    auto SweepFromKey() const noexcept -> const crypto::Key& final;
    auto SweepFromSubaccount() const noexcept
        -> const identifier::Account& final;
    auto SweepToAddress() const noexcept -> std::string_view final;
    auto UseEnhancedNotifications() const noexcept -> bool final;

    auto Add(const proto::BlockchainTransaction& tx) noexcept -> void final;
    [[nodiscard]] auto Check() noexcept -> std::optional<SendResult> final;
    auto Finalize(const Log& log, alloc::Strategy alloc) noexcept(false)
        -> void final;
    [[nodiscard]] auto Notify(const PaymentCode& recipient) noexcept
        -> bool final;
    [[nodiscard]] auto Notify(std::span<const PaymentCode> recipients) noexcept
        -> bool final;
    [[nodiscard]] auto SendToAddress(
        std::string_view address,
        const Amount& amount) noexcept -> bool final;
    [[nodiscard]] auto SendToPaymentCode(
        const PaymentCode& recipient,
        const Amount& amount) noexcept -> bool final;
    [[nodiscard]] auto SetMemo(std::string_view) noexcept -> bool final;
    [[nodiscard]] auto SetSpendUnconfirmedChange(bool value) noexcept
        -> bool final;
    [[nodiscard]] auto SetSpendUnconfirmedIncoming(bool value) noexcept
        -> bool final;
    [[nodiscard]] auto SetSweepFromAccount(bool value) noexcept -> bool final;
    [[nodiscard]] auto SetSweepFromKey(const crypto::Key& key) noexcept
        -> bool final;
    [[nodiscard]] auto SetSweepFromSubaccount(
        const identifier::Account& aubaccount) noexcept -> bool final;
    [[nodiscard]] auto SetSweepToAddress(std::string_view address) noexcept
        -> bool final;
    [[nodiscard]] auto SetUseEnhancedNotifications(bool value) noexcept
        -> bool final;

    SpendPrivate(
        const api::session::Client& api,
        blockchain::Type chain,
        const identifier::Nym& spender,
        opentxs::PasswordPrompt reason) noexcept;
    SpendPrivate(
        const api::session::Client& api,
        blockchain::Type chain,
        const proto::BlockchainTransactionProposal& proto) noexcept(false);
    SpendPrivate() = delete;
    SpendPrivate(const SpendPrivate&) = delete;
    SpendPrivate(SpendPrivate&&) = delete;
    auto operator=(const SpendPrivate&) -> SpendPrivate& = delete;
    auto operator=(SpendPrivate&&) -> SpendPrivate& = delete;

    ~SpendPrivate() final;

private:
    static constexpr auto proposal_version_ = VersionNumber{1};
    static constexpr auto notification_version_ = VersionNumber{1};
    static constexpr auto output_version_ = VersionNumber{1};
    static constexpr auto sweep_version_ = VersionNumber{1};
    static constexpr auto sweep_key_version_ = VersionNumber{1};
    static constexpr auto max_outputs_ = 1000_uz;

    const api::session::Client& api_;
    const VersionNumber version_;
    const blockchain::Type chain_;
    const identifier::Generic id_;
    const identifier::Nym spender_;
    const Nym_p nym_;
    const Time expires_;
    const opentxs::PasswordPrompt reason_;
    CString memo_;
    Vector<AddressRecipient> address_recipients_;
    Vector<PaymentCodeRecipient> pc_recipients_;
    boost::container::flat_set<PaymentCode> notifications_;
    node::Funding policy_;
    bool spend_unconfirmed_change_;
    bool spend_unconfirmed_incoming_;
    bool use_enhanced_notifications_;
    mutable std::optional<proto::HDPath> payment_code_path_;
    mutable std::optional<PaymentCode> sender_payment_code_;
    std::optional<std::pair<ByteArray, crypto::AddressStyle>> sweep_to_address_;
    std::optional<identifier::Account> sweep_subaccount_;
    std::optional<crypto::Key> sweep_key_;
    std::optional<proto::BlockchainTransaction> transaction_;

    auto account(const PaymentCode& recipient) const noexcept(false)
        -> const crypto::PaymentCode&;
    auto check_funding_default() const noexcept -> std::optional<SendResult>;
    auto check_send_to_self(const PaymentCode& pc) const noexcept(false)
        -> void;
    auto check_sender() const noexcept -> bool;
    auto is_sweep() const noexcept -> bool;
    auto nym() const noexcept(false) -> const identity::Nym&;
    auto path() const noexcept(false) -> const proto::HDPath&;
    auto sender_payment_code() const noexcept(false) -> const PaymentCode&;
    auto serialize_notification(
        const PaymentCode& sender,
        const proto::HDPath& path,
        const PaymentCode& in,
        proto::BlockchainTransactionProposedNotification& out) const
        noexcept(false) -> void;
    auto serialize_address(
        std::string_view address,
        crypto::AddressStyle type,
        proto::BlockchainTransactionProposedOutput& out) const noexcept -> void;
    auto serialize_amount(
        const Amount& amount,
        proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
        -> void;
    auto serialize_output(
        const AddressRecipient& in,
        proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
        -> void;
    auto serialize_output(
        const PaymentCodeRecipient& in,
        proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
        -> void;
    auto serialize_payment_code(
        const PaymentCodeRecipient& in,
        proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
        -> void;
    auto serialize_sweep(proto::BlockchainTransactionProposedSweep& out) const
        noexcept(false) -> void;
    auto validate_address(std::string_view address) const noexcept(false)
        -> std::pair<crypto::AddressStyle, ByteArray>;
    auto validate_amount(const Amount& amount) const noexcept(false) -> void;
    auto validate_payment_code(const PaymentCode& pc) const noexcept(false)
        -> void;

    auto add_address(
        std::string_view address,
        crypto::AddressStyle type,
        const Amount& amount) noexcept(false) -> void;
    auto add_payment_code(
        const PaymentCode& recipient,
        const Amount& amount,
        const identifier::Account& subaccount,
        Bip32Index index,
        ReadView pubkey) noexcept(false) -> void;
    auto check_funding_sweep() noexcept -> std::optional<SendResult>;
    auto deserialize_notification(
        const proto::BlockchainTransactionProposedNotification&
            in) noexcept(false) -> void;
    auto deserialize_output(
        const proto::BlockchainTransactionProposedOutput& in) noexcept(false)
        -> void;
    auto deserialize_sweep(
        const proto::BlockchainTransactionProposedSweep& in) noexcept(false)
        -> void;

    SpendPrivate(
        const api::session::Client& api,
        VersionNumber version,
        blockchain::Type chain,
        identifier::Generic id,
        identifier::Nym spender,
        Time expires,
        opentxs::PasswordPrompt reason) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
