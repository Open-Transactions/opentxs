// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/LowLevel.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Armored : public LowLevel
{
protected:
    auto Check(const ot::Data& data) noexcept(false) -> bool;

    Armored() noexcept;

    ~Armored() override = default;
};
}  // namespace ottest
