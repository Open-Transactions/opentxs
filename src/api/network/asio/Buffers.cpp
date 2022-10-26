// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "api/network/asio/Buffers.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>
#include <mutex>

#include "BoostAsio.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::api::network::asio
{
struct Buffers::Imp {
    auto clear(Index id) noexcept -> void
    {
        auto lock = Lock{lock_};
        buffers_.erase(id);
    }
    auto get(const std::size_t bytes) noexcept -> Handle
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
    UnallocatedMap<Index, Space> buffers_{};
};

Buffers::Buffers() noexcept
    : imp_(std::make_unique<Imp>().release())
{
}

auto Buffers::clear(Index id) noexcept -> void { imp_->clear(id); }

auto Buffers::get(const std::size_t bytes) noexcept -> Handle
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
