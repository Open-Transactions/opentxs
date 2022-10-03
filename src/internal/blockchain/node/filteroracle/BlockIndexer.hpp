// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <string_view>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
namespace filteroracle
{
class Shared;
}  // namespace filteroracle

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
class BlockIndexer
{
public:
    auto Start() noexcept -> void;

    BlockIndexer(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        std::shared_ptr<Shared> shared) noexcept;
    BlockIndexer(const BlockIndexer&) = delete;
    BlockIndexer(BlockIndexer&&) = delete;
    auto operator=(const BlockIndexer&) -> BlockIndexer& = delete;
    auto operator=(BlockIndexer&&) -> BlockIndexer& = delete;

    ~BlockIndexer();

private:
    class Imp;

    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::filteroracle
