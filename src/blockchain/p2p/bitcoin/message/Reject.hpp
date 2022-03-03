// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Reject final : public implementation::Message
{
public:
    auto getMessage() const noexcept -> const UnallocatedCString&
    {
        return message_;
    }
    auto getRejectCode() const noexcept -> bitcoin::RejectCode { return code_; }
    auto getReason() const noexcept -> const UnallocatedCString&
    {
        return reason_;
    }
    auto getExtraData() const noexcept -> OTData
    {
        return Data::Factory(extra_);
    }

    Reject(
        const api::Session& api,
        const blockchain::Type network,
        const UnallocatedCString& message,
        const bitcoin::RejectCode code,
        const UnallocatedCString& reason,
        const Data& extra) noexcept;
    Reject(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const UnallocatedCString& message,
        const bitcoin::RejectCode code,
        const UnallocatedCString& reason,
        const Data& extra) noexcept(false);

    ~Reject() final = default;

private:
    const UnallocatedCString message_;
    const bitcoin::RejectCode code_{};
    const UnallocatedCString reason_;
    const OTData extra_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;

    Reject(const Reject&) = delete;
    Reject(Reject&&) = delete;
    auto operator=(const Reject&) -> Reject& = delete;
    auto operator=(Reject&&) -> Reject& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
