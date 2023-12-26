// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Signature.pb.h>
#include <span>

#include "internal/identity/wot/verification/Item.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace identity
{
namespace wot
{
namespace verification
{
namespace internal
{
struct Nym;
}  // namespace internal
}  // namespace verification
}  // namespace wot

class Nym;
}  // namespace identity

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::implementation
{
class Item final : public internal::Item
{
public:
    auto Begin() const noexcept -> Time final { return start_; }
    auto ClaimID() const noexcept -> const identifier::Generic& final
    {
        return claim_;
    }
    auto End() const noexcept -> Time final { return end_; }
    auto ID() const noexcept -> const identifier::Generic& final { return id_; }
    auto Serialize(const api::Crypto& crypto) const noexcept
        -> SerializedType final;
    auto Signature() const noexcept -> const proto::Signature& final
    {
        return sig_;
    }
    auto Superscedes() const noexcept -> std::span<const VerificationID> final
    {
        return superscedes_;
    }
    auto Value() const noexcept -> Type final { return value_; }
    auto Version() const noexcept -> VersionNumber final { return version_; }

    Item() = delete;
    Item(const Item&) = delete;
    Item(Item&&) = delete;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;

    ~Item() final = default;

private:
    friend opentxs::Factory;

    const VersionNumber version_;
    const identifier::Generic claim_;
    const Type value_;
    const Time start_;
    const Time end_;
    const identifier::Generic id_;
    const proto::Signature sig_;
    const Vector<VerificationID> superscedes_;

    static auto get_sig(
        const identity::Nym& signer,
        const wot::VerificationID& id,
        const wot::ClaimID& claim,
        VersionNumber version,
        Type value,
        Time start,
        Time end,
        std::span<const identity::wot::VerificationID> superscedes,
        const PasswordPrompt& reason) noexcept(false) -> proto::Signature;
    static auto sig_form(
        const wot::VerificationID& id,
        const wot::ClaimID& claim,
        VersionNumber version,
        Type value,
        Time start,
        Time end,
        std::span<const identity::wot::VerificationID> superscedes) noexcept
        -> SerializedType;

    Item(
        const internal::Nym& parent,
        const wot::ClaimID& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        Type value,
        Time start,
        Time end,
        VersionNumber version,
        std::span<const identity::wot::VerificationID>
            superscedes) noexcept(false);
    Item(
        const internal::Nym& parent,
        const SerializedType& serialized) noexcept(false);
};
}  // namespace opentxs::identity::wot::verification::implementation
