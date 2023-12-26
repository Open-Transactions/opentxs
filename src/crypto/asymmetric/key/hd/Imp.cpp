// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/hd/Imp.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <HDPath.pb.h>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Symmetric.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"           // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/HDIndex.hpp"

namespace opentxs::crypto::asymmetric::key::implementation
{
HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey,
    allocator_type alloc) noexcept(false)
    : EllipticCurve(api, ecdsa, serializedKey, alloc)
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
    const crypto::asymmetric::Algorithm keyType,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          role,
          version,
          sessionKey,
          reason,
          alloc)
    , path_(nullptr)
    , chain_code_(nullptr)
    , plaintext_chain_code_(api.Factory().Secret(0))
    , parent_(0)
{
}

HD::HD(
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
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason,
          alloc)
    , path_(nullptr)
    , chain_code_(nullptr)
    , plaintext_chain_code_(api.Factory().Secret(0))
    , parent_(0)
{
}

HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason,
          alloc)
    , path_(std::make_shared<proto::HDPath>(path))
    , chain_code_(encrypt_key(chainCode.Bytes(), false, sessionKey, reason))
    , plaintext_chain_code_(chainCode)
    , parent_(parent)
{
    assert_false(nullptr == path_);
    assert_false(nullptr == chain_code_);
}

HD::HD(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Algorithm keyType,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    allocator_type alloc) noexcept(false)
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          alloc)
    , path_(std::make_shared<proto::HDPath>(path))
    , chain_code_()
    , plaintext_chain_code_(chainCode)
    , parent_(parent)
{
    assert_false(nullptr == path_);
}

HD::HD(const HD& rhs, allocator_type alloc) noexcept
    : EllipticCurve(rhs, alloc)
    , path_(bool(rhs.path_) ? new proto::HDPath(*rhs.path_) : nullptr)
    , chain_code_(
          bool(rhs.chain_code_) ? new proto::Ciphertext(*rhs.chain_code_)
                                : nullptr)
    , plaintext_chain_code_(rhs.plaintext_chain_code_)
    , parent_(rhs.parent_)
{
}

HD::HD(const HD& rhs, const ReadView newPublic, allocator_type alloc) noexcept
    : EllipticCurve(rhs, newPublic, alloc)
    , path_()
    , chain_code_()
    , plaintext_chain_code_(api_.Factory().Secret(0))
    , parent_()
{
}

HD::HD(const HD& rhs, Secret&& newSecretKey, allocator_type alloc) noexcept
    : EllipticCurve(rhs, std::move(newSecretKey), alloc)
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
        LogError()()(e.what()).Flush();

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
            opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305);

        if (false == sessionKey) {
            throw std::runtime_error{"Failed to extract session key"};
        }

        const auto decrypted = sessionKey.Internal().Decrypt(
            chaincode,
            plaintext_chain_code_.WriteInto(Secret::Mode::Mem),
            reason);

        if (false == decrypted) {
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
        LogError()()("missing path").Flush();

        return output;
    }

    const auto& path = *path_;
    auto size = path.child_size();

    if (0 > size) {
        LogError()()("Invalid depth (")(size)(")").Flush();

        return output;
    }

    if (std::numeric_limits<Bip32Depth>::max() < size) {
        LogError()()("Invalid depth (")(size)(")").Flush();

        return output;
    }

    depth = static_cast<std::uint8_t>(size);

    if (0 < depth) {
        const auto& index = *(path_->child().rbegin());
        child = index;
    }

    success = true;

    return output;
}

auto HD::Path() const noexcept -> const UnallocatedCString
{
    auto path = String::Factory();

    if (path_) {
        if (path_->has_seed()) {
            const auto root = api_.Factory().Internal().SeedID(path_->seed());
            path->Concatenate(String::Factory(root, api_.Crypto()));

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

    LogError()()("HDPath not instantiated.").Flush();

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

auto HD::Xprv(const PasswordPrompt& reason, Writer&& out) const noexcept -> bool
{
    auto lock = Lock{lock_};
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    return api_.Crypto().BIP32().SerializePrivate(
        0x0488ADE4,
        depth,
        parent_,
        child,
        chaincode(lock, reason),
        private_key(lock, reason),
        std::move(out));
}

auto HD::Xpub(const PasswordPrompt& reason, Writer&& out) const noexcept -> bool
{
    auto lock = Lock{lock_};
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    return api_.Crypto().BIP32().SerializePublic(
        0x0488B21E,
        depth,
        parent_,
        child,
        chaincode(lock, reason),
        PublicKey(),
        std::move(out));
}

HD::~HD() = default;
}  // namespace opentxs::crypto::asymmetric::key::implementation
