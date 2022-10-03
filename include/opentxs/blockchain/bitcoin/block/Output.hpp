// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Output;
}  // namespace internal

class Script;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Output
{
public:
    using ContactID = identifier::Generic;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Output& = 0;
    virtual auto Note(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString = 0;
    virtual auto Keys() const noexcept -> UnallocatedVector<crypto::Key> = 0;
    virtual auto Payee() const noexcept -> ContactID = 0;
    virtual auto Payer() const noexcept -> ContactID = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    virtual auto Script() const noexcept -> const block::Script& = 0;
    virtual auto Value() const noexcept -> blockchain::Amount = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Output& = 0;

    Output(const Output&) = delete;
    Output(Output&&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;

    virtual ~Output() = default;

protected:
    Output() noexcept = default;
};
}  // namespace opentxs::blockchain::bitcoin::block
