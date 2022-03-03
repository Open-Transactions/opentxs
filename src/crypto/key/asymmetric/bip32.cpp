// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "crypto/key/asymmetric/HD.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/api/Crypto.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/HDPath.pb.h"

namespace opentxs::crypto::key::implementation
{
auto HD::ChildKey(const Bip32Index index, const PasswordPrompt& reason)
    const noexcept -> std::unique_ptr<key::HD>
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
                    std::copy(
                        path_->child().begin(),
                        path_->child().end(),
                        std::back_inserter(out));
                }

                return out;
            }();

            if (hasPrivate) {
                return api_.Crypto().BIP32().DerivePrivateKey(
                    *this, {index}, reason);
            } else {
                return api_.Crypto().BIP32().DerivePublicKey(
                    *this, {index}, reason);
            }
        }();
        const auto& [privkey, ccode, pubkey, spath, parent] = serialized;
        const auto path = [&] {
            auto out = proto::HDPath{};

            if (path_) {
                out = *path_;
                out.add_child(index);
            }

            return out;
        }();

        switch (type_) {
            case crypto::key::asymmetric::Algorithm::ED25519: {
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
                    reason);
            }
            case crypto::key::asymmetric::Algorithm::Secp256k1: {
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
                    reason);
            }
            default: {
                throw std::runtime_error{"Unsupported key type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::crypto::key::implementation
