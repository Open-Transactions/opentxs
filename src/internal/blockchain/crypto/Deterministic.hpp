// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"

namespace opentxs::blockchain::crypto::internal
{
struct Deterministic : virtual public crypto::Deterministic,
                       virtual public Subaccount {
};
}  // namespace opentxs::blockchain::crypto::internal
