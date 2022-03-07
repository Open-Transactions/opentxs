// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#pragma once

#include <gmock/gmock.h>

#include "opentxs/api/session/Storage.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/Bip47Channel.pb.h"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Contact.pb.h"
#include "serialization/protobuf/Context.pb.h"
#include "serialization/protobuf/Credential.pb.h"
#include "serialization/protobuf/HDAccount.pb.h"
#include "serialization/protobuf/Issuer.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/PeerReply.pb.h"
#include "serialization/protobuf/PeerRequest.pb.h"
#include "serialization/protobuf/Purse.pb.h"
#include "serialization/protobuf/Seed.pb.h"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/StorageThread.pb.h"
#include "serialization/protobuf/StorageThreadItem.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"

namespace opentxs::api::session::internal
{
    class Storage : virtual public session::Storage
    {

    };
}

class MStoreage
{
public:
    bool get() { return false; }
};

class StorageMock : virtual public opentxs::api::session::internal::Storage
{
public:
    MOCK_METHOD(
        opentxs::UnallocatedCString,
        AccountAlias,
        (const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::ObjectList,
        AccountList,
        (),
        (const, final));
    MOCK_METHOD(
        opentxs::OTUnitID,
        AccountContract,
        (const opentxs::Identifier& accountID),
        (const, final));
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ());
    MOCK_METHOD(
        opentxs::OTNymID,
        AccountIssuer,
        (const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::OTNymID,
        AccountOwner,
        (const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::OTNotaryID,
        AccountServer,
        (const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::OTNymID,
        AccountSigner,
        (const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnitType,
        AccountUnit,
        (const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::OTIdentifier>,
        AccountsByContract,
        (const opentxs::identifier::UnitDefinition&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::OTIdentifier>,
        AccountsByIssuer,
        (const opentxs::identifier::Nym&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::OTIdentifier>,
        AccountsByOwner,
        (const opentxs::identifier::Nym&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::OTIdentifier>,
        AccountsByServer,
        (const opentxs::identifier::Notary&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::OTIdentifier>,
        AccountsByUnit,
        (const opentxs::UnitType),
        (const, final));
    MOCK_METHOD(
        opentxs::UnitType,
        Bip47Chain,
        (const opentxs::identifier::Nym&, const opentxs::Identifier&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::OTIdentifier>,
        Bip47ChannelsByChain,
        (const opentxs::identifier::Nym&, const opentxs::UnitType),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::UnallocatedCString>,
        BlockchainAccountList,
        (const opentxs::UnallocatedCString&, const opentxs::UnitType),
        (const, final));
    MOCK_METHOD(
        opentxs::UnitType,
        BlockchainAccountType,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedVector<opentxs::OTIdentifier>,
        BlockchainThreadMap,
        (const opentxs::identifier::Nym&, const opentxs::Data&),
        (const, noexcept, final));
    MOCK_METHOD(
        opentxs::UnallocatedVector<opentxs::OTData>,
        BlockchainTransactionList,
        (const opentxs::identifier::Nym&),
        (const, noexcept, final));
    MOCK_METHOD(
        bool,
        CheckTokenSpent,
        (const opentxs::identifier::Notary&, const opentxs::identifier::UnitDefinition&, const std::uint64_t, const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedCString,
        ContactAlias,
        (const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        opentxs::ObjectList,
        ContactList,
        (),
        (const, final));
    MOCK_METHOD(
        opentxs::ObjectList,
        ContextList,
        (const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedCString,
        ContactOwnerNym,
        (const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        void,
        ContactSaveIndices,
        (),
        (const, final));
    MOCK_METHOD(
        opentxs::VersionNumber,
        ContactUpgradeLevel,
        (),
        (const, final));
    MOCK_METHOD(
        bool,
        CreateThread,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedSet<opentxs::UnallocatedCString>&),
        (const, final));
    MOCK_METHOD(
        bool,
        DeleteAccount,
        (const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        opentxs::OTNymID,
        DefaultNym,
        (),
        (const, final));
    MOCK_METHOD(
        opentxs::UnallocatedCString,
        DefaultSeed,
        (),
        (const, final));
    MOCK_METHOD(
        bool,
        DeleteContact,
        (const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        bool,
        DeletePaymentWorkflow,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));
    MOCK_METHOD(
        std::uint32_t,
        HashType,
        (),
        (const, final));

    auto Internal() const noexcept -> const opentxs::api::session::internal::Storage& final
    {
        return *this;
    }

    auto Internal() noexcept -> opentxs::api::session::internal::Storage& final
    {
        return *this;
    }

    MOCK_METHOD(
        opentxs::ObjectList,
        IssuerList,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        LoadHelper,
        (const opentxs::UnallocatedCString&, opentxs::UnallocatedCString&, opentxs::UnallocatedCString&, const bool),
        (const));

    auto Load(
        const opentxs::UnallocatedCString& accountID,
        opentxs::UnallocatedCString& output,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return LoadHelper(accountID, output, alias, checking);
    }

    MOCK_METHOD(
        bool,
        LoadHelper,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, opentxs::proto::HDAccount&, const bool checking),
        (const));

    auto Load(
        const opentxs::UnallocatedCString& nymID,
        const opentxs::UnallocatedCString& accountID,
        opentxs::proto::HDAccount& output,
        const bool checking = false) const -> bool final
    {
        return LoadHelper(nymID, accountID, output, checking);
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ())
    auto Load(
        const opentxs::identifier::Nym& nymID,
        const opentxs::Identifier& channelID,
        opentxs::proto::Bip47Channel& output,
        const bool checking = false) const -> bool final
    {
        return false;
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ())
    auto Load(
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Contact& contact,
        const bool checking = false) const -> bool final
    {
        return false;
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ())
    auto Load(
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Contact& contact,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return false;
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ())
    auto Load(
        const opentxs::UnallocatedCString& nym,
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Context& context,
        const bool checking = false) const -> bool final
    {
        return false;
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ())
    auto Load(
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Credential& cred,
        const bool checking = false) const -> bool final
    {
        return false;
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ())
    auto Load(
        const opentxs::identifier::Nym& id,
        opentxs::proto::Nym& nym,
        const bool checking = false) const -> bool final
    {
        return false;
    }
////    MOCK_METHOD(
////        ,
////        ,
////        (),
////        ());
    auto Load(
        const opentxs::identifier::Nym& id,
        opentxs::proto::Nym& nym,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    MOCK_METHOD(
        bool,
        LoadNymHelper,
        (const opentxs::identifier::Nym&, opentxs::AllocateOutput, const bool),
        (const));

    virtual auto LoadNym(
        const opentxs::identifier::Nym& id,
        opentxs::AllocateOutput destination,
        const bool checking = false) const -> bool final
    {
        return LoadNymHelper(id, destination, checking);
    }

    virtual auto Load(
        const opentxs::UnallocatedCString& nymID,
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Issuer& issuer,
        const bool checking = false) const -> bool final
    {
        return false;
    }
    virtual auto Load(
        const opentxs::UnallocatedCString& nymID,
        const opentxs::UnallocatedCString& workflowID,
        opentxs::proto::PaymentWorkflow& workflow,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    MOCK_METHOD(
        bool,
        LoadHelper,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::StorageBox, opentxs::UnallocatedCString&, opentxs::UnallocatedCString&, const bool),
        (const));

    virtual auto Load(
        const opentxs::UnallocatedCString& nymID,
        const opentxs::UnallocatedCString& id,
        const opentxs::StorageBox box,
        opentxs::UnallocatedCString& output,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return LoadHelper(nymID, id, box, output, alias, checking);
    }

    virtual auto Load(
        const opentxs::UnallocatedCString& nymID,
        const opentxs::UnallocatedCString& id,
        const opentxs::StorageBox box,
        opentxs::proto::PeerReply& request,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::UnallocatedCString& nymID,
        const opentxs::UnallocatedCString& id,
        const opentxs::StorageBox box,
        opentxs::proto::PeerRequest& request,
        std::time_t& time,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::identifier::Nym& nym,
        const opentxs::identifier::Notary& notary,
        const opentxs::identifier::UnitDefinition& unit,
        opentxs::proto::Purse& output,
        const bool checking) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Seed& seed,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::UnallocatedCString& id,
        opentxs::proto::Seed& seed,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::identifier::Notary& id,
        opentxs::proto::ServerContract& contract,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::identifier::Notary& id,
        opentxs::proto::ServerContract& contract,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::UnallocatedCString& nymId,
        const opentxs::UnallocatedCString& threadId,
        opentxs::proto::StorageThread& thread) const -> bool final
    {
        return false;
    }

    virtual auto Load(opentxs::proto::Ciphertext& output, const bool checking = false)
        const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::identifier::UnitDefinition& id,
        opentxs::proto::UnitDefinition& contract,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    virtual auto Load(
        const opentxs::identifier::UnitDefinition& id,
        opentxs::proto::UnitDefinition& contract,
        opentxs::UnallocatedCString& alias,
        const bool checking = false) const -> bool final
    {
        return false;
    }

    MOCK_METHOD(
        const opentxs::UnallocatedSet<opentxs::UnallocatedCString>,
        LocalNyms,
        (),
        (const, final));

    MOCK_METHOD(
        void,
        MapPublicNyms,
        (opentxs::NymLambda&),
        (const, final));

    MOCK_METHOD(
        void,
        MapServers,
        (opentxs::ServerLambda&),
        (const, final));

    MOCK_METHOD(
        void,
        MapUnitDefinitions,
        (opentxs::UnitLambda&),
        (const, final));

    MOCK_METHOD(
        bool,
        MarkTokenSpent,
        (const opentxs::identifier::Notary&, const opentxs::identifier::UnitDefinition&, const std::uint64_t, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        MoveThreadItem,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        opentxs::ObjectList,
        NymBoxList,
        (const opentxs::UnallocatedCString&, const opentxs::StorageBox box),
        (const, final));

    MOCK_METHOD(
        opentxs::ObjectList,
        NymList,
        (),
        (const, final));

    MOCK_METHOD(
        opentxs::ObjectList,
        PaymentWorkflowList,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        opentxs::UnallocatedCString,
        PaymentWorkflowLookup,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::UnallocatedCString>,
        PaymentWorkflowsByAccount,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        opentxs::UnallocatedSet<opentxs::UnallocatedCString>,
        PaymentWorkflowsByState,
        (const opentxs::UnallocatedCString&, const opentxs::otx::client::PaymentWorkflowType, const opentxs::otx::client::PaymentWorkflowState),
        (const, final));

    MOCK_METHOD(
        (opentxs::UnallocatedSet<opentxs::UnallocatedCString>),
        PaymentWorkflowsByUnit,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        (std::pair<opentxs::otx::client::PaymentWorkflowType, opentxs::otx::client::PaymentWorkflowState>),
        PaymentWorkflowState,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        RelabelThread,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        RemoveBlockchainThreadItem,
        (const opentxs::identifier::Nym&, const opentxs::Identifier&, const opentxs::blockchain::Type, const opentxs::Data&),
        (const, noexcept, final));

    MOCK_METHOD(
        bool,
        RemoveNymBoxItem,
        (const opentxs::UnallocatedCString&, const opentxs::StorageBox, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        RemoveServer,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        RemoveThreadItem,
        (const opentxs::identifier::Nym&, const opentxs::Identifier&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        RemoveUnitDefinition,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        RenameThread,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        void,
        RunGC,
        (),
        (const, final));

    MOCK_METHOD(
        opentxs::UnallocatedCString,
        ServerAlias,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        opentxs::ObjectList,
        ServerList,
        (),
        (const, final));

    MOCK_METHOD(
        opentxs::ObjectList,
        SeedList,
        (),
        (const, final));

    MOCK_METHOD(
        bool,
        SetAccountAlias,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetContactAlias,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetDefaultNym,
        (const opentxs::identifier::Nym&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetDefaultSeed,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetNymAlias,
        (const opentxs::identifier::Nym&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetPeerRequestTime,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::StorageBox),
        (const, final));

    MOCK_METHOD(
        bool,
        SetReadState,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const bool unread),
        (const, final));

    MOCK_METHOD(
        bool,
        SetSeedAlias,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetServerAlias,
        (const opentxs::identifier::Notary&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetThreadAlias,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        SetUnitDefinitionAlias,
        (const opentxs::identifier::UnitDefinition&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::identifier::Nym&, const opentxs::identifier::Nym&, const opentxs::identifier::Nym&, const opentxs::identifier::Notary&, const opentxs::identifier::UnitDefinition&, const opentxs::UnitType),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::UnallocatedCString&, const opentxs::identity::wot::claim::ClaimType, const opentxs::proto::HDAccount&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::identifier::Nym&, const opentxs::Identifier&, const opentxs::proto::Bip47Channel&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::Contact&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::Context&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::Credential&),
        (const, final));

    MOCK_METHOD(
        bool,
        StoreHelper,
        (const opentxs::proto::Nym& data, const opentxs::UnallocatedCString&),
        (const));

    virtual auto Store(
        const opentxs::proto::Nym& data,
        const opentxs::UnallocatedCString& alias = {}) const -> bool final
    {
        return StoreHelper(data, alias);
    };

    MOCK_METHOD(
        bool,
        StoreHelper,
        (const opentxs::ReadView&, const opentxs::UnallocatedCString&),
        (const));

    virtual auto Store(
        const opentxs::ReadView& data,
        const opentxs::UnallocatedCString& alias = {}) const -> bool final
    {
        return StoreHelper(data, alias);
    };

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::UnallocatedCString&, const opentxs::proto::Issuer&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::UnallocatedCString&, const opentxs::proto::PaymentWorkflow&),
        (const, final));

    MOCK_METHOD(
        bool,
        StoreHelper,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const std::uint64_t, const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&, const opentxs::StorageBox, const opentxs::UnallocatedCString&),
        (const));

    virtual auto Store(
        const opentxs::UnallocatedCString& nymid,
        const opentxs::UnallocatedCString& threadid,
        const opentxs::UnallocatedCString& itemid,
        const std::uint64_t time,
        const opentxs::UnallocatedCString& alias,
        const opentxs::UnallocatedCString& data,
        const opentxs::StorageBox box,
        const opentxs::UnallocatedCString& account = {}) const -> bool final
    {
        return StoreHelper(nymid, threadid, itemid, time, alias, data, box, account);
    };

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::identifier::Nym&, const opentxs::Identifier&, const opentxs::blockchain::Type, const opentxs::Data&, const opentxs::Time),
        (const, noexcept, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::PeerReply&, const opentxs::UnallocatedCString&, const opentxs::StorageBox),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::PeerRequest&, const opentxs::UnallocatedCString&, const opentxs::StorageBox),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::identifier::Nym&, const opentxs::proto::Purse&),
        (const, final));

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::Seed&),
        (const, final));

    MOCK_METHOD(
        bool,
        StoreHelper,
        (const opentxs::proto::ServerContract&, const opentxs::UnallocatedCString&),
        (const));

    virtual auto Store(
        const opentxs::proto::ServerContract& data,
        const opentxs::UnallocatedCString& alias = {}) const -> bool final
    {
        return StoreHelper(data, alias);
    };

    MOCK_METHOD(
        bool,
        Store,
        (const opentxs::proto::Ciphertext&),
        (const, final));

    MOCK_METHOD(
        bool,
        StoreHelper,
        (const opentxs::proto::UnitDefinition& data, const opentxs::UnallocatedCString&),
        (const));

    virtual auto Store(
        const opentxs::proto::UnitDefinition& data,
        const opentxs::UnallocatedCString& alias = {}) const -> bool final
    {
        return StoreHelper(data, alias);
    }

    MOCK_METHOD(
        opentxs::ObjectList,
        ThreadList,
        (const opentxs::UnallocatedCString&, const bool),
        (const, final));

    MOCK_METHOD(
        opentxs::UnallocatedCString,
        ThreadAlias,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        UnaffiliatedBlockchainTransaction,
        (const opentxs::identifier::Nym&, const opentxs::Data&),
        (const, noexcept, final));

    MOCK_METHOD(
        opentxs::UnallocatedCString,
        UnitDefinitionAlias,
        (const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        opentxs::ObjectList,
        UnitDefinitionList,
        (),
        (const, final));

    MOCK_METHOD(
        std::size_t,
        UnreadCount,
        (const opentxs::UnallocatedCString&, const opentxs::UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        void,
        UpgradeNyms,
        (),
        (final));
};
