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
    using Amount = opentxs::blockchain::Amount;
    using Nonce = Amount;

    virtual Amount GetBalance() const noexcept = 0;
    virtual Nonce GetNonce() const noexcept = 0;
    virtual Nonce IncrementNonce() const noexcept = 0;
    virtual void SetBalance(const Amount balance) const noexcept = 0;
    virtual void SetNonce(const Nonce nonce) const noexcept = 0;

    OPENTXS_NO_EXPORT ~Ethereum() override = default;

protected:
    Ethereum() noexcept = default;

private:
    Ethereum(const Ethereum&) = delete;
    Ethereum(Ethereum&&) = delete;
    Ethereum& operator=(const Ethereum&) = delete;
    Ethereum& operator=(Ethereum&&) = delete;
};
}  // namespace opentxs::blockchain::crypto
