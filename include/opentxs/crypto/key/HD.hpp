// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Hash;
}  // namespace crypto
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
}  // namespace key
}  // namespace crypto

using OTHDKey = Pimpl<crypto::key::HD>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key
{
class OPENTXS_EXPORT HD : virtual public EllipticCurve
{
public:
    static auto CalculateFingerprint(
        const api::crypto::Hash& hash,
        const ReadView pubkey) noexcept -> Bip32Fingerprint;

    virtual auto Chaincode(const PasswordPrompt& reason) const noexcept
        -> ReadView = 0;
    virtual auto ChildKey(const Bip32Index index, const PasswordPrompt& reason)
        const noexcept -> std::unique_ptr<HD> = 0;
    virtual auto Depth() const noexcept -> int = 0;
    virtual auto Fingerprint() const noexcept -> Bip32Fingerprint = 0;
    virtual auto Parent() const noexcept -> Bip32Fingerprint = 0;
    virtual auto Xprv(const PasswordPrompt& reason) const noexcept
        -> UnallocatedCString = 0;
    virtual auto Xpub(const PasswordPrompt& reason) const noexcept
        -> UnallocatedCString = 0;

    ~HD() override = default;

protected:
    HD() = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::crypto::key
