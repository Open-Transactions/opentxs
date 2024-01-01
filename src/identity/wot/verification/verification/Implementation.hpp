// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/Verification.pb.h>
#include <span>

#include "core/contract/Signable.hpp"
#include "identity/wot/verification/verification/VerificationPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/Verification.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace protobuf
{
class Signature;
class VerificationItem;
}  // namespace protobuf

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::implementation
{
class Verification final : public VerificationPrivate,
                           public contract::implementation::Signable<
                               wot::Verification::identifier_type>
{
public:
    [[nodiscard]] auto Claim() const noexcept -> const ClaimID& final
    {
        return claim_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> VerificationPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Verification>{alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    [[nodiscard]] auto ID() const noexcept
        -> const wot::Verification::identifier_type& final
    {
        return Signable::ID();
    }
    [[nodiscard]] auto IsValid() const noexcept -> bool final { return true; }
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Serialize(protobuf::Verification& out) const noexcept -> void final;
    [[nodiscard]] auto Start() const noexcept -> Time final { return start_; }
    [[nodiscard]] auto Stop() const noexcept -> Time final { return stop_; }
    [[nodiscard]] auto Superscedes() const noexcept
        -> std::span<const VerificationID> final
    {
        return superscedes_;
    }
    [[nodiscard]] auto Value() const noexcept -> verification::Type final
    {
        return value_;
    }
    [[nodiscard]] auto Version() const noexcept -> VersionNumber final
    {
        return Signable::Version();
    }

    [[nodiscard]] auto Finish(const PasswordPrompt& reason) noexcept -> bool;

    Verification(
        const api::Session& api,
        Nym_p verifier,
        ClaimID claim,
        verification::Type value,
        Time start,
        Time stop,
        std::span<const VerificationID> superscedes,
        allocator_type alloc) noexcept(false);
    Verification(
        const api::Session& api,
        const protobuf::VerificationItem& proto,
        Nym_p verifier,
        allocator_type alloc) noexcept(false);
    Verification() = delete;
    Verification(const Verification& rhs, allocator_type alloc) noexcept;
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;

    ~Verification() final = default;

private:
    static constexpr auto default_version_ = VersionNumber{1u};

    const ClaimID claim_;
    const verification::Type value_;
    const Time start_;
    const Time stop_;
    const Vector<VerificationID> superscedes_;

    auto calculate_id() const -> identifier_type final;
    auto final_form() const noexcept -> protobuf::Verification;
    auto signing_form() const noexcept -> protobuf::Verification;
    auto validate() const -> bool final;
    auto verify_signature(const protobuf::Signature& signature) const
        -> bool final;

    auto update_signature(const PasswordPrompt& reason) -> bool final;
};
}  // namespace opentxs::identity::wot::verification::implementation
