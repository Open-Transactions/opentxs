// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/hasher/HasherPrivate.hpp"  // IWYU pragma: associated

#include "internal/util/P0330.hpp"

namespace opentxs::crypto
{
HasherPrivate::HasherPrivate(const void*, const void*) noexcept
    : size_1_(0_uz)
    , size_2_(0_uz)
    , context_(nullptr)
    , stage_2_(nullptr)
{
}

HasherPrivate::HasherPrivate() noexcept
    : HasherPrivate(nullptr, nullptr)
{
}

auto HasherPrivate::operator()(ReadView) noexcept -> bool { return false; }

auto HasherPrivate::operator()(Writer&&) noexcept -> bool { return false; }

HasherPrivate::~HasherPrivate() = default;
}  // namespace opentxs::crypto
