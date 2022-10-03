// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <functional>
#include <future>

#include "opentxs/Export.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
class Asio;
}  // namespace internal
}  // namespace network
}  // namespace api

namespace network
{
namespace asio
{
class Endpoint;
class Socket;
}  // namespace asio
}  // namespace network

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
/**
 The api::network::Asio API contains functions used for networking via
 Boost::ASIO.
 */
class OPENTXS_EXPORT Asio
{
public:
    virtual auto GetPublicAddress4() const noexcept
        -> std::shared_future<ByteArray> = 0;
    virtual auto GetPublicAddress6() const noexcept
        -> std::shared_future<ByteArray> = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Asio& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Asio& = 0;

    Asio(const Asio&) = delete;
    Asio(Asio&&) = delete;
    auto operator=(const Asio&) -> Asio& = delete;
    auto operator=(Asio&&) -> Asio& = delete;

    OPENTXS_NO_EXPORT virtual ~Asio() = default;

protected:
    Asio() = default;
};
}  // namespace opentxs::api::network
