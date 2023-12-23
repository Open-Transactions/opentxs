// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/contract/Types.internal.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Context;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context
{
class ConsensusPrivate
{
public:
    UnallocatedSet<TransactionNumber> available_transaction_numbers_;
    UnallocatedSet<TransactionNumber> issued_transaction_numbers_;
    RequestNumber request_number_;
    UnallocatedSet<RequestNumber> acknowledged_request_numbers_;
    identifier::Generic local_nymbox_hash_;
    identifier::Generic remote_nymbox_hash_;
    VersionNumber current_version_;
    contract::Signature sig_;

    ConsensusPrivate() = delete;
    ConsensusPrivate(const ConsensusPrivate&) = delete;
    ConsensusPrivate(ConsensusPrivate&&) = delete;
    auto operator=(const ConsensusPrivate&) -> ConsensusPrivate& = delete;
    auto operator=(ConsensusPrivate&&) -> ConsensusPrivate& = delete;

    virtual ~ConsensusPrivate() = default;

protected:
    ConsensusPrivate(const VersionNumber targetVersion) noexcept;
    ConsensusPrivate(
        const api::Session& api,
        const VersionNumber targetVersion,
        const proto::Context& serialized) noexcept;
};
}  // namespace opentxs::otx::context
