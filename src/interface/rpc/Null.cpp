// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <RPCResponse.pb.h>

#include "opentxs/interface/rpc/response/Base.hpp"  // IWYU pragma: keep
#include "opentxs/internal.factory.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
}  // namespace api
}  // namespace opentxs

namespace opentxs
{
auto Factory::RPC(const api::Context& api) -> rpc::internal::RPC*
{
    struct Blank final : public rpc::internal::RPC {
        auto Process(const proto::RPCCommand& command) const
            -> proto::RPCResponse final
        {
            return {};
        }
        auto Process(const rpc::request::Base& command) const
            -> std::unique_ptr<rpc::response::Base> final
        {
            return {};
        }

        ~Blank() final = default;
    };

    return std::make_unique<Blank>().release();
}
}  // namespace opentxs
