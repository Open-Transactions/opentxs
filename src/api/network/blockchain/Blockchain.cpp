// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "opentxs/api/network/Blockchain.hpp"  // IWYU pragma: associated

#include <memory>

#include "api/network/blockchain/Base.hpp"
#include "api/network/blockchain/Blockchain.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/network/Factory.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"

namespace opentxs::factory
{
auto BlockchainNetworkAPINull() noexcept
    -> std::unique_ptr<api::network::Blockchain>
{
    using ReturnType = api::network::implementation::Blockchain;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>().release());
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
Blockchain::Blockchain(Imp* imp) noexcept
    : imp_(imp)
{
}

auto Blockchain::Disable(const Chain type) const noexcept -> bool
{
    return imp_->Disable(type);
}

auto Blockchain::Enable(const Chain type, const std::string_view seednode)
    const noexcept -> bool
{
    return imp_->Enable(type, seednode);
}

auto Blockchain::EnabledChains(alloc::Default alloc) const noexcept
    -> Set<Chain>
{
    return imp_->EnabledChains(alloc);
}

auto Blockchain::GetChain(const Chain type) const noexcept(false)
    -> BlockchainHandle
{
    return imp_->GetChain(type);
}

auto Blockchain::Internal() const noexcept -> const internal::Blockchain&
{
    return *imp_;
}

auto Blockchain::Internal() noexcept -> internal::Blockchain& { return *imp_; }

auto Blockchain::Profile() const noexcept -> BlockchainProfile
{
    return imp_->Profile();
}

auto Blockchain::Start(const Chain type, const std::string_view seednode)
    const noexcept -> bool
{
    return imp_->Start(type, seednode);
}

auto Blockchain::Stats() const noexcept -> opentxs::blockchain::node::Stats
{
    return imp_->Stats();
}

auto Blockchain::Stop(const Chain type) const noexcept -> bool
{
    return imp_->Stop(type);
}

Blockchain::~Blockchain()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network::implementation
