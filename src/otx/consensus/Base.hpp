// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Context.pb.h>
#include <cs_shared_guarded.h>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <span>

#include "core/contract/Signable.hpp"
#include "internal/core/contract/Types.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/ClientPrivate.hpp"
#include "otx/consensus/ServerPrivate.hpp"

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

class Factory;
class NymFile;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context::implementation
{
template <typename CRTP, typename DataType>
class Base
    : virtual public internal::Base,
      public opentxs::contract::implementation::Signable<identifier::Generic>
{
public:
    auto AcknowledgedNumbers() const -> UnallocatedSet<RequestNumber> final;
    auto AvailableNumbers() const -> std::size_t final;
    auto HaveLocalNymboxHash() const -> bool final;
    auto HaveRemoteNymboxHash() const -> bool final;
    auto IssuedNumbers() const -> UnallocatedSet<TransactionNumber> final;
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
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Validate() const noexcept -> bool final;
    auto VerifyAcknowledgedNumber(const RequestNumber& req) const -> bool final;
    auto VerifyAvailableNumber(const TransactionNumber& number) const
        -> bool final;
    auto VerifyIssuedNumber(const TransactionNumber& number) const
        -> bool final;
    auto Version() const noexcept -> VersionNumber final;

    auto AddAcknowledgedNumber(const RequestNumber req) -> bool final;
    auto ConsumeAvailable(const TransactionNumber& number) -> bool final;
    auto ConsumeIssued(const TransactionNumber& number) -> bool final;
    auto IncrementRequest() -> RequestNumber final;
    auto InitializeNymbox(const PasswordPrompt& reason) -> bool final;
    auto mutable_Nymfile(const PasswordPrompt& reason)
        -> Editor<opentxs::NymFile> final;
    auto RecoverAvailableNumber(const TransactionNumber& number) -> bool final;
    auto Refresh(proto::Context& out, const PasswordPrompt& reason)
        -> bool final;
    auto RemoveAcknowledgedNumber(const UnallocatedSet<RequestNumber>& req)
        -> bool final;
    auto Reset() -> void final;
    auto Save(const PasswordPrompt& reason) noexcept -> bool final;
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
    using Data = DataType;
    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    const identifier::Notary server_id_;
    const Nym_p remote_nym_;
    const VersionNumber target_version_;

    auto calculate_id() const -> identifier_type final;
    auto contract(const Data& data) const -> proto::Context;
    auto get_data() const noexcept
    {
        return static_cast<const CRTP*>(this)->data_.lock_shared();
    }
    auto have_local_nymbox_hash(const Data& data) const -> bool;
    auto have_remote_nymbox_hash(const Data& data) const -> bool;
    using Signable::serialize;
    auto serialize(const Data& data, const otx::ConsensusType type) const
        -> proto::Context;
    auto serialize(const Data& data, proto::Context& out) const -> bool;
    virtual auto serialize(const Data& data) const -> proto::Context = 0;
    using Signable::signatures;
    auto signatures() const noexcept
        -> std::span<const contract::Signature> final;
    auto signatures(const Data& data) const noexcept
        -> std::span<const contract::Signature>;
    virtual auto type() const -> UnallocatedCString = 0;
    using Signable::validate;
    auto validate() const -> bool final;
    auto validate(const Data& data) const -> bool;
    auto verify_available_number(const Data& data, const TransactionNumber& req)
        const -> bool;
    auto verify_acknowledged_number(const Data& data, const RequestNumber& req)
        const -> bool;
    auto verify_issued_number(const Data& data, const TransactionNumber& number)
        const -> bool;
    auto version(const Data& data) const noexcept -> VersionNumber;

    auto add_acknowledged_number(Data& data, const RequestNumber req) -> bool;
    auto consume_available(Data& data, const TransactionNumber& number) -> bool;
    auto consume_issued(Data& data, const TransactionNumber& number) -> bool;
    auto finish_acknowledgements(
        Data& data,
        const UnallocatedSet<RequestNumber>& req) -> void;
    auto get_data() noexcept
    {
        auto handle = static_cast<CRTP*>(this)->data_.lock();
        clear_signatures(*handle);

        return handle;
    }
    auto issue_number(Data& data, const TransactionNumber& number) -> bool;
    auto recover_available_number(Data& data, const TransactionNumber& number)
        -> bool;
    auto remove_acknowledged_number(
        Data& data,
        const UnallocatedSet<RequestNumber>& req) -> bool;
    auto reset(Data& data) -> void;
    auto save(Data& data, const PasswordPrompt& reason) -> bool;
    auto set_local_nymbox_hash(Data& data, const identifier::Generic& hash)
        -> void;
    auto set_remote_nymbox_hash(Data& data, const identifier::Generic& hash)
        -> void;
    using Signable::update_signature;
    auto update_signature(const PasswordPrompt& reason) -> bool final;
    auto update_signature(Data& data, const PasswordPrompt& reason) -> bool;
    auto update_version(Data& data, const VersionNumber version) noexcept
        -> void;

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

    static auto calculate_id(
        const api::Session& api,
        const Nym_p& client,
        const Nym_p& server) noexcept(false) -> identifier_type;

    virtual auto client_nym_id() const -> const identifier::Nym& = 0;
    virtual auto server_nym_id() const -> const identifier::Nym& = 0;
    auto sig_version(const Data& data) const -> proto::Context;
    using Signable::verify_signature;
    auto verify_signature(const proto::Signature& signature) const
        -> bool final;
    auto verify_signature(const Data& data, const proto::Signature& signature)
        const -> bool;

    auto clear_signatures(Data& data) noexcept -> void;
    auto insert_available_number(Data& data, const TransactionNumber& number)
        -> bool;
    auto insert_issued_number(Data& data, const TransactionNumber& number)
        -> bool;
};
}  // namespace opentxs::otx::context::implementation
