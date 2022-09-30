// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace server
{
class Server;
}  // namespace server

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class MessageProcessor
{
public:
    auto DropIncoming(const int count) const noexcept -> void;
    auto DropOutgoing(const int count) const noexcept -> void;

    auto cleanup() noexcept -> void;
    auto init(
        const bool inproc,
        const int port,
        const Secret& privkey) noexcept(false) -> void;
    auto Start() noexcept -> void;

    MessageProcessor(Server& server, const PasswordPrompt& reason) noexcept;
    MessageProcessor() = delete;
    MessageProcessor(const MessageProcessor&) = delete;
    MessageProcessor(MessageProcessor&&) = delete;
    auto operator=(const MessageProcessor&) -> MessageProcessor& = delete;
    auto operator=(MessageProcessor&&) -> MessageProcessor& = delete;

    ~MessageProcessor();

private:
    class Imp;

    Imp* imp_;
};
}  // namespace opentxs::server
