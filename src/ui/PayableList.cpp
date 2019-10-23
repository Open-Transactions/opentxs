// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/PayableList.hpp"

#include "internal/api/client/Client.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "PayableList.hpp"

#define OT_METHOD "opentxs::ui::implementation::PayableList::"

#if OT_QT
namespace opentxs::ui
{
QT_MODEL_WRAPPER(PayableListQt, PayableList)
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
PayableList::PayableList(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const proto::ContactItemType& currency
#if OT_QT
    ,
    const bool qt,
    const RowCallbacks insertCallback,
    const RowCallbacks removeCallback
#endif
    )
    : PayableListList(
          api,
          publisher,
          nymID
#if OT_QT
          ,
          qt,
          insertCallback,
          removeCallback,
          Roles{},
          5
#endif
          )
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<PayableList>(&PayableList::process_contact)},
          {api_.Endpoints().NymDownload(),
           new MessageProcessor<PayableList>(&PayableList::process_nym)},
      })
    , owner_contact_id_(Identifier::Factory(last_id_))
    , currency_(currency)
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&PayableList::startup, this));

    OT_ASSERT(startup_)
}

void PayableList::construct_row(
    const PayableListRowID& id,
    const PayableListSortKey& index,
    const CustomData& custom) const noexcept
{
    OT_ASSERT(1 == custom.size())

    std::unique_ptr<const std::string> paymentCode;
    paymentCode.reset(static_cast<const std::string*>(custom[0]));

    OT_ASSERT(paymentCode);
    OT_ASSERT(false == paymentCode->empty());

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::PayableListItem(
            *this, api_, publisher_, id, index, *paymentCode, currency_));
}

#if OT_QT
QVariant PayableList::data(
    const QModelIndex& index,
    int role /*= Qt::DisplayRole*/) const noexcept
{
    if (Qt::DisplayRole == role) {
        const auto [valid, pRow] = check_index(index);
        if (false == valid) { return {}; }
        const auto& row = *pRow;

        switch (index.column()) {
            case 0: {
                return row.ContactID().c_str();
            }
            case 1: {
                return row.DisplayName().c_str();
            }
            case 2: {
                return row.ImageURI().c_str();
            }
            case 3: {
                return row.Section().c_str();
            }
            case 4: {
                return row.PaymentCode().c_str();
            }
            default: {
                return {};
            }
        }
    } else
        return qt_super::data(index, role);
}
#endif

const Identifier& PayableList::ID() const { return owner_contact_id_; }

void PayableList::process_contact(
    const PayableListRowID& id,
    const PayableListSortKey& key)
{
    if (owner_contact_id_ == id) { return; }

    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    const auto contact = api_.Contacts().Contact(id, reason);

    if (false == bool(contact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Contact ")(id)(
            " can not be loaded.")
            .Flush();

        return;
    }

    OT_ASSERT(contact);

    auto paymentCode =
        std::make_unique<std::string>(contact->PaymentCode(currency_));

    OT_ASSERT(paymentCode);

    if (!paymentCode->empty()) {

        add_item(id, key, {paymentCode.release()});
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Skipping unpayable contact ")(id)
            .Flush();
    }
}

void PayableList::process_contact(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto contactID = Identifier::Factory(id);

    OT_ASSERT(false == contactID->empty())

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void PayableList::process_nym(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto nymID = identifier::Nym::Factory(id);

    OT_ASSERT(false == nymID->empty())

    const auto contactID = api_.Contacts().ContactID(nymID);
    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void PayableList::startup()
{
    const auto contacts = api_.Contacts().ContactList();
    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(contacts.size())(
        " contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        process_contact(Identifier::Factory(id), alias);
    }

    finish_startup();
}

PayableList::~PayableList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
