// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "crypto/bip32/Imp.hpp"  // IWYU pragma: associated

namespace opentxs::crypto
{
auto Bip32::Imp::DeriveKey(const EcdsaCurve&, const Secret&, const Path&) const
    -> Key
{
    return blank_.get();
}

auto Bip32::Imp::DerivePrivateKey(
    const key::HD&,
    const Path&,
    const PasswordPrompt&) const noexcept(false) -> Key
{
    return blank_.get();
}

auto Bip32::Imp::DerivePublicKey(
    const key::HD&,
    const Path&,
    const PasswordPrompt&) const noexcept(false) -> Key
{
    return blank_.get();
}

auto Bip32::Imp::root_node(
    const EcdsaCurve&,
    const ReadView,
    const AllocateOutput,
    const AllocateOutput,
    const AllocateOutput) const noexcept -> bool
{
    return false;
}
}  // namespace opentxs::crypto
