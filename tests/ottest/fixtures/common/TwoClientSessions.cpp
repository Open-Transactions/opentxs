// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/common/TwoClientSessions.hpp"  // IWYU pragma: associated

namespace ottest
{
TwoClientSessions::TwoClientSessions() noexcept
    : client_2_(StartClient(1))
{
}
}  // namespace ottest
