// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "crypto/key/asymmetric/HD.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <HDPath.pb.h>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include "crypto/key/asymmetric/EllipticCurve.hpp"
#include "internal/api/crypto/Symmetric.hpp"
#include "internal/core/String.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/HDIndex.hpp"

namespace opentxs::crypto::key
{
auto HD::CalculateFingerprint(
    const api::crypto::Hash& hash,
    const ReadView key) noexcept -> Bip32Fingerprint
{
    auto output = Bip32Fingerprint{0};
    auto digest = ByteArray{};

    if (33 != key.size()) {
        LogError()(OT_PRETTY_STATIC(HD))("Invalid public key").Flush();

        return output;
    }

    const auto hashed =
        hash.Digest(crypto::HashType::Bitcoin, key, digest.WriteInto());

    if (false == hashed) {
        LogError()(OT_PRETTY_STATIC(HD))("Failed to calculate public key hash")
            .Flush();

        return output;
    }

    if (false == digest.Extract(output)) {
        LogError()(OT_PRETTY_STATIC(HD))("Failed to set output").Flush();

        return {};
    }

    return output;
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : EllipticCurve(api, ecdsa, serializedKey)
    , path_(
          serializedKey.has_path()
              ? std::make_shared<proto::HDPath>(serializedKey.path())
              : nullptr)
    , chain_code_(
          serializedKey.has_chaincode()
              ? std::make_unique<proto::Ciphertext>(serializedKey.chaincode())
              : nullptr)
    , plaintext_chain_code_(api.Factory().Secret(0))
    , parent_(serializedKey.bip32_parent())
{
}

HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : EllipticCurve(api, ecdsa, keyType, role, version, reason)
    , path_(nullptr)
    , chain_code_(nullptr)
    , plaintext_chain_code_(api.Factory().Secret(0))
    , parent_(0)
{
}

HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason)
    , path_(nullptr)
    , chain_code_(nullptr)
    , plaintext_chain_code_(api.Factory().Secret(0))
    , parent_(0)
{
}

HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason)
    , path_(std::make_shared<proto::HDPath>(path))
    , chain_code_(encrypt_key(sessionKey, reason, false, chainCode.Bytes()))
    , plaintext_chain_code_(chainCode)
    , parent_(parent)
{
    OT_ASSERT(path_);
    OT_ASSERT(chain_code_);
}

HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version) noexcept(false)
    : EllipticCurve(api, ecdsa, keyType, privateKey, publicKey, role, version)
    , path_(std::make_shared<proto::HDPath>(path))
    , chain_code_()
    , plaintext_chain_code_(chainCode)
    , parent_(parent)
{
    OT_ASSERT(path_);
}

HD::HD(const HD& rhs) noexcept
    : EllipticCurve(rhs)
    , path_(bool(rhs.path_) ? new proto::HDPath(*rhs.path_) : nullptr)
    , chain_code_(
          bool(rhs.chain_code_) ? new proto::Ciphertext(*rhs.chain_code_)
                                : nullptr)
    , plaintext_chain_code_(rhs.plaintext_chain_code_)
    , parent_(rhs.parent_)
{
}

HD::HD(const HD& rhs, const ReadView newPublic) noexcept
    : EllipticCurve(rhs, newPublic)
    , path_()
    , chain_code_()
    , plaintext_chain_code_(api_.Factory().Secret(0))
    , parent_()
{
}

HD::HD(const HD& rhs, Secret&& newSecretKey) noexcept
    : EllipticCurve(rhs, std::move(newSecretKey))
    , path_()
    , chain_code_()
    , plaintext_chain_code_(api_.Factory().Secret(0))
    , parent_()
{
}

auto HD::Chaincode(const PasswordPrompt& reason) const noexcept -> ReadView
{
    auto lock = Lock{lock_};

    return chaincode(lock, reason);
}

auto HD::chaincode(const Lock& lock, const PasswordPrompt& reason)
    const noexcept -> ReadView
{
    try {

        return get_chain_code(lock, reason).Bytes();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto HD::Depth() const noexcept -> int
{
    if (false == bool(path_)) { return -1; }

    return path_->child_size();
}

auto HD::erase_private_data(const Lock& lock) -> void
{
    EllipticCurve::erase_private_data(lock);
    const_cast<std::shared_ptr<const proto::HDPath>&>(path_).reset();
    const_cast<std::unique_ptr<const proto::Ciphertext>&>(chain_code_).reset();
}

auto HD::Fingerprint() const noexcept -> Bip32Fingerprint
{
    return CalculateFingerprint(api_.Crypto().Hash(), PublicKey());
}

auto HD::get_chain_code(const Lock& lock, const PasswordPrompt& reason) const
    noexcept(false) -> Secret&
{
    if (0 == plaintext_chain_code_.size()) {
        if (false == bool(encrypted_key_)) {
            throw std::runtime_error{"Missing encrypted private key"};
        }
        if (false == bool(chain_code_)) {
            throw std::runtime_error{"Missing encrypted chain code"};
        }

        const auto& chaincode = *chain_code_;
        const auto& privateKey = *encrypted_key_;
        // Private key data and chain code are encrypted to the same session
        // key, and this session key is only embedded in the private key
        // ciphertext
        auto sessionKey = api_.Crypto().Symmetric().InternalSymmetric().Key(
            privateKey.key(),
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305);

        if (false == sessionKey.get()) {
            throw std::runtime_error{"Failed to extract session key"};
        }

        auto allocator = plaintext_chain_code_.WriteInto(Secret::Mode::Mem);

        if (false == sessionKey->Decrypt(chaincode, reason, allocator)) {
            throw std::runtime_error{"Failed to decrypt chain code"};
        }
    }

    return plaintext_chain_code_;
}

auto HD::get_params() const noexcept -> std::tuple<bool, Bip32Depth, Bip32Index>
{
    std::tuple<bool, Bip32Depth, Bip32Index> output{false, 0x0, 0x0};
    auto& [success, depth, child] = output;

    if (false == bool(path_)) {
        LogError()(OT_PRETTY_CLASS())("missing path").Flush();

        return output;
    }

    const auto& path = *path_;
    auto size = path.child_size();

    if (0 > size) {
        LogError()(OT_PRETTY_CLASS())("Invalid depth (")(size)(")").Flush();

        return output;
    }

    if (std::numeric_limits<Bip32Depth>::max() < size) {
        LogError()(OT_PRETTY_CLASS())("Invalid depth (")(size)(")").Flush();

        return output;
    }

    depth = static_cast<std::uint8_t>(size);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    if (0 < depth) {
        const auto& index = *(path_->child().rbegin());
        child = index;
    }
#pragma GCC diagnostic pop

    success = true;

    return output;
}

auto HD::Path() const noexcept -> const UnallocatedCString
{
    auto path = String::Factory();

    if (path_) {
        if (path_->has_root()) {
            const auto root =
                api_.Factory().IdentifierFromBase58(path_->root());
            path->Concatenate(String::Factory(root));

            for (const auto& it : path_->child()) {
                static auto slash = String::Factory(UnallocatedCString{" / "});
                path->Concatenate(slash);
                if (it < HDIndex{Bip32Child::HARDENED}) {
                    path->Concatenate(String::Factory(std::to_string(it)));
                } else {
                    path->Concatenate(String::Factory(
                        std::to_string(it - HDIndex{Bip32Child::HARDENED})));
                    static auto amp = String::Factory(UnallocatedCString{"'"});
                    path->Concatenate(amp);
                }
            }
        }
    }

    return path->Get();
}

auto HD::Path(proto::HDPath& output) const noexcept -> bool
{
    if (path_) {
        output = *path_;

        return true;
    }

    LogError()(OT_PRETTY_CLASS())("HDPath not instantiated.").Flush();

    return false;
}

auto HD::serialize(const Lock& lock, Serialized& output) const noexcept -> bool
{
    if (false == EllipticCurve::serialize(lock, output)) { return false; }

    if (has_private(lock)) {
        if (path_) { *(output.mutable_path()) = *path_; }

        if (chain_code_) { *output.mutable_chaincode() = *chain_code_; }
    }

    if (1 < version_) { output.set_bip32_parent(parent_); }

    return true;
}

auto HD::Xprv(const PasswordPrompt& reason) const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    auto privateKey = api_.Factory().SecretFromBytes(private_key(lock, reason));

    // FIXME Bip32::SerializePrivate should accept ReadView

    return api_.Crypto().BIP32().SerializePrivate(
        0x0488ADE4,
        depth,
        parent_,
        child,
        api_.Factory().DataFromBytes(chaincode(lock, reason)),
        privateKey);
}

auto HD::Xpub(const PasswordPrompt& reason) const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    // FIXME Bip32::SerializePublic should accept ReadView

    return api_.Crypto().BIP32().SerializePublic(
        0x0488B21E,
        depth,
        parent_,
        child,
        api_.Factory().DataFromBytes(chaincode(lock, reason)),
        api_.Factory().DataFromBytes(PublicKey()));
}
}  // namespace opentxs::crypto::key::implementation
