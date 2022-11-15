// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::client::StorageBox

#pragma once

#include <future>
#include <memory>

#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Message;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::activity
{
class MailCache
{
public:
    auto LoadMail(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox& box) const noexcept
        -> std::unique_ptr<Message>;

    auto CacheText(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        const UnallocatedCString& text) noexcept -> void;
    auto GetText(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        const PasswordPrompt& reason) noexcept
        -> std::shared_future<UnallocatedCString>;

    MailCache(
        const api::Session& api,
        const opentxs::network::zeromq::socket::Publish&
            messageLoaded) noexcept;
    MailCache() = delete;
    MailCache(const MailCache&) = delete;
    MailCache(MailCache&&) = delete;
    auto operator=(const MailCache&) -> MailCache& = delete;
    auto operator=(MailCache&&) -> MailCache& = delete;

    ~MailCache();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::api::session::activity
