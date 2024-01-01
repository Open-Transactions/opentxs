// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/crypto/Wallet.hpp"

#include "opentxs/blockchain/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Subaccount;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Account;
}  // namespace identifier

namespace protobuf
{
class HDPath;
}  // namespace protobuf

class PaymentCode;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Wallet : virtual public crypto::Wallet {
    virtual auto AddEthereum(
        const identifier::Nym& nym,
        const protobuf::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& = 0;
    virtual auto AddHD(
        const identifier::Nym& nym,
        const protobuf::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& = 0;
    virtual auto AddPaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const protobuf::HDPath& path,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& = 0;

    ~Wallet() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
