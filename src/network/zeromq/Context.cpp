// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Context.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/network/zeromq/DealerSocket.hpp"
#include "opentxs/network/zeromq/PairSocket.hpp"
#include "opentxs/network/zeromq/Proxy.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/zeromq/RouterSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include "PairEventListener.hpp"

#include <zmq.h>

#include <vector>

template class opentxs::Pimpl<opentxs::network::zeromq::Context>;

#define INPROC_PREFIX "inproc://opentxs/"
#define PATH_SEPERATOR "/"
#define OT_METHOD "opentxs::Context::"

namespace opentxs::network::zeromq
{
OTZMQContext Context::Factory()
{
    return OTZMQContext(new implementation::Context());
}

std::string Context::EncodePrivateZ85(const opentxs::crypto::key::Ed25519& key)
{
    auto data = opentxs::Data::Factory();
    const auto retrieved = key.GetKey(data);

    OT_ASSERT(retrieved);

    return RawToZ85(data->data(), data->size());
}

std::string Context::RawToZ85(const void* input, const std::size_t size)
{
    if (0 != size % 4) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size.").Flush();

        return {};
    }

    const std::size_t outputSize = size + size / 4 + 1;
    std::vector<char> output{};
    output.resize(outputSize);
    auto encoded = ::zmq_z85_encode(
        output.data(), static_cast<const unsigned char*>(input), size);

    OT_ASSERT(nullptr != encoded);

    return {output.data(), output.size()};
}

OTData Context::Z85ToRaw(const void* input, const std::size_t size)
{
    if (0 != size % 5) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size.").Flush();

        return Data::Factory();
    }

    const std::size_t outputSize = size * 4 / 5;
    std::vector<std::uint8_t> output{};
    output.resize(outputSize);
    auto decoded =
        ::zmq_z85_decode(output.data(), static_cast<const char*>(input));

    OT_ASSERT(nullptr != decoded);

    return Data::Factory(output.data(), output.size());
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Context::Context()
    : context_(zmq_ctx_new())
{
    OT_ASSERT(nullptr != context_);
    OT_ASSERT(1 == zmq_has("curve"));
}

Context::operator void*() const
{
    OT_ASSERT(nullptr != context_)

    return context_;
}

std::string Context::BuildEndpoint(
    const std::string& path,
    const int instance,
    const int version) const
{
    return std::string(INPROC_PREFIX) + std::to_string(instance) +
           PATH_SEPERATOR + path + PATH_SEPERATOR + std::to_string(version);
}

std::string Context::BuildEndpoint(
    const std::string& path,
    const int instance,
    const int version,
    const std::string& suffix) const
{
    return BuildEndpoint(path, instance, version) + PATH_SEPERATOR + suffix;
}

Context* Context::clone() const { return new Context; }

OTZMQDealerSocket Context::DealerSocket(
    const ListenCallback& callback,
    const Socket::Direction direction) const
{
    return DealerSocket::Factory(*this, direction, callback);
}

OTZMQSubscribeSocket Context::PairEventListener(
    const PairEventCallback& callback,
    const int instance) const
{
    return OTZMQSubscribeSocket(
        new class PairEventListener(*this, callback, instance));
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback) const
{
    return PairSocket::Factory(*this, callback);
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback,
    const opentxs::network::zeromq::PairSocket& peer) const
{
    return PairSocket::Factory(callback, peer);
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback,
    const std::string& endpoint) const
{
    return PairSocket::Factory(*this, callback, endpoint);
}

OTZMQProxy Context::Proxy(
    network::zeromq::Socket& frontend,
    network::zeromq::Socket& backend) const
{
    return opentxs::network::zeromq::Proxy::Factory(*this, frontend, backend);
}

OTZMQPublishSocket Context::PublishSocket() const
{
    return PublishSocket::Factory(*this);
}

OTZMQPullSocket Context::PullSocket(const Socket::Direction direction) const
{
    return PullSocket::Factory(*this, direction);
}

OTZMQPullSocket Context::PullSocket(
    const ListenCallback& callback,
    const Socket::Direction direction) const
{
    return PullSocket::Factory(*this, direction, callback);
}

OTZMQPushSocket Context::PushSocket(const Socket::Direction direction) const
{
    return PushSocket::Factory(*this, direction);
}

OTZMQReplySocket Context::ReplySocket(
    const ReplyCallback& callback,
    const Socket::Direction direction) const
{
    return ReplySocket::Factory(*this, direction, callback);
}

OTZMQRequestSocket Context::RequestSocket() const
{
    return RequestSocket::Factory(*this);
}

OTZMQRouterSocket Context::RouterSocket(
    const ListenCallback& callback,
    const Socket::Direction direction) const
{
    return RouterSocket::Factory(*this, direction, callback);
}

OTZMQSubscribeSocket Context::SubscribeSocket(
    const ListenCallback& callback) const
{
    return SubscribeSocket::Factory(*this, callback);
}

Context::~Context()
{
    if (nullptr != context_) { zmq_ctx_shutdown(context_); }
}
}  // namespace opentxs::network::zeromq::implementation
