// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/ellipticcurve/Imp.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <stdexcept>
#include <string>
#include <utility>

#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::key::implementation
{
EllipticCurve::EllipticCurve(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized,
    allocator_type alloc) noexcept(false)
    : Key(
          api,
          ecdsa,
          serialized,
          [&](auto& pubkey, auto&) -> EncryptedKey {
              return extract_key(api, ecdsa, serialized, pubkey);
          },
          alloc)
    , ecdsa_(ecdsa)
{
}

EllipticCurve::EllipticCurve(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Algorithm keyType,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : Key(
          api,
          ecdsa,
          keyType,
          role,
          version,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return create_key(
                  sessionKey,
                  ecdsa,
                  {},
                  role,
                  pub.WriteInto(),
                  prv.WriteInto(Secret::Mode::Mem),
                  prv,
                  {},
                  reason);
          },
          alloc)
    , ecdsa_(ecdsa)
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }

    OT_ASSERT(0 < plaintext_key_.size());
}

EllipticCurve::EllipticCurve(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : Key(
          api,
          ecdsa,
          keyType,
          role,
          true,
          (false == privateKey.empty()),
          version,
          ByteArray{publicKey},
          [&](auto&, auto&) mutable -> EncryptedKey {
              return encrypt_key(privateKey.Bytes(), true, sessionKey, reason);
          },
          {},
          alloc)
    , ecdsa_(ecdsa)
{
    auto lock = Lock{lock_};

    if (has_private(lock) && !encrypted_key_) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}

EllipticCurve::EllipticCurve(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    allocator_type alloc) noexcept(false)
    : Key(api,
          ecdsa,
          keyType,
          role,
          true,
          (false == privateKey.empty()),
          version,
          ByteArray{publicKey},
          {},
          {},
          alloc)
    , ecdsa_(ecdsa)
{
}

EllipticCurve::EllipticCurve(
    const EllipticCurve& rhs,
    allocator_type alloc) noexcept
    : Key(rhs, alloc)
    , ecdsa_(rhs.ecdsa_)
{
}

EllipticCurve::EllipticCurve(
    const EllipticCurve& rhs,
    const ReadView newPublic,
    allocator_type alloc) noexcept
    : Key(rhs, newPublic, alloc)
    , ecdsa_(rhs.ecdsa_)
{
}

EllipticCurve::EllipticCurve(
    const EllipticCurve& rhs,
    Secret&& newSecretKey,
    allocator_type alloc) noexcept
    : Key(
          rhs,
          [&] {
              auto pubkey = rhs.api_.Factory().Data();
              const auto rc = rhs.ecdsa_.ScalarMultiplyBase(
                  newSecretKey.Bytes(), pubkey.WriteInto());

              if (rc) {

                  return pubkey;
              } else {
                  LogError()(OT_PRETTY_CLASS())(
                      "Failed to calculate public key")
                      .Flush();

                  return rhs.api_.Factory().Data();
              }
          }(),
          std::move(newSecretKey),
          alloc)
    , ecdsa_(rhs.ecdsa_)
{
}

auto EllipticCurve::extract_key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& proto,
    Data& publicKey) -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::unique_ptr<proto::Ciphertext>{};
    publicKey.Assign(proto.key());

    if ((proto::KEYMODE_PRIVATE == proto.mode()) && proto.has_encryptedkey()) {
        output = std::make_unique<proto::Ciphertext>(proto.encryptedkey());

        OT_ASSERT(output);
    }

    return output;
}

auto EllipticCurve::IncrementPrivate(
    const Secret& rhs,
    const PasswordPrompt& reason,
    allocator_type alloc) const noexcept -> asymmetric::key::EllipticCurve
{
    try {
        auto lock = Lock{lock_};
        const auto& lhs = get_private_key(lock, reason);
        auto newKey = api_.Factory().Secret(0);
        auto rc =
            ecdsa_.ScalarAdd(lhs.Bytes(), rhs.Bytes(), newKey.WriteInto());

        if (false == rc) {
            throw std::runtime_error("Failed to increment private key");
        }

        return replace_secret_key(std::move(newKey), alloc);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {alloc};
    }
}

auto EllipticCurve::IncrementPublic(const Secret& rhs, allocator_type alloc)
    const noexcept -> asymmetric::key::EllipticCurve
{
    try {
        auto newKey = Space{};
        auto rc = ecdsa_.PubkeyAdd(key_.Bytes(), rhs.Bytes(), writer(newKey));

        if (false == rc) {
            throw std::runtime_error("Failed to increment public key");
        }

        return replace_public_key(reader(newKey), alloc);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {alloc};
    }
}

auto EllipticCurve::serialize_public(EllipticCurve* in)
    -> std::shared_ptr<proto::AsymmetricKey>
{
    auto copy = std::unique_ptr<EllipticCurve>{in};

    OT_ASSERT(copy);

    auto serialized = proto::AsymmetricKey{};

    {
        auto lock = Lock{copy->lock_};
        copy->erase_private_data(lock);

        if (false == copy->serialize(lock, serialized)) { return nullptr; }
    }

    return std::make_shared<proto::AsymmetricKey>(serialized);
}

auto EllipticCurve::SignDER(
    const ReadView preimage,
    const crypto::HashType hash,
    Writer&& output,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private(lock)) {
        LogError()(OT_PRETTY_CLASS())("Missing private key").Flush();

        return false;
    }

    bool success = ecdsa_.SignDER(
        preimage, private_key(lock, reason), hash, std::move(output));

    if (false == success) {
        LogError()(OT_PRETTY_CLASS())("Failed to sign preimage").Flush();
    }

    return success;
}

EllipticCurve::~EllipticCurve() = default;
}  // namespace opentxs::crypto::asymmetric::key::implementation
