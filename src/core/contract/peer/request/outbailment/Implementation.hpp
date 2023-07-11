// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/peer/request/base/Implementation.hpp"
#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/outbailment/OutbailmentPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/request/Outbailment.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
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

namespace opentxs::contract::peer::request::outbailment
{
class Implementation final : public OutbailmentPrivate,
                             public base::Implementation
{
public:
    [[nodiscard]] auto Amount() const noexcept -> opentxs::Amount final
    {
        return amount_;
    }
    [[nodiscard]] auto asOutbailmentPublic() const& noexcept
        -> const request::Outbailment& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Instructions() const noexcept -> std::string_view final
    {
        return instructions_;
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
        return make_deleter(this);
    }

    Implementation(
        const api::Session& api,
        Nym_p signer,
        identifier::Nym initiator,
        identifier::Nym responder,
        identifier::Notary notary,
        identifier::UnitDefinition unit,
        std::string_view instructions,
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
    static constexpr auto default_version_ = VersionNumber{4};

    const identifier::Notary notary_;
    const identifier::UnitDefinition unit_;
    const CString instructions_;
    const opentxs::Amount amount_;
    request::Outbailment self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::request::outbailment
