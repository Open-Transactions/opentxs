// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <memory>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Element;
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class ScriptForm
{
public:
    using Type = bitcoin::block::Script::Pattern;
    using Script = std::unique_ptr<const bitcoin::block::Script>;

    bool segwit_;
    Type primary_;
    Type secondary_;
    UnallocatedVector<ReadView> element_;
    Script script_;

    ScriptForm(
        const api::Session& api,
        const crypto::Element& input,
        blockchain::Type chain,
        Type primary,
        Type secondary) noexcept;
    ScriptForm(
        const api::Session& api,
        const crypto::Element& input,
        blockchain::Type chain,
        Type primary) noexcept;
    ScriptForm() = delete;
    ScriptForm(const ScriptForm&) = delete;
    ScriptForm(ScriptForm&& rhs) noexcept;
    auto operator=(ScriptForm&& rhs) noexcept -> ScriptForm&;
    auto operator=(const ScriptForm&) -> ScriptForm& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
