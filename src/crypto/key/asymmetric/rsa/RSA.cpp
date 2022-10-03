// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "crypto/key/asymmetric/rsa/RSA.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <stdexcept>

#include "crypto/key/asymmetric/Asymmetric.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::crypto::key::implementation
{
RSA::RSA(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serialized) noexcept(false)
    : Asymmetric(
          api,
          engine,
          serialized,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return deserialize_key(api, serialized, pub, prv);
          })
    , params_(api_.Factory().DataFromBytes(serialized.params()))
{
}

RSA::RSA(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const Parameters& options,
    Space& params,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(
          api,
          engine,
          crypto::key::asymmetric::Algorithm::Legacy,
          role,
          version,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return create_key(
                  api,
                  engine,
                  options,
                  role,
                  pub.WriteInto(),
                  prv.WriteInto(),
                  prv,
                  writer(params),
                  reason);
          })
    , params_(api_.Factory().Data(params))
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}

RSA::RSA(const RSA& rhs) noexcept
    : Asymmetric(rhs)
    , params_(rhs.params_)
{
}

auto RSA::asPublic() const noexcept -> std::unique_ptr<key::Asymmetric>
{

    auto output = std::make_unique<RSA>(*this);

    OT_ASSERT(output);

    auto& copy = *output;

    {
        auto lock = Lock{copy.lock_};
        copy.erase_private_data(lock);

        OT_ASSERT(false == copy.has_private(lock));
    }

    return output;
}

auto RSA::deserialize_key(
    const api::Session& api,
    const proto::AsymmetricKey& proto,
    Data& publicKey,
    Secret&) noexcept(false) -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::unique_ptr<proto::Ciphertext>{};
    publicKey.Assign(proto.key());

    if (proto.has_encryptedkey()) {
        output = std::make_unique<proto::Ciphertext>(proto.encryptedkey());

        OT_ASSERT(output);
    }

    return output;
}

auto RSA::serialize(const Lock& lock, Serialized& output) const noexcept -> bool
{
    if (false == Asymmetric::serialize(lock, output)) { return false; }

    if (crypto::key::asymmetric::Role::Encrypt == role_) {
        output.set_params(params_.data(), params_.size());
    }

    return true;
}
}  // namespace opentxs::crypto::key::implementation
