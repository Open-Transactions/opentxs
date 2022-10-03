// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class Ciphertext;
class SymmetricKey;
}  // namespace proto

class PasswordPrompt;
class Secret;

using OTSymmetricKey = Pimpl<crypto::key::Symmetric>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key
{
class OPENTXS_EXPORT Symmetric
{
public:
    /** Generate a blank, invalid key */
    static auto Factory() -> OTSymmetricKey;

    virtual operator bool() const = 0;

    virtual auto api() const -> const api::Session& = 0;

    /** Decrypt ciphertext using the symmetric key
     *
     *  \param[in] ciphertext The data to be decrypted
     *  \param[out] plaintext The encrypted output
     */
    OPENTXS_NO_EXPORT virtual auto Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        const AllocateOutput plaintext) const -> bool = 0;
    virtual auto Decrypt(
        const ReadView& ciphertext,
        const PasswordPrompt& reason,
        const AllocateOutput plaintext) const -> bool = 0;
    /** Encrypt plaintext using the symmetric key
     *
     *  \param[in] plaintext The data to be encrypted
     *  \param[in] iv Nonce for the encryption operation
     *  \param[out] ciphertext The encrypted output
     *  \param[in] attachKey Set to true if the serialized key should be
     *                       embedded in the ciphertext
     *  \param[in] mode The symmetric algorithm to use for encryption
     */
    OPENTXS_NO_EXPORT virtual auto Encrypt(
        const ReadView plaintext,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::Error,
        const ReadView iv = {}) const -> bool = 0;
    virtual auto Encrypt(
        const ReadView plaintext,
        const PasswordPrompt& reason,
        AllocateOutput ciphertext,
        const bool attachKey = true,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::Error,
        const ReadView iv = {}) const -> bool = 0;
    virtual auto ID(const PasswordPrompt& reason) const
        -> identifier::Generic = 0;
    virtual auto RawKey(const PasswordPrompt& reason, Secret& output) const
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::SymmetricKey& output) const
        -> bool = 0;
    virtual auto Unlock(const PasswordPrompt& reason) const -> bool = 0;

    virtual auto ChangePassword(
        const PasswordPrompt& reason,
        const Secret& newPassword) -> bool = 0;

    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    auto operator=(const Symmetric&) -> Symmetric& = delete;
    auto operator=(Symmetric&&) -> Symmetric& = delete;

    virtual ~Symmetric() = default;

protected:
    Symmetric() = default;

private:
    friend OTSymmetricKey;

    virtual auto clone() const -> Symmetric* = 0;
};
}  // namespace opentxs::crypto::key
