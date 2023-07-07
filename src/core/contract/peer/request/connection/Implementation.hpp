// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "core/contract/peer/request/base/Implementation.hpp"
#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/connection/ConnectionPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
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

namespace opentxs::contract::peer::request::connection
{
class Implementation final : public ConnectionPrivate,
                             public base::Implementation
{
public:
    [[nodiscard]] auto asConnectionPublic() const& noexcept
        -> const request::Connection& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Kind() const noexcept -> ConnectionInfoType final
    {
        return kind_;
    }

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> final
    {
        return make_deleter(this);
    }

    Implementation(
        const api::Session& api,
        Nym_p signer,
        identifier::Nym initiator,
        identifier::Nym responder,
        ConnectionInfoType kind,
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

    const ConnectionInfoType kind_;
    request::Connection self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::request::connection
