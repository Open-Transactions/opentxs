// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
class KeyPrivate;
}  // namespace asymmetric
}  // namespace crypto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key
{
class OPENTXS_EXPORT HD : public EllipticCurve
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> HD&;

    auto Chaincode(const PasswordPrompt& reason) const noexcept -> ReadView;
    auto ChildKey(
        const Bip32Index index,
        const PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept -> HD;
    auto Depth() const noexcept -> int;
    auto Fingerprint() const noexcept -> Bip32Fingerprint;
    auto Parent() const noexcept -> Bip32Fingerprint;
    auto Xprv(const PasswordPrompt& reason, Writer&& out) const noexcept
        -> bool;
    auto Xpub(const PasswordPrompt& reason, Writer&& out) const noexcept
        -> bool;

    OPENTXS_NO_EXPORT HD(KeyPrivate* imp) noexcept;
    HD(allocator_type alloc = {}) noexcept;
    HD(const HD& rhs, allocator_type alloc = {}) noexcept;
    HD(HD&& rhs) noexcept;
    HD(HD&& rhs, allocator_type alloc) noexcept;
    auto operator=(const HD& rhs) noexcept -> HD&;
    auto operator=(HD&& rhs) noexcept -> HD&;

    ~HD() override;
};
}  // namespace opentxs::crypto::asymmetric::key
