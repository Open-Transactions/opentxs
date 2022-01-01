// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "api/network/asio/Buffers.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::api::network::asio
{
struct Buffers::Imp {
    auto clear(Index id) noexcept -> void
    {
        auto lock = Lock{lock_};
        buffers_.erase(id);
    }
    auto get(const std::size_t bytes) noexcept -> std::pair<Index, AsioBuffer>
    {
        auto lock = Lock{lock_};
        auto id = ++counter_;
        auto& buffer = buffers_[id];
        buffer.resize(bytes, std::byte{0x0});

        return {id, boost::asio::buffer(buffer.data(), buffer.size())};
    }

private:
    mutable std::mutex lock_{};
    Index counter_{-1};
    std::pmr::map<Index, Space> buffers_{};
};

Buffers::Buffers() noexcept
    : imp_(std::make_unique<Imp>().release())
{
}

auto Buffers::clear(Index id) noexcept -> void { imp_->clear(id); }

auto Buffers::get(const std::size_t bytes) noexcept
    -> std::pair<Index, AsioBuffer>
{
    return imp_->get(bytes);
}

Buffers::~Buffers()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network::asio
