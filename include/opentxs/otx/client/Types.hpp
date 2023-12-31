// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

class Message;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client
{
enum class Depositability : std::int8_t;         // IWYU pragma: export
enum class Messagability : std::int8_t;          // IWYU pragma: export
enum class PaymentType : int;                    // IWYU pragma: export
enum class PaymentWorkflowState : std::uint8_t;  // IWYU pragma: export
enum class PaymentWorkflowType : std::uint8_t;   // IWYU pragma: export
enum class RemoteBoxType;                        // IWYU pragma: export
enum class SendResult : std::int8_t;             // IWYU pragma: export
enum class StorageBox : std::uint8_t;            // IWYU pragma: export
enum class ThreadStatus : std::uint8_t;          // IWYU pragma: export

using NetworkOperationStatus = std::int32_t;
using NetworkReplyMessage = std::pair<SendResult, std::shared_ptr<Message>>;
using SetID = std::function<void(const identifier::Generic&)>;
}  // namespace opentxs::otx::client
