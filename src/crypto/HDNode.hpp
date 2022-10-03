// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class HDNode
{
    OTSecret data_space_;
    OTSecret hash_space_;

public:
    WritableView data_;
    WritableView hash_;

    auto Assign(const EcdsaCurve& curve, Bip32::Key& out) const noexcept(false)
        -> void;
    auto check() const noexcept(false) -> void;

    auto Fingerprint() const noexcept -> Bip32Fingerprint;
    auto ParentCode() const noexcept -> ReadView;
    auto ParentPrivate() const noexcept -> ReadView;
    auto ParentPublic() const noexcept -> ReadView;

    auto ChildCode() noexcept -> WritableView;
    auto ChildPrivate() noexcept -> AllocateOutput;
    auto ChildPublic() noexcept -> AllocateOutput;

    auto InitCode() noexcept -> AllocateOutput;
    auto InitPrivate() noexcept -> AllocateOutput;
    auto InitPublic() noexcept -> AllocateOutput;

    auto Next() noexcept -> void;

    HDNode(const api::Crypto& crypto) noexcept;

private:
    const api::Crypto& crypto_;
    int switch_;
    OTSecret a_;
    OTSecret b_;

    auto parent() const noexcept -> const Secret&;

    auto child() noexcept -> Secret&;
    auto parent() noexcept -> Secret&;
};
}  // namespace opentxs::crypto::implementation
