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

auto Bip32::Imp::derive_private_key(
    const asymmetric::Algorithm,
    const proto::HDPath&,
    const ReadView,
    const ReadView,
    const ReadView,
    const Path&,
    const PasswordPrompt&) const noexcept(false) -> Key
{
    return blank_.get();
}

auto Bip32::Imp::derive_public_key(
    const asymmetric::Algorithm,
    const proto::HDPath&,
    const ReadView,
    const ReadView,
    const Path&,
    const PasswordPrompt&) const noexcept(false) -> Key
{
    return blank_.get();
}

auto Bip32::Imp::root_node(
    const EcdsaCurve&,
    const ReadView,
    Writer&&,
    Writer&&,
    Writer&&) const noexcept -> bool
{
    return false;
}
}  // namespace opentxs::crypto
