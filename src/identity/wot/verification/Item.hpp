// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Signature.pb.h>

#include "internal/identity/wot/verification/Item.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace verification
{
namespace internal
{
struct Nym;
}
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
    auto Valid() const noexcept -> Validity final { return valid_; }
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
    const Validity valid_;
    const Time start_;
    const Time end_;
    const identifier::Generic id_;
    const proto::Signature sig_;

    static auto calculate_id(
        const api::Session& api,
        const VersionNumber version,
        const identifier::Generic& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid,
        const identifier::Nym& nym) noexcept(false) -> identifier::Generic;
    static auto get_sig(
        const api::Crypto& crypto,
        const identity::Nym& signer,
        const VersionNumber version,
        const identifier::Generic& id,
        const identifier::Generic& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid,
        const PasswordPrompt& reason) noexcept(false) -> proto::Signature;
    static auto id_form(
        const api::Crypto& crypto,
        const VersionNumber version,
        const identifier::Generic& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid) noexcept -> SerializedType;
    static auto sig_form(
        const api::Crypto& crypto,
        const VersionNumber version,
        const identifier::Generic& id,
        const identifier::Generic& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid) noexcept -> SerializedType;

    Item(
        const internal::Nym& parent,
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Type value = Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = DefaultVersion) noexcept(false);
    Item(
        const internal::Nym& parent,
        const SerializedType& serialized) noexcept(false);
};
}  // namespace opentxs::identity::wot::verification::implementation
