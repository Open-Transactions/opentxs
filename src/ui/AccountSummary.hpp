// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using AccountSummaryList = List<
    AccountSummaryExternalInterface,
    AccountSummaryInternalInterface,
    AccountSummaryRowID,
    AccountSummaryRowInterface,
    AccountSummaryRowInternal,
    AccountSummaryRowBlank,
    AccountSummarySortKey>;

class AccountSummary final : public AccountSummaryList
{
public:
    proto::ContactItemType Currency() const override { return currency_; }
    const identifier::Nym& NymID() const override { return nym_id_; }

    ~AccountSummary();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    const proto::ContactItemType currency_;
    std::set<OTIdentifier> issuers_;
    std::map<OTIdentifier, OTIdentifier> server_issuer_map_;
    std::map<OTIdentifier, OTIdentifier> nym_server_map_;

    void construct_row(
        const AccountSummaryRowID& id,
        const AccountSummarySortKey& index,
        const CustomData& custom) const override;

    AccountSummarySortKey extract_key(
        const identifier::Nym& nymID,
        const Identifier& issuerID);
    void process_connection(const network::zeromq::Message& message);
    void process_issuer(const Identifier& issuerID);
    void process_issuer(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void process_server(const network::zeromq::Message& message);
    void process_server(const OTIdentifier& serverID);
    void startup();

    AccountSummary(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType currency);
    AccountSummary() = delete;
    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    AccountSummary& operator=(const AccountSummary&) = delete;
    AccountSummary& operator=(AccountSummary&&) = delete;
};
}  // namespace opentxs::ui::implementation
