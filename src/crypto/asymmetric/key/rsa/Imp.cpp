// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/rsa/Imp.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <stdexcept>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Role.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::key::implementation
{
RSA::RSA(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serialized,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , RSAPrivate(alloc)
    , Key(
          api,
          engine,
          serialized,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return deserialize_key(api, serialized, pub, prv);
          },
          alloc)
    , params_(api_.Factory().DataFromBytes(serialized.params()))
    , self_(this)
{
}

RSA::RSA(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    const Parameters& options,
    Space& params,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , RSAPrivate(alloc)
    , Key(
          api,
          engine,
          crypto::asymmetric::Algorithm::Legacy,
          role,
          version,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return create_key(
                  sessionKey,
                  engine,
                  options,
                  role,
                  pub.WriteInto(),
                  prv.WriteInto(),
                  prv,
                  writer(params),
                  reason);
          },
          alloc)
    , params_(api_.Factory().Data(params))
    , self_(this)
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}

RSA::RSA(const RSA& rhs, allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , RSAPrivate(alloc)
    , Key(rhs, alloc)
    , params_(rhs.params_)
    , self_(this)
{
}

auto RSA::clone(allocator_type alloc) const noexcept -> RSA*
{
    auto pmr = alloc::PMR<RSA>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
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

auto RSA::get_deleter() const noexcept -> std::function<void(KeyPrivate*)>
{
    return [alloc = alloc::PMR<RSA>{get_allocator()}](KeyPrivate* in) mutable {
        auto* p = dynamic_cast<RSA*>(in);

        OT_ASSERT(nullptr != p);

        alloc.destroy(p);
        alloc.deallocate(p, 1_uz);
    };
}

auto RSA::PreferredHash() const noexcept -> crypto::HashType
{
    return crypto::HashType::Sha256;
}

auto RSA::serialize(const Lock& lock, Serialized& output) const noexcept -> bool
{
    if (false == Key::serialize(lock, output)) { return false; }

    if (crypto::asymmetric::Role::Encrypt == role_) {
        output.set_params(params_.data(), params_.size());
    }

    return true;
}

RSA::~RSA() { Reset(self_); }
}  // namespace opentxs::crypto::asymmetric::key::implementation
