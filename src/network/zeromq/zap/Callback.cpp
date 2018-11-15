// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"

#include <atomic>

#include "Callback.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Callback>;

#define OT_METHOD "opentxs::network::zeromq::zap::implementation::Callback::"

namespace opentxs::network::zeromq::zap
{
OTZMQZAPCallback Callback::Factory(
    const std::string& domain,
    const ReceiveCallback& callback)
{
    auto output = OTZMQZAPCallback(new implementation::Callback());
    output->SetDomain(domain, callback);

    return output;
}

OTZMQZAPCallback Callback::Factory()
{
    return OTZMQZAPCallback(new implementation::Callback());
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
Callback::Callback()
    : default_callback_(
          std::bind(&Callback::default_callback, this, std::placeholders::_1))
    , domain_lock_()
    , policy_(Policy::Accept)
{
}

OTZMQZAPReply Callback::default_callback(const zap::Request& in) const
{
    auto output = Reply::Factory(in);

    if (Policy::Accept == policy_.load()) {
        output->SetCode(zap::Status::Success);
        output->SetStatus("OK");
    } else {
        output->SetCode(zap::Status::AuthFailure);
        output->SetStatus("Unsupported domain");
    }

    return output;
}

const Callback::Lambda& Callback::get_domain(const std::string& domain) const
{
    Lock lock(domain_lock_);

    try {

        return domains_.at(domain);
    } catch (...) {

        return default_callback_;
    }
}

OTZMQZAPReply Callback::Process(const zap::Request& request) const
{
    auto [valid, error] = request.Validate();

    if (false == valid) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Rejecting invalid request.")
            .Flush();

        return zap::Reply::Factory(request, Status::SystemError, error);
    }

    const auto& domain = get_domain(request.Domain());

    return domain(request);
}

bool Callback::SetDomain(
    const std::string& domain,
    const ReceiveCallback& callback) const
{
    Lock lock(domain_lock_);

    if (domain.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid domain.").Flush();

        return false;
    }

    if (0 < domains_.count(domain)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Domain ")(domain)(
            " already registered.")
            .Flush();

        return false;
    }

    return domains_.emplace(domain, callback).second;
}

bool Callback::SetPolicy(const Policy policy) const
{
    policy_.store(policy);

    return true;
}
}  // namespace opentxs::network::zeromq::zap::implementation
