// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string_view>

#include "core/contract/peer/reply/base/Implementation.hpp"
#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/connection/ConnectionPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
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

namespace opentxs::contract::peer::reply::connection
{
class Implementation final : public ConnectionPrivate,
                             public base::Implementation
{
public:
    [[nodiscard]] auto Accepted() const noexcept -> bool final
    {
        return accepted_;
    }
    [[nodiscard]] auto asConnectionPublic() const& noexcept
        -> const reply::Connection& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Endpoint() const noexcept -> std::string_view final
    {
        return endpoint_;
    }
    [[nodiscard]] auto Key() const noexcept -> std::string_view final
    {
        return key_;
    }
    [[nodiscard]] auto Login() const noexcept -> std::string_view final
    {
        return login_;
    }
    [[nodiscard]] auto Password() const noexcept -> std::string_view final
    {
        return password_;
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
        peer::Request::identifier_type ref,
        bool accepted,
        std::string_view endpoint,
        std::string_view login,
        std::string_view password,
        std::string_view key,
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

    const bool accepted_;
    const CString endpoint_;
    const CString login_;
    const CString password_;
    const CString key_;
    reply::Connection self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::reply::connection
