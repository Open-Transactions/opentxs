// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::AsymmetricKeyType

#pragma once

#include <opentxs/protobuf/Enums.pb.h>

#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"

namespace opentxs
{
auto translate(const crypto::asymmetric::Algorithm in) noexcept
    -> protobuf::AsymmetricKeyType;
auto translate(const crypto::asymmetric::Mode in) noexcept -> protobuf::KeyMode;
auto translate(const crypto::asymmetric::Role in) noexcept -> protobuf::KeyRole;
auto translate(const crypto::symmetric::Source in) noexcept
    -> protobuf::SymmetricKeyType;
auto translate(const crypto::symmetric::Algorithm in) noexcept
    -> protobuf::SymmetricMode;
auto translate(const protobuf::KeyMode in) noexcept -> crypto::asymmetric::Mode;
auto translate(const protobuf::KeyRole in) noexcept -> crypto::asymmetric::Role;
auto translate(const protobuf::AsymmetricKeyType in) noexcept
    -> crypto::asymmetric::Algorithm;
auto translate(const protobuf::SymmetricKeyType in) noexcept
    -> crypto::symmetric::Source;
auto translate(const protobuf::SymmetricMode in) noexcept
    -> crypto::symmetric::Algorithm;
}  // namespace opentxs
