// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/contract/Peer.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <span>

#include "internal/blockchain/Params.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
User PeerRequest::alex_s_{GetPaymentCodeVector3().alice_.words_, "Alex"};
User PeerRequest::bob_s_{GetPaymentCodeVector3().bob_.words_, "Bob"};
std::atomic<bool> PeerRequest::init_{false};
}  // namespace ottest

namespace ottest
{
PeerRequest::PeerRequest() noexcept
    : reason_(client_1_.Factory().PasswordPrompt(""))
    , alex_([this]() -> const auto& {
        if (false == init_.exchange(true)) {
            alex_s_.init(client_1_);
            bob_s_.init(client_1_);
        }

        return alex_s_;
    }())
    , bob_(bob_s_)
    , notary_(client_1_.Factory().NotaryIDFromRandom())
    , unit_(client_1_.Factory().UnitIDFromRandom())
    , amount_(42)
    , txid_(
          opentxs::IsHex,
          "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"sv)
    , tx_(opentxs::blockchain::params::get(opentxs::blockchain::Type::Bitcoin)
              .GenesisBlock(client_1_.Crypto())
              .get()[0])
{
}
}  // namespace ottest
