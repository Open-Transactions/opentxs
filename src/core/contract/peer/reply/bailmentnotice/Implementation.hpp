// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/reply/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/reply/base/Implementation.hpp"
#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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

namespace opentxs::contract::peer::reply::bailmentnotice
{
class Implementation final : public BailmentNoticePrivate,
                             public base::Implementation
{
public:
    [[nodiscard]] auto asBailmentNoticePublic() const& noexcept
        -> const reply::BailmentNotice& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Value() const noexcept -> bool final { return value_; }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }

    Implementation(
        const api::Session& api,
        Nym_p signer,
        identifier::Nym initiator,
        identifier::Nym responder,
        peer::Request::identifier_type ref,
        bool value,
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
    static constexpr auto default_version_ = VersionNumber{4u};

    const bool value_;
    reply::BailmentNotice self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::reply::bailmentnotice
