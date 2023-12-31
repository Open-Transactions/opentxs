// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/hd/Imp.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/HDPath.pb.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "internal/api/Crypto.hpp"
#include "internal/crypto/Crypto.hpp"
#include "internal/crypto/asymmetric/Factory.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/Ed25519.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::crypto::asymmetric::key::implementation
{
auto HD::ChildKey(
    const Bip32Index index,
    const PasswordPrompt& reason,
    allocator_type alloc) const noexcept -> asymmetric::key::HD
{
    try {
        static const auto blank = api_.Factory().Secret(0);
        const auto hasPrivate = [&] {
            auto lock = Lock{lock_};

            return has_private(lock);
        }();
        const auto serialized = [&] {
            const auto path = [&] {
                auto out = Bip32::Path{};

                if (path_) {
                    std::ranges::copy(path_->child(), std::back_inserter(out));
                }

                return out;
            }();

            if (hasPrivate) {
                return api_.Crypto().BIP32().Internal().DerivePrivateKey(
                    *this, {index}, reason);
            } else {
                return api_.Crypto().BIP32().Internal().DerivePublicKey(
                    *this, {index}, reason);
            }
        }();
        const auto& [privkey, ccode, pubkey, spath, parent] = serialized;
        const auto path = [&] {
            auto out = protobuf::HDPath{};

            if (path_) {
                out = *path_;
                out.add_child(index);
            }

            return out;
        }();

        switch (type_) {
            case crypto::asymmetric::Algorithm::ED25519: {
                return factory::Ed25519Key(
                    api_,
                    api_.Crypto().Internal().EllipticProvider(type_),
                    hasPrivate ? privkey : blank,
                    ccode,
                    pubkey,
                    path,
                    parent,
                    role_,
                    version_,
                    reason,
                    alloc);
            }
            case crypto::asymmetric::Algorithm::Secp256k1: {
                return factory::Secp256k1Key(
                    api_,
                    api_.Crypto().Internal().EllipticProvider(type_),
                    hasPrivate ? privkey : blank,
                    ccode,
                    pubkey,
                    path,
                    parent,
                    role_,
                    version_,
                    reason,
                    alloc);
            }
            case crypto::asymmetric::Algorithm::Error:
            case crypto::asymmetric::Algorithm::Null:
            case crypto::asymmetric::Algorithm::Legacy:
            default: {
                throw std::runtime_error{"Unsupported key type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::crypto::asymmetric::key::implementation
