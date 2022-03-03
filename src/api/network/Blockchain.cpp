// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "opentxs/api/network/Blockchain.hpp"  // IWYU pragma: associated

#include <memory>

#include "api/network/Blockchain.hpp"
#include "internal/api/network/Factory.hpp"

namespace opentxs::factory
{
auto BlockchainNetworkAPINull() noexcept -> api::network::Blockchain::Imp*
{
    using ReturnType = api::network::Blockchain;

    return std::make_unique<ReturnType::Imp>().release();
}
}  // namespace opentxs::factory

namespace opentxs::api::network
{
Blockchain::Blockchain(Imp* imp) noexcept
    : imp_(imp)
{
}

auto Blockchain::AddSyncServer(
    const UnallocatedCString& endpoint) const noexcept -> bool
{
    return imp_->AddSyncServer(endpoint);
}

auto Blockchain::ConnectedSyncServers() const noexcept -> Endpoints
{
    return imp_->ConnectedSyncServers();
}

auto Blockchain::DeleteSyncServer(
    const UnallocatedCString& endpoint) const noexcept -> bool
{
    return imp_->DeleteSyncServer(endpoint);
}

auto Blockchain::Disable(const Chain type) const noexcept -> bool
{
    return imp_->Disable(type);
}

auto Blockchain::Enable(const Chain type, const UnallocatedCString& seednode)
    const noexcept -> bool
{
    return imp_->Enable(type, seednode);
}

auto Blockchain::EnabledChains() const noexcept -> UnallocatedSet<Chain>
{
    return imp_->EnabledChains();
}

auto Blockchain::GetChain(const Chain type) const noexcept(false)
    -> const opentxs::blockchain::node::Manager&
{
    return imp_->GetChain(type);
}

auto Blockchain::GetSyncServers() const noexcept -> Endpoints
{
    return imp_->GetSyncServers();
}

auto Blockchain::Internal() const noexcept -> internal::Blockchain&
{
    return const_cast<Imp&>(*imp_);
}

auto Blockchain::Start(const Chain type, const UnallocatedCString& seednode)
    const noexcept -> bool
{
    return imp_->Start(type, seednode);
}

auto Blockchain::StartSyncServer(
    const UnallocatedCString& syncEndpoint,
    const UnallocatedCString& publicSyncEndpoint,
    const UnallocatedCString& updateEndpoint,
    const UnallocatedCString& publicUpdateEndpoint) const noexcept -> bool
{
    return imp_->StartSyncServer(
        syncEndpoint, publicSyncEndpoint, updateEndpoint, publicUpdateEndpoint);
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
}  // namespace opentxs::api::network
