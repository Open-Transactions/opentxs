// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "network/zeromq/socket/Pipeline.hpp"  // IWYU pragma: associated

#include <iterator>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/message/Message.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/Types.internal.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"     // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Gatekeeper.hpp"

namespace opentxs::factory
{
auto Pipeline(
    const network::zeromq::Context& context,
    std::function<void(network::zeromq::Message&&)>&& callback,
    network::zeromq::socket::EndpointRequests subscribe,
    network::zeromq::socket::EndpointRequests pull,
    network::zeromq::socket::EndpointRequests dealer,
    network::zeromq::socket::SocketRequests extra,
    const std::string_view threadname,
    const std::optional<network::zeromq::BatchID>& preallocated,
    alloc::Strategy alloc) noexcept -> opentxs::network::zeromq::Pipeline
{
    return pmr::construct<network::zeromq::Pipeline::Imp>(
        alloc.result_,
        context,
        std::move(callback),
        subscribe,
        pull,
        dealer,
        extra,
        threadname,
        preallocated);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq
{
Pipeline::Imp::Imp(
    const zeromq::Context& context,
    Callback&& callback,
    socket::EndpointRequests subscribe,
    socket::EndpointRequests pull,
    socket::EndpointRequests dealer,
    socket::SocketRequests extra,
    const std::string_view threadname,
    const std::optional<zeromq::BatchID>& preallocated,
    allocator_type alloc) noexcept
    : Imp(context,
          std::move(callback),
          MakeArbitraryInproc(alloc),
          MakeArbitraryInproc(alloc),
          subscribe,
          pull,
          dealer,
          extra,
          threadname,
          preallocated,
          alloc)
{
}

Pipeline::Imp::Imp(
    const zeromq::Context& context,
    Callback&& callback,
    const CString internalEndpoint,
    const CString outgoingEndpoint,
    socket::EndpointRequests subscribe,
    socket::EndpointRequests pull,
    socket::EndpointRequests dealer,
    socket::SocketRequests extra,
    const std::string_view threadname,
    const std::optional<zeromq::BatchID>& preallocated,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , context_(context)
    , total_socket_count_(fixed_sockets_ + extra.get().size())
    , gate_()
    , shutdown_(false)
    , handle_([&] {
        auto sockets = [&] {
            auto out = Vector<socket::Type>{alloc};
            out.reserve(total_socket_count_);
            out.emplace_back(socket::Type::Subscribe);  // NOTE sub_
            out.emplace_back(socket::Type::Pull);       // NOTE pull_
            out.emplace_back(socket::Type::Pair);       // NOTE outgoing_
            out.emplace_back(socket::Type::Dealer);     // NOTE dealer_
            out.emplace_back(socket::Type::Pair);       // NOTE internal_

            for (const auto& [type, args, external] : extra.get()) {
                out.emplace_back(type);
            }

            assert_true(out.size() == total_socket_count_);

            return out;
        }();

        if (preallocated.has_value()) {

            return context_.Internal().MakeBatch(
                preallocated.value(), std::move(sockets), threadname);
        } else {

            return context_.Internal().MakeBatch(
                std::move(sockets), threadname);
        }
    }())
    , batch_([&]() -> auto& {
        auto& batch = handle_.batch_;
        batch.listen_callbacks_.emplace_back(
            ListenCallback::Factory(std::move(callback)));

        assert_true(batch.sockets_.size() == total_socket_count_);

        return batch;
    }())
    , sub_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(0);
        auto rc = socket.ClearSubscriptions();

        assert_true(rc);

        apply(subscribe, socket, alloc);

        return socket;
    }())
    , pull_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(1);
        apply(pull, socket, alloc);

        return socket;
    }())
    , outgoing_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(2);
        const auto rc = socket.Bind(outgoingEndpoint.c_str());

        assert_true(rc);

        return socket;
    }())
    , dealer_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(3);
        apply(dealer, socket, alloc);

        return socket;
    }())
    , internal_([&]() -> auto& {
        auto& socket = batch_.sockets_.at(4);
        const auto rc = socket.Bind(internalEndpoint.c_str());

        assert_true(rc);

        return socket;
    }())
    , to_dealer_([&] {
        auto socket = factory::ZMQSocket(context_, socket::Type::Pair);
        const auto rc = socket.Connect(outgoingEndpoint.c_str());

        assert_true(rc);

        return socket;
    }())
    , to_internal_([&] {
        auto socket = factory::ZMQSocket(context_, socket::Type::Pair);
        const auto rc = socket.Connect(internalEndpoint.c_str());

        assert_true(rc);

        return socket;
    }())
    , thread_(context_.Internal().Start(
          batch_.id_,
          [&] {
              auto out = StartArgs{
                  {outgoing_.ID(),
                   &outgoing_,
                   [id = outgoing_.ID(), socket = &dealer_](auto&& m) {
                       socket->SendDeferred(std::move(m));
                   }},
                  {internal_.ID(),
                   &internal_,
                   [id = internal_.ID(),
                    &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                       m.Internal().Prepend(id);
                       cb.Process(std::move(m));
                   }},
                  {dealer_.ID(),
                   &dealer_,
                   [id = dealer_.ID(),
                    &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                       m.Internal().Prepend(id);
                       cb.Process(std::move(m));
                   }},
                  {pull_.ID(),
                   &pull_,
                   [id = pull_.ID(),
                    &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                       m.Internal().Prepend(id);
                       cb.Process(std::move(m));
                   }},
                  {sub_.ID(),
                   &sub_,
                   [id = sub_.ID(),
                    &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                       m.Internal().Prepend(id);
                       cb.Process(std::move(m));
                   }},
              };

              assert_true(batch_.sockets_.size() == total_socket_count_);
              assert_true(
                  (fixed_sockets_ + extra.get().size()) == total_socket_count_);

              // NOTE adjust to the last fixed socket because the iterator will
              // be preincremented
              auto s = std::next(batch_.sockets_.begin(), fixed_sockets_ - 1_z);

              for (const auto& [type, policy, args] : extra.get()) {
                  auto& socket = *(++s);
                  using enum socket::Policy;

                  if (External == policy) {
                      const auto rc = socket.SetExposedUntrusted();

                      assert_true(rc);
                  }

                  apply(args, socket, alloc);
                  out.emplace_back(
                      socket.ID(),
                      &socket,
                      [id = socket.ID(),
                       &cb = batch_.listen_callbacks_.at(0).get()](auto&& m) {
                          m.Internal().Prepend(id);
                          cb.Process(std::move(m));
                      });
              }

              return out;
          }()))
    , extra_([&, this]() -> decltype(extra_) {
        if (auto count = batch_.sockets_.size(); count == fixed_sockets_) {

            return {};
        } else if (count > fixed_sockets_) {
            auto first = std::next(batch_.sockets_.begin(), fixed_sockets_);

            return {std::addressof(*first), count - fixed_sockets_};
        } else {
            LogAbort()().Abort();
        }
    }())
    , external_([&, this] {
        const auto span = extra.get();
        const auto count = span.size();

        assert_true(extra_.size() == count);

        auto out = decltype(external_){alloc};

        for (auto n = 0_uz; n < count; ++n) {
            const auto& [_1, policy, _2] = span[n];
            const auto& socket = extra_[n];
            using enum socket::Policy;

            if (External == policy) { out.emplace(socket.ID()); }
        }

        out.reserve(out.size());

        return out;
    }())
{
    assert_false(nullptr == thread_);
}

auto Pipeline::Imp::apply(
    const socket::EndpointRequests& endpoint,
    socket::Raw& socket,
    alloc::Strategy alloc) noexcept -> void
{
    for (const auto& [point, direction] : endpoint.get()) {
        const auto nullTerminated = CString{point, alloc.work_};

        if (socket::Direction::Connect == direction) {
            const auto rc = socket.Connect(nullTerminated.c_str());

            assert_true(rc);
        } else {
            const auto rc = socket.Bind(nullTerminated.c_str());

            assert_true(rc);
        }
    }
}

auto Pipeline::Imp::BatchID() const noexcept -> std::size_t
{
    return batch_.id_;
}

auto Pipeline::Imp::bind(
    SocketID id,
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    const auto done = gate_.get();

    if (done) { return false; }

    context_.Internal().Modify(
        id,
        [ep = CString{endpoint},
         extra = std::move(notify),
         &cb = batch_.listen_callbacks_.at(0).get()](auto& socket) {
            const auto value = socket.Bind(ep.c_str());

            if (extra) { cb.Process(extra(value)); }
        });

    return true;
}

auto Pipeline::Imp::BindSubscriber(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return bind(sub_.ID(), endpoint, std::move(notify));
}

auto Pipeline::Imp::Close() const noexcept -> bool
{
    gate_.shutdown();

    if (auto sent = shutdown_.exchange(true); false == sent) {
        handle_.Release();
    }

    return true;
}

auto Pipeline::Imp::connect(
    SocketID id,
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    const auto done = gate_.get();

    if (done) { return false; }

    context_.Internal().Modify(
        id,
        [ep = CString{endpoint},
         extra = std::move(notify),
         &cb = batch_.listen_callbacks_.at(0).get()](auto& socket) {
            const auto value = socket.Connect(ep.c_str());

            if (extra) { cb.Process(extra(value)); }
        });

    return true;
}

auto Pipeline::Imp::ConnectDealer(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return connect(dealer_.ID(), endpoint, std::move(notify));
}

auto Pipeline::Imp::ConnectionIDDealer() const noexcept -> std::size_t
{
    return dealer_.ID();
}

auto Pipeline::Imp::ConnectionIDInternal() const noexcept -> std::size_t
{
    return internal_.ID();
}

auto Pipeline::Imp::ConnectionIDPull() const noexcept -> std::size_t
{
    return pull_.ID();
}

auto Pipeline::Imp::ConnectionIDSubscribe() const noexcept -> std::size_t
{
    return sub_.ID();
}

auto Pipeline::Imp::ExtraSocket(std::size_t index) noexcept(false)
    -> socket::Raw&
{
    if (index >= extra_.size()) {

        throw std::out_of_range{"invalid extra socket index"};
    }

    return extra_[index];
}

auto Pipeline::Imp::PullFrom(const std::string_view endpoint) const noexcept
    -> bool
{
    return connect(pull_.ID(), endpoint);
}

auto Pipeline::Imp::PullFromThread(std::string_view endpoint) noexcept -> bool
{
    return pull_.Connect(endpoint.data());
}

auto Pipeline::Imp::Push(zeromq::Message&& msg) const noexcept -> bool
{
    const auto done = gate_.get();

    if (done) { return false; }

    to_internal_.modify_detach([data = std::move(msg)](auto& socket) mutable {
        socket.SendDeferred(std::move(data));
    });

    return true;
}

auto Pipeline::Imp::Send(zeromq::Message&& msg) const noexcept -> bool
{
    const auto done = gate_.get();

    if (done) { return false; }

    to_dealer_.modify_detach([data = std::move(msg)](auto& socket) mutable {
        socket.SendDeferred(std::move(data));
    });

    return true;
}

auto Pipeline::Imp::SendFromThread(zeromq::Message&& msg) noexcept -> bool
{
    return dealer_.SendDeferred(std::move(msg));
}

auto Pipeline::Imp::SetCallback(Callback&& cb) const noexcept -> void
{
    auto& listener =
        const_cast<ListenCallback&>(batch_.listen_callbacks_.at(0).get());
    listener.Replace(std::move(cb));
}

auto Pipeline::Imp::SubscribeFromThread(std::string_view endpoint) noexcept
    -> bool
{
    return sub_.Connect(endpoint.data());
}

auto Pipeline::Imp::SubscribeTo(const std::string_view endpoint) const noexcept
    -> bool
{
    return connect(sub_.ID(), endpoint);
}

Pipeline::Imp::~Imp() { Close(); }
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Pipeline::Pipeline(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Pipeline::Pipeline(Pipeline&& rhs) noexcept
    : Pipeline(std::exchange(rhs.imp_, nullptr))
{
}

auto Pipeline::BatchID() const noexcept -> std::size_t
{
    return imp_->BatchID();
}

auto Pipeline::BindSubscriber(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return imp_->BindSubscriber(endpoint, std::move(notify));
}

auto Pipeline::Close() const noexcept -> bool { return imp_->Close(); }

auto Pipeline::ConnectDealer(
    const std::string_view endpoint,
    std::function<Message(bool)> notify) const noexcept -> bool
{
    return imp_->ConnectDealer(endpoint, std::move(notify));
}

auto Pipeline::ConnectionIDDealer() const noexcept -> std::size_t
{
    return imp_->ConnectionIDDealer();
}

auto Pipeline::ConnectionIDInternal() const noexcept -> std::size_t
{
    return imp_->ConnectionIDInternal();
}

auto Pipeline::ConnectionIDPull() const noexcept -> std::size_t
{
    return imp_->ConnectionIDPull();
}

auto Pipeline::ConnectionIDSubscribe() const noexcept -> std::size_t
{
    return imp_->ConnectionIDSubscribe();
}

auto Pipeline::get_allocator() const noexcept -> alloc::Default
{
    return imp_->get_allocator();
}

auto Pipeline::Internal() const noexcept -> const internal::Pipeline&
{
    return *imp_;
}

auto Pipeline::Internal() noexcept -> internal::Pipeline& { return *imp_; }

auto Pipeline::PullFrom(const std::string_view endpoint) const noexcept -> bool
{
    return imp_->PullFrom(endpoint);
}

auto Pipeline::Push(Message&& msg) const noexcept -> bool
{
    return imp_->Push(std::move(msg));
}

auto Pipeline::Send(Message&& msg) const noexcept -> bool
{
    return imp_->Send(std::move(msg));
}

auto Pipeline::SubscribeTo(const std::string_view endpoint) const noexcept
    -> bool
{
    return imp_->SubscribeTo(endpoint);
}

Pipeline::~Pipeline() { pmr::destroy(imp_); }
}  // namespace opentxs::network::zeromq
