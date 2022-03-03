// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class PeerObject;
}  // namespace proto

class PeerObject;

using OTPeerObject = Pimpl<PeerObject>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT PeerObject
{
public:
    virtual auto Message() const noexcept
        -> const std::unique_ptr<UnallocatedCString>& = 0;
    virtual auto Nym() const noexcept -> const Nym_p& = 0;
    virtual auto Payment() const noexcept
        -> const std::unique_ptr<UnallocatedCString>& = 0;
    virtual auto Purse() const noexcept -> const otx::blind::Purse& = 0;
    virtual auto Request() const noexcept -> const OTPeerRequest = 0;
    virtual auto Reply() const noexcept -> const OTPeerReply = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::PeerObject&) const noexcept
        -> bool = 0;
    virtual auto Type() const noexcept -> contract::peer::PeerObjectType = 0;
    virtual auto Validate() const noexcept -> bool = 0;

    virtual auto Message() noexcept -> std::unique_ptr<UnallocatedCString>& = 0;
    virtual auto Payment() noexcept -> std::unique_ptr<UnallocatedCString>& = 0;
    virtual auto Purse() noexcept -> otx::blind::Purse& = 0;

    virtual ~PeerObject() = default;

protected:
    PeerObject() = default;

private:
    PeerObject(const PeerObject&) = delete;
    PeerObject(PeerObject&&) = delete;
    auto operator=(const PeerObject&) noexcept -> PeerObject& = delete;
    auto operator=(PeerObject&&) noexcept -> PeerObject& = delete;
};
}  // namespace opentxs
