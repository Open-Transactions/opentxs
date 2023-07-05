// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/ConsensusPrivate.hpp"  // IWYU pragma: associated

#include <Context.pb.h>
#include <Signature.pb.h>
#include <memory>

#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::otx::context
{
ConsensusPrivate::ConsensusPrivate(const VersionNumber targetVersion) noexcept
    : available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(0)
    , acknowledged_request_numbers_()
    , local_nymbox_hash_()
    , remote_nymbox_hash_()
    , current_version_(targetVersion)
    , sig_()
{
}

ConsensusPrivate::ConsensusPrivate(
    const api::Session& api,
    const VersionNumber targetVersion,
    const proto::Context& serialized) noexcept
    : available_transaction_numbers_([&] {
        auto out = decltype(available_transaction_numbers_){};

        for (const auto& it : serialized.availabletransactionnumber()) {
            out.insert(it);
        }

        return out;
    }())
    , issued_transaction_numbers_([&] {
        auto out = decltype(issued_transaction_numbers_){};

        for (const auto& it : serialized.issuedtransactionnumber()) {
            out.insert(it);
        }

        return out;
    }())
    , request_number_(serialized.requestnumber())
    , acknowledged_request_numbers_([&] {
        auto out = decltype(acknowledged_request_numbers_){};

        for (const auto& it : serialized.acknowledgedrequestnumber()) {
            out.insert(it);
        }

        return out;
    }())
    , local_nymbox_hash_(
          api.Factory().IdentifierFromBase58(serialized.localnymboxhash()))
    , remote_nymbox_hash_(
          api.Factory().IdentifierFromBase58(serialized.remotenymboxhash()))
    , current_version_(targetVersion)
    , sig_([&]() -> decltype(sig_) {
        if (serialized.has_signature()) {
            return std::make_shared<proto::Signature>(serialized.signature());
        } else {
            return {};
        }
    }())
{
}
}  // namespace opentxs::otx::context
