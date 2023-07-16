// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/peer/reply/bailment/BailmentPrivate.hpp"
#include "core/contract/peer/reply/base/Implementation.hpp"
#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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

namespace opentxs::contract::peer::reply::bailment
{
class Implementation final : public BailmentPrivate, public base::Implementation
{
public:
    [[nodiscard]] auto asBailmentPublic() const& noexcept
        -> const reply::Bailment& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Instructions() const noexcept -> std::string_view final
    {
        return instructions_;
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
        peer::Request::identifier_type ref,
        std::string_view instructions,
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

    const CString instructions_;
    reply::Bailment self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::reply::bailment
