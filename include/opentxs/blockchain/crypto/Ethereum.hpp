// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Ethereum : virtual public Imported
{
public:
    virtual auto GetBalance() const noexcept -> blockchain::Amount = 0;
    virtual auto GetNonce() const noexcept -> blockchain::Amount = 0;
    virtual auto IncrementNonce() const noexcept -> blockchain::Amount = 0;
    virtual auto SetBalance(const Amount balance) const noexcept -> void = 0;
    virtual auto SetNonce(const blockchain::Amount nonce) const noexcept
        -> void = 0;

    Ethereum(const Ethereum&) = delete;
    Ethereum(Ethereum&&) = delete;
    auto operator=(const Ethereum&) -> Ethereum& = delete;
    auto operator=(Ethereum&&) -> Ethereum& = delete;

    OPENTXS_NO_EXPORT ~Ethereum() override = default;

protected:
    Ethereum() noexcept = default;
};
}  // namespace opentxs::blockchain::crypto
