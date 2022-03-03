// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/identity/credential/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class Signature;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Key : virtual public Base
{
public:
    virtual auto GetKeypair(
        const crypto::key::asymmetric::Algorithm type,
        const opentxs::crypto::key::asymmetric::Role role) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetKeypair(const opentxs::crypto::key::asymmetric::Role role)
        const -> const crypto::key::Keypair& = 0;
    virtual auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const opentxs::Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t = 0;
    OPENTXS_NO_EXPORT virtual auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const crypto::HashType hash = crypto::HashType::Error) const
        -> bool = 0;

    ~Key() override = default;

protected:
    Key() noexcept {}  // TODO Signable

private:
    Key(const Key&) = delete;
    Key(Key&&) = delete;
    auto operator=(const Key&) -> Key& = delete;
    auto operator=(Key&&) -> Key& = delete;
};
}  // namespace opentxs::identity::credential
