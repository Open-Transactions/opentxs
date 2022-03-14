// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>

#include "opentxs/util/Options.hpp"

namespace ottest
{
auto check_options(
    const opentxs::Options& test,
    const OptionsData& expect) noexcept -> bool
{
    auto output{true};
    output &= test.BlockchainBindIpv4() == expect.blockchain_bind_ipv4_;
    output &= test.BlockchainBindIpv6() == expect.blockchain_bind_ipv6_;
    output &= test.BlockchainStorageLevel() == expect.blockchain_storage_level_;
    output &=
        test.BlockchainWalletEnabled() == expect.blockchain_wallet_enabled_;
    output &= test.DisabledBlockchains() == expect.blockchain_disabled_chains_;
    output &= test.Home() == expect.home_;
    output &= test.Ipv4ConnectionMode() == expect.ipv4_connection_mode_;
    output &= test.Ipv6ConnectionMode() == expect.ipv6_connection_mode_;
    output &= test.LogLevel() == expect.log_level_;
    output &= test.NotaryBindIP() == expect.notary_bind_ip_;
    output &= test.NotaryBindPort() == expect.notary_bind_port_;
    output &= test.NotaryInproc() == expect.notary_bind_inproc_;
    output &= test.NotaryName() == expect.notary_name_;
    output &= test.NotaryPublicEEP() == expect.notary_public_eep_;
    output &= test.NotaryPublicIPv4() == expect.notary_public_ipv4_;
    output &= test.NotaryPublicIPv6() == expect.notary_public_ipv6_;
    output &= test.NotaryPublicOnion() == expect.notary_public_onion_;
    output &= test.NotaryPublicPort() == expect.notary_public_port_;
    output &= test.NotaryTerms() == expect.notary_terms_;
    output &= test.ProvideBlockchainSyncServer() ==
              expect.blockchain_sync_server_enabled_;
    output &=
        test.RemoteBlockchainSyncServers() == expect.blockchain_sync_servers_;
    output &= test.RemoteLogEndpoint() == expect.log_endpoint_;
    output &= test.StoragePrimaryPlugin() == expect.storage_primary_plugin_;

    EXPECT_EQ(test.BlockchainBindIpv4(), expect.blockchain_bind_ipv4_);
    EXPECT_EQ(test.BlockchainBindIpv6(), expect.blockchain_bind_ipv6_);
    EXPECT_EQ(test.BlockchainStorageLevel(), expect.blockchain_storage_level_);
    EXPECT_EQ(
        test.BlockchainWalletEnabled(), expect.blockchain_wallet_enabled_);
    EXPECT_EQ(test.DisabledBlockchains(), expect.blockchain_disabled_chains_);
    EXPECT_EQ(test.Home(), expect.home_);
    EXPECT_EQ(test.Ipv4ConnectionMode(), expect.ipv4_connection_mode_);
    EXPECT_EQ(test.Ipv6ConnectionMode(), expect.ipv6_connection_mode_);
    EXPECT_EQ(test.LogLevel(), expect.log_level_);
    EXPECT_EQ(test.NotaryBindIP(), expect.notary_bind_ip_);
    EXPECT_EQ(test.NotaryBindPort(), expect.notary_bind_port_);
    EXPECT_EQ(test.NotaryInproc(), expect.notary_bind_inproc_);
    EXPECT_EQ(test.NotaryName(), expect.notary_name_);
    EXPECT_EQ(test.NotaryPublicEEP(), expect.notary_public_eep_);
    EXPECT_EQ(test.NotaryPublicIPv4(), expect.notary_public_ipv4_);
    EXPECT_EQ(test.NotaryPublicIPv6(), expect.notary_public_ipv6_);
    EXPECT_EQ(test.NotaryPublicOnion(), expect.notary_public_onion_);
    EXPECT_EQ(test.NotaryPublicPort(), expect.notary_public_port_);
    EXPECT_EQ(test.NotaryTerms(), expect.notary_terms_);
    EXPECT_EQ(
        test.ProvideBlockchainSyncServer(),
        expect.blockchain_sync_server_enabled_);
    EXPECT_EQ(
        test.RemoteBlockchainSyncServers(), expect.blockchain_sync_servers_);
    EXPECT_EQ(test.RemoteLogEndpoint(), expect.log_endpoint_);
    EXPECT_EQ(test.StoragePrimaryPlugin(), expect.storage_primary_plugin_);

    return output;
}
}  // namespace ottest
