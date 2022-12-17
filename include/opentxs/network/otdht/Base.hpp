// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace otdht
{
class Acknowledgement;
class Data;
class PublishContract;
class PublishContractReply;
class PushTransaction;
class PushTransactionReply;
class Query;
class QueryContract;
class QueryContractReply;
class Request;
}  // namespace otdht

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT Base
{
public:
    class Imp;

    auto asAcknowledgement() const noexcept -> const Acknowledgement&;
    auto asData() const noexcept -> const Data&;
    auto asPublishContract() const noexcept -> const PublishContract&;
    auto asPublishContractReply() const noexcept -> const PublishContractReply&;
    auto asPushTransaction() const noexcept -> const PushTransaction&;
    auto asPushTransactionReply() const noexcept -> const PushTransactionReply&;
    auto asQuery() const noexcept -> const Query&;
    auto asQueryContract() const noexcept -> const QueryContract&;
    auto asQueryContractReply() const noexcept -> const QueryContractReply&;
    auto asRequest() const noexcept -> const Request&;

    auto Serialize(zeromq::Message& out) const noexcept -> bool;
    auto Type() const noexcept -> MessageType;
    auto Version() const noexcept -> VersionNumber;

    Base() noexcept;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    virtual ~Base();

protected:
    Imp* imp_;

    Base(Imp* imp) noexcept;
};
}  // namespace opentxs::network::otdht
