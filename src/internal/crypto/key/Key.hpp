// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::AsymmetricKeyType

#pragma once

#include <Enums.pb.h>

#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"

namespace opentxs
{
auto translate(const crypto::asymmetric::Algorithm in) noexcept
    -> proto::AsymmetricKeyType;
auto translate(const crypto::asymmetric::Mode in) noexcept -> proto::KeyMode;
auto translate(const crypto::asymmetric::Role in) noexcept -> proto::KeyRole;
auto translate(const crypto::symmetric::Source in) noexcept
    -> proto::SymmetricKeyType;
auto translate(const crypto::symmetric::Algorithm in) noexcept
    -> proto::SymmetricMode;
auto translate(const proto::KeyMode in) noexcept -> crypto::asymmetric::Mode;
auto translate(const proto::KeyRole in) noexcept -> crypto::asymmetric::Role;
auto translate(const proto::AsymmetricKeyType in) noexcept
    -> crypto::asymmetric::Algorithm;
auto translate(const proto::SymmetricKeyType in) noexcept
    -> crypto::symmetric::Source;
auto translate(const proto::SymmetricMode in) noexcept
    -> crypto::symmetric::Algorithm;
}  // namespace opentxs
