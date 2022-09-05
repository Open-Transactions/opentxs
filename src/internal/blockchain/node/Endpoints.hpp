// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node
{
struct Endpoints final : public Allocated {
    const CString block_cache_job_ready_publish_;
    const CString block_cache_pull_;
    const CString block_fetcher_job_ready_publish_;
    const CString block_fetcher_pull_;
    const CString block_oracle_pull_;
    const CString block_tip_publish_;
    const CString cfilter_downloader_pull_;
    const CString fee_oracle_pull_;
    const CString filter_oracle_reindex_publish_;
    const CString header_oracle_job_ready_;
    const CString header_oracle_pull_;
    const CString manager_pull_;
    const CString new_filter_publish_;
    const CString new_header_publish_;
    const CString otdht_pull_;
    const CString shutdown_publish_;
    const CString wallet_pull_;
    const CString wallet_to_accounts_push_;

    auto get_allocator() const noexcept -> allocator_type final
    {
        return block_cache_job_ready_publish_.get_allocator();
    }

    Endpoints(allocator_type alloc) noexcept;
    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&& rhs) noexcept;
    Endpoints(Endpoints&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Endpoints&) -> Endpoints& = delete;
    auto operator=(Endpoints&&) -> Endpoints& = delete;

    ~Endpoints() final = default;
};
}  // namespace opentxs::blockchain::node
