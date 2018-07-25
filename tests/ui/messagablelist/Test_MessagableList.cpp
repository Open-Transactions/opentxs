// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define ALICE_NYM_ID "ot2CyrTzwREHzboZ2RyCT8QsTj3Scaa55JRG"
#define ALICE_NYM_NAME "Alice"
#define DEFAULT_ME_NAME "Owner"
#define BOB_PAYMENT_CODE                                                       \
    "PM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjBxt" \
    "eQ5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97"
#define BOB_NYM_NAME "Bob"
#define CHRIS_PAYMENT_CODE                                                     \
    "PM8TJfV1DQD6VScd5AWsSax8RgK9cUREe939M1d85MwGCKJukyghX6B5E7kqcCyEYu6Tu1Zv" \
    "dG8aWh6w8KGhSfjgL8fBKuZS6aUjhV9xLV1R16CcgWhw"
#define CHRIS_NYM_NAME "Chris"

using namespace opentxs;

template class opentxs::Pimpl<opentxs::PaymentCode>;

namespace
{
class Test_MessagableList : public ::testing::Test
{
public:
    using WidgetUpdateCounter = std::map<std::string, int>;

    const std::string fingerprint_{OT::App().API().Exec().Wallet_ImportSeed(
        "response seminar brave tip suit recall often sound stick owner "
        "lottery motion",
        "")};
    const OTIdentifier nym_id_{
        Identifier::Factory(OT::App().API().Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL,
            ALICE_NYM_NAME,
            fingerprint_,
            -1))};
    std::string contact_widget_id_{""};
    WidgetUpdateCounter counter_;
    std::mutex counter_lock_;
    OTZMQListenCallback callback_{network::zeromq::ListenCallback::Factory(
        [=](const network::zeromq::Message& message) -> void {
            ASSERT_EQ(1, message.size());
            IncrementCounter(message.at(0));
        })};
    OTZMQSubscribeSocket subscriber_{setup_listener(callback_)};
    const ui::MessagableList& messagable_list_{OT::App().UI().MessagableList(nym_id_)};
    std::thread loop_{&Test_MessagableList::loop, this};
    std::atomic<bool> shutdown_{false};
    const OTPaymentCode bob_payment_code_{
        PaymentCode::Factory(BOB_PAYMENT_CODE)};
    OTIdentifier bob_contact_id_{Identifier::Factory()};
    const OTPaymentCode chris_payment_code_{
        PaymentCode::Factory(CHRIS_PAYMENT_CODE)};
    OTIdentifier chris_contact_id_{Identifier::Factory()};

    static OTZMQSubscribeSocket setup_listener(
        const network::zeromq::ListenCallback& callback)
    {
        auto output = OT::App().ZMQ().Context().SubscribeSocket(callback);
        output->Start(network::zeromq::Socket::WidgetUpdateEndpoint);

        return output;
    }

    void IncrementCounter(const std::string& widgetID)
    {
        Lock lock(counter_lock_);
        otErr << "Widget " << widgetID << " update counter set to "
              << ++counter_[widgetID] << std::endl;
    }

    int GetCounter(const std::string& widgetID)
    {
        Lock lock(counter_lock_);

        return counter_[widgetID];
    }

    void loop()
    {
        while (contact_widget_id_.empty()) {
            contact_widget_id_ = messagable_list_.WidgetID()->str();
        }

        while (false == shutdown_.load()) {
            switch (GetCounter(contact_widget_id_)) {
                // The transition between update counter 0 and update counter 1
                // happens based on a background thread running in the
                // MessagableList object. Therefore it's not possible to test the
                // expected state in case 0 because the values could change
                // between when we hit the case label and read the value
                case 0: {
		  std::cout << "Step 0\n";
                } //break;
                case 1: {
                    const auto me = messagable_list_.First();

                    // A MessagableList has a valid First object as soon as it has
                    // been constructed
                    ASSERT_EQ(false, me.get().Valid()); // fixed

                    // We haven't added Bob yet
                    EXPECT_EQ(true, me.get().Last());

                    // Our nym name should have loaded by now
                    EXPECT_EQ("", me.get().DisplayName());

                    // Signal the main thread to add the next contact
                    IncrementCounter(contact_widget_id_);
                } break;
                // messagable_list_ will not be in a deterministic state until
                // case 3
                case 2: {
                } break;
                case 3: {
                    const auto first = messagable_list_.First();

		    // Bob has been added
		    ASSERT_EQ(BOB_NYM_NAME, first.get().DisplayName());
                    ASSERT_EQ(true, first.get().Valid());
                    ASSERT_EQ(true, first.get().Last());

                    const auto bob = messagable_list_.Next();
		    EXPECT_EQ(BOB_NYM_NAME, bob.get().DisplayName());
                    ASSERT_EQ(true, bob.get().Valid());
                    ASSERT_EQ(true, bob.get().Last());                    

                    // Signal the main thread to add the next contact
                    IncrementCounter(contact_widget_id_);
                } break;
                // messagable_list_ will not be in a deterministic state until
                // case 5
                case 4: {
                } break;
                case 5: {
		    // Bob has been added		  
                    const auto bob = messagable_list_.First();		    

                    ASSERT_EQ(true, bob.get().Valid());
                    ASSERT_EQ(false, bob.get().Last());
		    EXPECT_EQ(bob.get().DisplayName(), BOB_NYM_NAME);

		    // Chris has been added
                    const auto chris = messagable_list_.Next();
                    ASSERT_EQ(true, chris.get().Valid());
		    ASSERT_EQ(true, chris.get().Last());
		    EXPECT_EQ(chris.get().DisplayName(), CHRIS_NYM_NAME);
		    
		    // Next circles back to First
		    const auto next = messagable_list_.Next();
		    EXPECT_EQ(next.get().DisplayName(), BOB_NYM_NAME);
                    ASSERT_EQ(true, chris.get().Valid());
		    ASSERT_EQ(false, bob.get().Last());		   

                    IncrementCounter(contact_widget_id_);
                } break;
                case 6: {
                    // Ensure returning to First() before reaching the end of
                    // the list behaves as expected

                    // Bob
                    auto line = messagable_list_.First();
                    ASSERT_EQ(true, line.get().Valid());
                    ASSERT_EQ(false, line.get().Last());

		    // Chris
                    line = messagable_list_.Next();
		    ASSERT_EQ(true, line.get().Valid());
                    ASSERT_EQ(true, line.get().Last());
		    
                    // Bob
                    line = messagable_list_.First();
                    // Chris
                    line = messagable_list_.Next();
		    // Bob
                    line = messagable_list_.Next();
                    
                    ASSERT_EQ(true, line.get().Valid());
                    ASSERT_EQ(false, line.get().Last());
                    EXPECT_EQ(line.get().DisplayName(), BOB_NYM_NAME);

                    IncrementCounter(contact_widget_id_);
                } break;
                default: {
                    shutdown_.store(true);
                }
            }
        }
    }

    ~Test_MessagableList() { loop_.join(); }
};

TEST_F(Test_MessagableList, Contact_List)
{
    ASSERT_EQ(false, nym_id_->empty());
    ASSERT_EQ(true, bob_payment_code_->VerifyInternally());

    while (GetCounter(contact_widget_id_) < 2) { ; }

    const auto bob = OT::App().Contact().NewContact(
        BOB_NYM_NAME, bob_payment_code_->ID(), bob_payment_code_);

    ASSERT_EQ(true, bool(bob));

    bob_contact_id_ = Identifier::Factory(bob->ID());

    ASSERT_EQ(false, bob_contact_id_->empty());

    while (GetCounter(contact_widget_id_) < 4) { ; }

    const auto chris = OT::App().Contact().NewContact(
        CHRIS_NYM_NAME, chris_payment_code_->ID(), chris_payment_code_);

    ASSERT_EQ(true, bool(chris));

    chris_contact_id_ = Identifier::Factory(chris->ID());
}
}  // namespace
