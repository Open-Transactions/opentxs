// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::network::zeromq::socket::implementation
{
template <typename T>
class _Bidirectional : virtual public T, public Bidirectional
{
protected:
    bool Send(opentxs::Data& message) const override
    {
        return Send(Message::Factory(message));
    }
    bool Send(const std::string& message) const override
    {
        return Send(Message::Factory(message));
    }
    bool Send(zeromq::Message& message) const override
    {
        return queue_message(message);
    }

    _Bidirectional(
        const zeromq::Context& context,
        const SocketType type,
        const Socket::Direction direction,
        const bool startThread)
        : Bidirectional(context, type, direction, startThread)
    {
    }

    virtual ~_Bidirectional() = default;

private:
    _Bidirectional() = delete;
    _Bidirectional(const _Bidirectional&) = delete;
    _Bidirectional(_Bidirectional&&) = delete;
    _Bidirectional& operator=(const _Bidirectional&) = delete;
    _Bidirectional& operator=(_Bidirectional&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
