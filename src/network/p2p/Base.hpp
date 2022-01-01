// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/p2p/MessageType.hpp"

#pragma once

#include "opentxs/network/p2p/Base.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
namespace network
{
namespace p2p
{
class Block;
class State;
}  // namespace p2p

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::p2p
{
class Base::Imp
{
public:
    using LocalType = MessageType;
    using RemoteType = WorkType;

    static constexpr auto default_version_ = VersionNumber{1};
    static constexpr auto hello_version_ = VersionNumber{1};

    const VersionNumber version_;
    const MessageType type_;
    const std::pmr::vector<State> state_;
    const std::string endpoint_;
    const std::pmr::vector<Block> blocks_;

    static auto translate(const LocalType in) noexcept -> RemoteType;
    static auto translate(const RemoteType in) noexcept -> LocalType;

    virtual auto asAcknowledgement() const noexcept -> const Acknowledgement&;
    virtual auto asPublishContract() const noexcept -> const PublishContract&;
    virtual auto asPublishContractReply() const noexcept
        -> const PublishContractReply&;
    virtual auto asData() const noexcept -> const Data&;
    virtual auto asQuery() const noexcept -> const Query&;
    virtual auto asQueryContract() const noexcept -> const QueryContract&;
    virtual auto asQueryContractReply() const noexcept
        -> const QueryContractReply&;
    virtual auto asRequest() const noexcept -> const Request&;
    virtual auto serialize(zeromq::Message& out) const noexcept -> bool;
    auto serialize_type(zeromq::Message& out) const noexcept -> bool;

    Imp(VersionNumber version,
        MessageType type,
        std::pmr::vector<State> state,
        std::string endpoint,
        std::pmr::vector<Block> blocks) noexcept;
    Imp(const MessageType type) noexcept;
    Imp() noexcept;

    virtual ~Imp() = default;

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::network::p2p
