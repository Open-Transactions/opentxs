// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
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
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class OPENTXS_EXPORT Output
{
public:
    using ContactID = OTIdentifier;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Output& = 0;
    virtual auto Note() const noexcept -> std::string = 0;
    virtual auto Keys() const noexcept -> std::pmr::vector<crypto::Key> = 0;
    virtual auto Payee() const noexcept -> ContactID = 0;
    virtual auto Payer() const noexcept -> ContactID = 0;
    virtual auto Print() const noexcept -> std::string = 0;
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
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
