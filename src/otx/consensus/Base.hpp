// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Context.pb.h>
#include <atomic>
#include <cstddef>
#include <memory>

#include "core/contract/Signable.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class Signature;
}  // namespace proto

class ByteArray;
class Factory;
class NymFile;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context::implementation
{
class Base : virtual public internal::Base,
             public opentxs::contract::implementation::Signable
{
public:
    auto AcknowledgedNumbers() const -> UnallocatedSet<RequestNumber> final;
    auto AvailableNumbers() const -> std::size_t final;
    auto HaveLocalNymboxHash() const -> bool final;
    auto HaveRemoteNymboxHash() const -> bool final;
    auto IssuedNumbers() const -> UnallocatedSet<TransactionNumber> final;
    auto Name() const noexcept -> UnallocatedCString final;
    auto NymboxHashMatch() const -> bool final;
    auto LegacyDataFolder() const -> UnallocatedCString final;
    auto LocalNymboxHash() const -> identifier::Generic final;
    auto Notary() const -> const identifier::Notary& final
    {
        return server_id_;
    }
    auto Nymfile(const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> final;
    auto RemoteNym() const -> const identity::Nym& final;
    auto RemoteNymboxHash() const -> identifier::Generic final;
    auto Request() const -> RequestNumber final;
    auto Serialize() const noexcept -> ByteArray final;
    auto Serialize(proto::Context& out) const -> bool final;
    auto VerifyAcknowledgedNumber(const RequestNumber& req) const -> bool final;
    auto VerifyAvailableNumber(const TransactionNumber& number) const
        -> bool final;
    auto VerifyIssuedNumber(const TransactionNumber& number) const
        -> bool final;

    auto AddAcknowledgedNumber(const RequestNumber req) -> bool final;
    auto CloseCronItem(const TransactionNumber) -> bool override
    {
        return false;
    }
    auto ConsumeAvailable(const TransactionNumber& number) -> bool final;
    auto ConsumeIssued(const TransactionNumber& number) -> bool final;
    auto IncrementRequest() -> RequestNumber final;
    auto InitializeNymbox(const PasswordPrompt& reason) -> bool final;
    auto mutable_Nymfile(const PasswordPrompt& reason)
        -> Editor<opentxs::NymFile> final;
    auto OpenCronItem(const TransactionNumber) -> bool override
    {
        return false;
    }
    auto RecoverAvailableNumber(const TransactionNumber& number) -> bool final;
    auto Refresh(proto::Context& out, const PasswordPrompt& reason)
        -> bool final;
    auto RemoveAcknowledgedNumber(const UnallocatedSet<RequestNumber>& req)
        -> bool final;
    auto Reset() -> void final;
    auto SetLocalNymboxHash(const identifier::Generic& hash) -> void final;
    auto SetRemoteNymboxHash(const identifier::Generic& hash) -> void final;
    auto SetRequest(const RequestNumber req) -> void final;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    ~Base() override = default;

protected:
    const identifier::Notary server_id_;
    Nym_p remote_nym_;
    UnallocatedSet<TransactionNumber> available_transaction_numbers_;
    UnallocatedSet<TransactionNumber> issued_transaction_numbers_;
    std::atomic<RequestNumber> request_number_;
    UnallocatedSet<RequestNumber> acknowledged_request_numbers_;
    identifier::Generic local_nymbox_hash_;
    identifier::Generic remote_nymbox_hash_;

    auto contract(const Lock& lock) const -> proto::Context;
    auto GetID(const Lock& lock) const -> identifier::Generic final;
    auto serialize(const Lock& lock, const otx::ConsensusType type) const
        -> proto::Context;
    virtual auto serialize(const Lock& lock) const -> proto::Context = 0;
    virtual auto type() const -> UnallocatedCString = 0;
    auto validate(const Lock& lock) const -> bool final;

    auto add_acknowledged_number(const Lock& lock, const RequestNumber req)
        -> bool;
    auto consume_available(const Lock& lock, const TransactionNumber& number)
        -> bool;
    auto consume_issued(const Lock& lock, const TransactionNumber& number)
        -> bool;
    auto finish_acknowledgements(
        const Lock& lock,
        const UnallocatedSet<RequestNumber>& req) -> void;
    auto issue_number(const Lock& lock, const TransactionNumber& number)
        -> bool;
    auto recover_available_number(
        const Lock& lock,
        const TransactionNumber& number) -> bool;
    auto remove_acknowledged_number(
        const Lock& lock,
        const UnallocatedSet<RequestNumber>& req) -> bool;
    auto save(const Lock& lock, const PasswordPrompt& reason) -> bool;
    auto set_local_nymbox_hash(
        const Lock& lock,
        const identifier::Generic& hash) -> void;
    auto set_remote_nymbox_hash(
        const Lock& lock,
        const identifier::Generic& hash) -> void;
    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;
    auto verify_available_number(const Lock& lock, const TransactionNumber& req)
        const -> bool;
    auto verify_acknowledged_number(const Lock& lock, const RequestNumber& req)
        const -> bool;
    auto verify_issued_number(const Lock& lock, const TransactionNumber& number)
        const -> bool;

    Base(
        const api::Session& api,
        const VersionNumber targetVersion,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Notary& server);
    Base(
        const api::Session& api,
        const VersionNumber targetVersion,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Notary& server);

private:
    friend opentxs::Factory;

    const VersionNumber target_version_;

    static auto calculate_id(
        const api::Session& api,
        const Nym_p& client,
        const Nym_p& server) noexcept(false) -> identifier::Generic;

    virtual auto client_nym_id(const Lock& lock) const
        -> const identifier::Nym& = 0;
    auto clone() const noexcept -> Base* final { return nullptr; }
    auto IDVersion(const Lock& lock) const -> proto::Context;
    virtual auto server_nym_id(const Lock& lock) const
        -> const identifier::Nym& = 0;
    auto SigVersion(const Lock& lock) const -> proto::Context;
    auto verify_signature(const Lock& lock, const proto::Signature& signature)
        const -> bool final;

    // Transition method used for converting from Nym class
    auto insert_available_number(const TransactionNumber& number) -> bool;
    // Transition method used for converting from Nym class
    auto insert_issued_number(const TransactionNumber& number) -> bool;
};
}  // namespace opentxs::otx::context::implementation
