// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Signature.pb.h>
#include <cs_shared_guarded.h>
#include <functional>
#include <shared_mutex>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::base
{
class Implementation : virtual public RequestPrivate
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> RequestPrivate* override
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto ID() const noexcept -> const identifier_type& final;
    [[nodiscard]] auto Initiator() const noexcept
        -> const identifier::Nym& final
    {
        return initiator_;
    }
    [[nodiscard]] auto IsValid() const noexcept -> bool final { return true; }
    [[nodiscard]] auto Received() const noexcept -> Time final;
    [[nodiscard]] auto Responder() const noexcept
        -> const identifier::Nym& final
    {
        return responder_;
    }
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool final;
    [[nodiscard]] auto Serialize(serialized_type& out) const noexcept
        -> bool final;
    [[nodiscard]] auto Signer() const noexcept -> Nym_p final
    {
        return signer_;
    }
    [[nodiscard]] auto Validate() const noexcept -> bool final;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber final
    {
        return version_;
    }

    [[nodiscard]] auto Finish(const PasswordPrompt& reason) noexcept -> bool;
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }

    Implementation() = delete;
    Implementation(const Implementation& rhs, allocator_type alloc) noexcept;
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&&) = delete;
    auto operator=(const Implementation&) -> Implementation& = delete;
    auto operator=(Implementation&&) -> Implementation& = delete;

    ~Implementation() override;

protected:
    const api::Session& api_;

    virtual auto id_form() const noexcept -> serialized_type;

    Implementation(
        const api::Session& api,
        Nym_p signer,
        VersionNumber version,
        identifier::Nym initiator,
        identifier::Nym responder,
        allocator_type alloc) noexcept(false);
    Implementation(
        const api::Session& api,
        Nym_p signer,
        const serialized_type& proto,
        allocator_type alloc) noexcept(false);

private:
    using GuardedTime = libguarded::shared_guarded<Time, std::shared_mutex>;

    static constexpr auto minimum_version_ = VersionNumber{2u};

    const Nym_p signer_;
    const VersionNumber version_;
    const identifier::Nym initiator_;
    const identifier::Nym responder_;
    const identifier_type cookie_;
    DeferredConstruction<identifier_type> id_;
    DeferredConstruction<proto::Signature> sig_;
    GuardedTime time_;

    static auto calculate_id(
        const api::Session& api,
        const serialized_type& contract) noexcept -> identifier_type;

    auto calculate_id() const noexcept -> identifier_type;
    auto check_nym() const noexcept(false) -> void;
    auto final_form() const noexcept -> serialized_type;
    auto signing_form() const noexcept -> serialized_type;
    auto validate() const noexcept -> bool;
    auto verify_signature(const proto::Signature& signature) const noexcept
        -> bool;

    auto add_signature(const PasswordPrompt& reason) -> bool;
};
}  // namespace opentxs::contract::peer::request::base
