// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Mode.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"
#include "serialization/protobuf/Enums.pb.h"

namespace opentxs
{
auto translate(const crypto::key::asymmetric::Algorithm in) noexcept
    -> proto::AsymmetricKeyType;
auto translate(const crypto::key::asymmetric::Mode in) noexcept
    -> proto::KeyMode;
auto translate(const crypto::key::asymmetric::Role in) noexcept
    -> proto::KeyRole;
auto translate(const crypto::key::symmetric::Source in) noexcept
    -> proto::SymmetricKeyType;
auto translate(const crypto::key::symmetric::Algorithm in) noexcept
    -> proto::SymmetricMode;
auto translate(const proto::KeyMode in) noexcept
    -> crypto::key::asymmetric::Mode;
auto translate(const proto::KeyRole in) noexcept
    -> crypto::key::asymmetric::Role;
auto translate(const proto::AsymmetricKeyType in) noexcept
    -> crypto::key::asymmetric::Algorithm;
auto translate(const proto::SymmetricKeyType in) noexcept
    -> crypto::key::symmetric::Source;
auto translate(const proto::SymmetricMode in) noexcept
    -> crypto::key::symmetric::Algorithm;
}  // namespace opentxs
