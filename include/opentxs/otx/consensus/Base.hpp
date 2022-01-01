// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>

#include "opentxs/Types.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"

namespace opentxs
{
namespace identifier
{
class Server;
}  // namespace identifier

namespace proto
{
class Context;
}  // namespace proto

class NymFile;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
namespace context
{
class OPENTXS_EXPORT Base : virtual public opentxs::contract::Signable
{
public:
    using TransactionNumbers = std::pmr::set<TransactionNumber>;
    using RequestNumbers = std::pmr::set<RequestNumber>;

    virtual auto AcknowledgedNumbers() const -> RequestNumbers = 0;
    virtual auto AvailableNumbers() const -> std::size_t = 0;
    virtual auto HaveLocalNymboxHash() const -> bool = 0;
    virtual auto HaveRemoteNymboxHash() const -> bool = 0;
    virtual auto IssuedNumbers() const -> TransactionNumbers = 0;
    virtual auto LegacyDataFolder() const -> std::string = 0;
    virtual auto LocalNymboxHash() const -> OTIdentifier = 0;
    virtual auto Notary() const -> const identifier::Server& = 0;
    virtual auto NymboxHashMatch() const -> bool = 0;
    virtual auto Nymfile(const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> = 0;
    virtual auto RemoteNym() const -> const identity::Nym& = 0;
    virtual auto RemoteNymboxHash() const -> OTIdentifier = 0;
    virtual auto Request() const -> RequestNumber = 0;
    using Signable::Serialize;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::Context& out) const
        -> bool = 0;
    virtual auto Type() const -> otx::ConsensusType = 0;
    virtual auto VerifyAcknowledgedNumber(const RequestNumber& req) const
        -> bool = 0;
    virtual auto VerifyAvailableNumber(const TransactionNumber& number) const
        -> bool = 0;
    virtual auto VerifyIssuedNumber(const TransactionNumber& number) const
        -> bool = 0;

    virtual auto AddAcknowledgedNumber(const RequestNumber req) -> bool = 0;
    virtual auto CloseCronItem(const TransactionNumber) -> bool = 0;
    virtual auto ConsumeAvailable(const TransactionNumber& number) -> bool = 0;
    virtual auto ConsumeIssued(const TransactionNumber& number) -> bool = 0;
    virtual auto IncrementRequest() -> RequestNumber = 0;
    virtual auto InitializeNymbox(const PasswordPrompt& reason) -> bool = 0;
    virtual auto mutable_Nymfile(const PasswordPrompt& reason)
        -> Editor<opentxs::NymFile> = 0;
    virtual auto OpenCronItem(const TransactionNumber) -> bool = 0;
    virtual auto RecoverAvailableNumber(const TransactionNumber& number)
        -> bool = 0;
    virtual auto RemoveAcknowledgedNumber(const RequestNumbers& req)
        -> bool = 0;
    virtual void Reset() = 0;
    OPENTXS_NO_EXPORT virtual auto Refresh(
        proto::Context& out,
        const PasswordPrompt& reason) -> bool = 0;
    virtual void SetLocalNymboxHash(const Identifier& hash) = 0;
    virtual void SetRemoteNymboxHash(const Identifier& hash) = 0;
    virtual void SetRequest(const RequestNumber req) = 0;

    ~Base() override = default;

protected:
    Base() = default;

private:
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
