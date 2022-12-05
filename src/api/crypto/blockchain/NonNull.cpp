// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Blockchain.hpp"  // IWYU pragma: associated

#include "api/crypto/blockchain/Imp_blockchain.hpp"

namespace opentxs::api::crypto::imp
{
Blockchain::Blockchain(
    const api::session::Client& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contacts,
    const api::Legacy& legacy,
    const UnallocatedCString& dataFolder,
    const Options& args) noexcept
    : imp_(std::make_unique<BlockchainImp>(
          api,
          activity,
          contacts,
          legacy,
          dataFolder,
          args,
          *this))
{
    // WARNING: do not access api_.Wallet() during construction
}
}  // namespace opentxs::api::crypto::imp
