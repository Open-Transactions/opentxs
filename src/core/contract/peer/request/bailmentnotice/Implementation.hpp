// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/peer/request/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/request/base/Implementation.hpp"
#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::bailmentnotice
{
class Implementation final : public BailmentNoticePrivate,
                             public base::Implementation
{
public:
    [[nodiscard]] auto Amount() const noexcept -> opentxs::Amount final
    {
        return amount_;
    }
    [[nodiscard]] auto asBailmentNoticePublic() const& noexcept
        -> const request::BailmentNotice& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Description() const noexcept -> std::string_view final
    {
        return description_;
    }
    [[nodiscard]] auto InReferenceToRequest() const noexcept
        -> const identifier_type& final
    {
        return in_reference_to_;
    }
    [[nodiscard]] auto Notary() const noexcept
        -> const identifier::Notary& final
    {
        return notary_;
    }
    [[nodiscard]] auto Unit() const noexcept
        -> const identifier::UnitDefinition& final
    {
        return unit_;
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Implementation(
        const api::Session& api,
        Nym_p signer,
        identifier::Nym initiator,
        identifier::Nym responder,
        identifier::Notary notary,
        identifier::UnitDefinition unit,
        peer::Request::identifier_type ref,
        std::string_view description,
        opentxs::Amount amount,
        allocator_type alloc) noexcept(false);
    Implementation(
        const api::Session& api,
        Nym_p signer,
        const serialized_type& proto,
        allocator_type alloc) noexcept(false);
    Implementation() = delete;
    Implementation(const Implementation& rhs, allocator_type alloc) noexcept;
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&&) = delete;
    auto operator=(const Implementation&) -> Implementation& = delete;
    auto operator=(Implementation&&) -> Implementation& = delete;

    ~Implementation() final;

private:
    static constexpr auto default_version_ = VersionNumber{6};

    const identifier::Notary notary_;
    const identifier::UnitDefinition unit_;
    const peer::Request::identifier_type in_reference_to_;
    const CString description_;
    const opentxs::Amount amount_;
    request::BailmentNotice self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::request::bailmentnotice
