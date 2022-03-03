// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/key/Asymmetric.hpp"

namespace opentxs::crypto::key
{
class OPENTXS_EXPORT RSA : virtual public Asymmetric
{
public:
    ~RSA() override = default;

protected:
    RSA() = default;

private:
    RSA(const RSA&) = delete;
    RSA(RSA&&) = delete;
    auto operator=(const RSA&) -> RSA& = delete;
    auto operator=(RSA&&) -> RSA& = delete;
};
}  // namespace opentxs::crypto::key
