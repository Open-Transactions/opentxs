// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Notary;
}  // namespace identifier

namespace otx
{
namespace context
{
namespace internal
{
class Base;
}  // namespace internal
}  // namespace context
}  // namespace otx

namespace protobuf
{
class Context;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context
{
class Base : virtual public opentxs::contract::Signable<identifier::Generic>
{
public:
    using TransactionNumbers = UnallocatedSet<TransactionNumber>;
    using RequestNumbers = UnallocatedSet<RequestNumber>;

    virtual auto AcknowledgedNumbers() const -> RequestNumbers = 0;
    virtual auto AvailableNumbers() const -> std::size_t = 0;
    virtual auto HaveLocalNymboxHash() const -> bool = 0;
    virtual auto HaveRemoteNymboxHash() const -> bool = 0;
    virtual auto IssuedNumbers() const -> TransactionNumbers = 0;
    virtual auto Internal() const noexcept -> const internal::Base& = 0;
    virtual auto LegacyDataFolder() const -> UnallocatedCString = 0;
    virtual auto LocalNymboxHash() const -> identifier::Generic = 0;
    virtual auto Notary() const -> const identifier::Notary& = 0;
    virtual auto NymboxHashMatch() const -> bool = 0;
    virtual auto RemoteNym() const -> const identity::Nym& = 0;
    virtual auto RemoteNymboxHash() const -> identifier::Generic = 0;
    virtual auto Request() const -> RequestNumber = 0;
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
    virtual auto Internal() noexcept -> internal::Base& = 0;
    virtual auto OpenCronItem(const TransactionNumber) -> bool = 0;
    virtual auto RecoverAvailableNumber(const TransactionNumber& number)
        -> bool = 0;
    virtual auto RemoveAcknowledgedNumber(const RequestNumbers& req)
        -> bool = 0;
    virtual void Reset() = 0;
    virtual auto Refresh(protobuf::Context& out, const PasswordPrompt& reason)
        -> bool = 0;
    virtual void SetLocalNymboxHash(const identifier::Generic& hash) = 0;
    virtual void SetRemoteNymboxHash(const identifier::Generic& hash) = 0;
    virtual void SetRequest(const RequestNumber req) = 0;

    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    ~Base() override = default;

protected:
    Base() = default;

private:
#ifdef _WIN32
public:
#endif
    virtual auto clone() const noexcept -> Base* { return nullptr; }
};
}  // namespace opentxs::otx::context
