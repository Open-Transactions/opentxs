// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Factory;
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class HDNode
{
    Secret data_space_;
    Secret hash_space_;

public:
    WriteBuffer data_;
    WriteBuffer hash_;

    auto Assign(const EcdsaCurve& curve, Bip32::Key& out) const noexcept(false)
        -> void;
    auto check() const noexcept(false) -> void;

    auto Fingerprint() const noexcept -> Bip32Fingerprint;
    auto ParentCode() const noexcept -> ReadView;
    auto ParentPrivate() const noexcept -> ReadView;
    auto ParentPublic() const noexcept -> ReadView;

    auto ChildCode() noexcept -> WriteBuffer;
    auto ChildPrivate() noexcept -> Writer;
    auto ChildPublic() noexcept -> Writer;

    auto InitCode() noexcept -> Writer;
    auto InitPrivate() noexcept -> Writer;
    auto InitPublic() noexcept -> Writer;

    auto Next() noexcept -> void;

    HDNode(const api::Factory& factory, const api::Crypto& crypto) noexcept;

private:
    const api::Crypto& crypto_;
    int switch_;
    Secret a_;
    Secret b_;

    auto parent() const noexcept -> const Secret&;

    auto child() noexcept -> Secret&;
    auto parent() noexcept -> Secret&;
};
}  // namespace opentxs::crypto::implementation
