// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Output;
}  // namespace internal

class Script;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::bitcoin
{
class OPENTXS_EXPORT Output
{
public:
    using ContactID = OTIdentifier;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Output& = 0;
    virtual auto Note() const noexcept -> UnallocatedCString = 0;
    virtual auto Keys() const noexcept -> UnallocatedVector<crypto::Key> = 0;
    virtual auto Payee() const noexcept -> ContactID = 0;
    virtual auto Payer() const noexcept -> ContactID = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    virtual auto Script() const noexcept -> const bitcoin::Script& = 0;
    virtual auto Value() const noexcept -> blockchain::Amount = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Output& = 0;

    virtual ~Output() = default;

protected:
    Output() noexcept = default;

private:
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin
