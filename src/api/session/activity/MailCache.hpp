// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
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

class Identifier;
class Message;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::activity
{
class MailCache
{
public:
    auto LoadMail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept -> std::unique_ptr<Message>;

    auto CacheText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box,
        const UnallocatedCString& text) noexcept -> void;
    auto GetText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box,
        const PasswordPrompt& reason) noexcept
        -> std::shared_future<UnallocatedCString>;

    MailCache(
        const api::Session& api,
        const opentxs::network::zeromq::socket::Publish&
            messageLoaded) noexcept;

    ~MailCache();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    MailCache() = delete;
    MailCache(const MailCache&) = delete;
    MailCache(MailCache&&) = delete;
    auto operator=(const MailCache&) -> MailCache& = delete;
    auto operator=(MailCache&&) -> MailCache& = delete;
};
}  // namespace opentxs::api::session::activity
