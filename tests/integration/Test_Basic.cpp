// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#include <mutex>

using namespace opentxs;

#define ACCOUNT_SUMMARY_BTC "ACCOUNT_SUMMARY_BTC"
#define ACCOUNT_SUMMARY_BCH "ACCOUNT_SUMMARY_BCH"
#define ACTIVITY_SUMMARY "ACTIVITY_SUMMARY"
#define CONTACTS_LIST "CONTACTS_LIST"
#define MESSAGABLE_LIST "MESSAGAGABLE_LIST"
#define PAYABLE_LIST_BTC "PAYABLE_LIST_BTC"
#define PAYABLE_LIST_BCH "PAYABLE_LIST_BCH"
#define PROFILE "PROFILE"

namespace
{
bool init_{false};

class Test_Basic : public ::testing::Test
{
public:
    // name, Widget ID, counter
    using WidgetData = std::map<std::string, std::pair<OTIdentifier, int>>;
    // id, name
    using WidgetMap = std::map<OTIdentifier, std::string>;
    // message type, counter
    using ServerReplyMap = std::map<std::string, int>;

    static const opentxs::ArgList args_;
    static const std::string SeedA_;
    static const std::string SeedB_;
    static const std::string Alice_;
    static const std::string Bob_;
    static const OTIdentifier alice_nym_id_;
    static const OTIdentifier bob_nym_id_;
    static const std::shared_ptr<const ServerContract> server_contract_;

    static WidgetData alice_updates_;
    // Widget id, name
    static WidgetMap alice_widget_map_;
    static WidgetData bob_updates_;
    static WidgetMap bob_widget_map_;
    static ServerReplyMap alice_server_reply_map_;
    static ServerReplyMap bob_server_reply_map_;

    static OTZMQListenCallback alice_ui_update_callback_;
    static OTZMQListenCallback bob_ui_update_callback_;
    static OTZMQListenCallback alice_server_reply_callback_;
    static OTZMQListenCallback bob_server_reply_callback_;

    static const opentxs::api::client::Manager* alice_;
    static const opentxs::api::client::Manager* bob_;
    static std::mutex callback_lock_;
    static std::mutex server_reply_lock_;

    const opentxs::api::client::Manager& alice_client_;
    const opentxs::api::client::Manager& bob_client_;
    const opentxs::api::server::Manager& server_;
    const Identifier& server_id_;
    OTZMQSubscribeSocket alice_ui_update_listener_;
    OTZMQSubscribeSocket bob_ui_update_listener_;
    OTZMQSubscribeSocket alice_server_reply_listener_;
    OTZMQSubscribeSocket bob_server_reply_listener_;

    Test_Basic()
        : alice_client_(OT::App().StartClient(args_, 0))
        , bob_client_(OT::App().StartClient(args_, 1))
        , server_(OT::App().StartServer(args_, 0, true))
        , server_id_(server_.ID())
        , alice_ui_update_listener_(
              alice_client_.ZeroMQ().SubscribeSocket(alice_ui_update_callback_))
        , bob_ui_update_listener_(
              bob_client_.ZeroMQ().SubscribeSocket(bob_ui_update_callback_))
        , alice_server_reply_listener_(alice_client_.ZeroMQ().SubscribeSocket(
              alice_server_reply_callback_))
        , bob_server_reply_listener_(
              bob_client_.ZeroMQ().SubscribeSocket(bob_server_reply_callback_))
    {
        subscribe_sockets();

        if (false == init_) { init(); }

        Lock lock(server_reply_lock_);
        alice_server_reply_map_.clear();
        bob_server_reply_map_.clear();
    }

    void add_ui_widget(
        const std::string& name,
        const Identifier& id,
        WidgetData& data,
        WidgetMap& map)
    {
        otErr << "Name: " << name << " ID: " << id.str() << std::endl;
        data.emplace(name, std::pair<OTIdentifier, int>({id, 0}));
        map.emplace(id, name);
    }

    static void check_alice_account_summary_btc(const int counter)
    {
        const auto& summary =
            alice_->UI().AccountSummary(alice_nym_id_, proto::CITEMTYPE_BTC);

        switch (counter) {
            case 0: {
                const auto& first = summary.First();
                EXPECT_FALSE(first->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_account_summary_bch(const int counter)
    {
        const auto& summary =
            alice_->UI().AccountSummary(alice_nym_id_, proto::CITEMTYPE_BCH);

        switch (counter) {
            case 0: {
                const auto& first = summary.First();
                EXPECT_FALSE(first->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_activity_summary(const int counter)
    {
        const auto& summary = alice_->UI().ActivitySummary(alice_nym_id_);

        switch (counter) {
            case 0: {
                const auto& first = summary.First();
                EXPECT_FALSE(first->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_contacts(const int counter)
    {
        const auto& contacts = alice_->UI().ContactList(alice_nym_id_);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                const auto& first = contacts.First();
                ASSERT_TRUE(first->Valid());

                EXPECT_TRUE(
                    first->DisplayName() == "Alice" ||
                    first->DisplayName() == "Owner");
                EXPECT_TRUE(first->Valid());

                EXPECT_STREQ("", first->ImageURI().c_str());
                EXPECT_STREQ("ME", first->Section().c_str());

                const auto& next = contacts.Next();
                ASSERT_TRUE(next->Last());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_messagable_list(const int counter)
    {
        const auto& list = alice_->UI().MessagableList(alice_nym_id_);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                ASSERT_TRUE(list.First()->Valid());

                // TODO check section
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_payables_btc(const int counter)
    {
        const auto& list =
            alice_->UI().PayableList(alice_nym_id_, proto::CITEMTYPE_BTC);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                EXPECT_TRUE(list.First()->Valid());
                EXPECT_STREQ("Alice", list.First()->DisplayName().c_str());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_payables_bch(const int counter)
    {
        const auto& list =
            alice_->UI().PayableList(alice_nym_id_, proto::CITEMTYPE_BTC);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                EXPECT_FALSE(list.First()->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_alice_profile(const int counter)
    {
        const auto& profile = alice_->UI().Profile(alice_nym_id_);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
            } break;  // skip
            case 2: {
            } break;  // skip
            case 3: {
                auto alice_paymentcode_ =
                    alice_->Factory().PaymentCode(SeedA_, 0, 1);
                EXPECT_STREQ(
                    alice_paymentcode_->asBase58().c_str(),
                    profile.PaymentCode().c_str());

                EXPECT_STREQ("Alice", profile.DisplayName().c_str());

                const auto& alice_first_section_ = profile.First();

                ASSERT_TRUE(alice_first_section_->Valid());

                // TODO check section
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_account_summary_btc(const int counter)
    {
        const auto& summary =
            bob_->UI().AccountSummary(bob_nym_id_, proto::CITEMTYPE_BTC);

        switch (counter) {
            case 0: {
                const auto& bob_first_account_ = summary.First();

                EXPECT_FALSE(bob_first_account_->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_account_summary_bch(const int counter)
    {
        const auto& summary =
            bob_->UI().AccountSummary(bob_nym_id_, proto::CITEMTYPE_BCH);

        switch (counter) {
            case 0: {
                const auto& first = summary.First();

                EXPECT_FALSE(first->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_activity_summary(const int counter)
    {
        const auto& summary = bob_->UI().ActivitySummary(bob_nym_id_);

        switch (counter) {
            case 0: {
                const auto& first = summary.First();

                EXPECT_FALSE(first->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_contacts(const int counter)
    {
        const auto& contacts = bob_->UI().ContactList(bob_nym_id_);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                const auto& bob_first_contact_ = contacts.First();
                ASSERT_TRUE(bob_first_contact_->Valid());
                EXPECT_STRNE("", bob_first_contact_->ContactID().c_str());

                EXPECT_TRUE(
                    bob_first_contact_->DisplayName() == "Bob" ||
                    bob_first_contact_->DisplayName() == "Owner");

                EXPECT_TRUE(bob_first_contact_->Valid());

                EXPECT_STREQ("", bob_first_contact_->ImageURI().c_str());

                EXPECT_STREQ("ME", bob_first_contact_->Section().c_str());

                const auto& next_bob_contact_ = contacts.Next();

                ASSERT_TRUE(next_bob_contact_->Last());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_messagable_list(const int counter)
    {
        const auto& list = bob_->UI().MessagableList(bob_nym_id_);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                const auto& first = list.First();

                ASSERT_TRUE(first->Valid());

                // TODO check section
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_payables_btc(const int counter)
    {
        const auto& list =
            bob_->UI().PayableList(bob_nym_id_, proto::CITEMTYPE_BTC);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                EXPECT_TRUE(list.First()->Valid());
                EXPECT_STREQ("Bob", list.First()->DisplayName().c_str());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_payables_bch(const int counter)
    {
        const auto& list =
            bob_->UI().PayableList(bob_nym_id_, proto::CITEMTYPE_BCH);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
                const auto& first = list.First();
                EXPECT_FALSE(first->Valid());
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static void check_bob_profile(const int counter)
    {
        const auto& profile = bob_->UI().Profile(bob_nym_id_);

        switch (counter) {
            case 0: {
            } break;  // skip
            case 1: {
            } break;  // skip
            case 2: {
            } break;  // skip
            case 3: {
                auto bob_paymentcode_ =
                    bob_->Factory().PaymentCode(SeedB_, 0, 1);
                EXPECT_STREQ(
                    bob_paymentcode_->asBase58().c_str(),
                    profile.PaymentCode().c_str());

                EXPECT_STREQ("Bob", profile.DisplayName().c_str());

                const auto& bob_first_section_ = profile.First();

                ASSERT_TRUE(bob_first_section_->Valid());

                // TODO check section
            } break;
            default: {
                ASSERT_TRUE(false);
            }
        }
    }

    static const int& get_counter(
        const Identifier& id,
        WidgetData& data,
        WidgetMap& map)
    {
        return data.at(map.at(id)).second;
    }

    static const int get_reply(const std::string& type, ServerReplyMap& map)
    {
        Lock lock(server_reply_lock_);

        try {
            return map.at(type);
        } catch (...) {
            return 0;
        }
    }

    void import_server_contract(
        const ServerContract& contract,
        const opentxs::api::client::Manager& client)
    {
        auto clientVersion =
            client.Wallet().Server(server_contract_->PublicContract());

        OT_ASSERT(clientVersion)

        client.Sync().SetIntroductionServer(*clientVersion);
    }

    static void increment_counter(
        const Identifier& id,
        WidgetData& data,
        WidgetMap& map)
    {
        const auto& name = map.at(id);
        auto& counter = data.at(name).second;
        ++counter;
    }

    void init()
    {
        const_cast<std::string&>(SeedA_) =
            alice_client_.Exec().Wallet_ImportSeed(
                "spike nominee miss inquiry fee nothing belt list other "
                "daughter leave valley twelve gossip paper",
                "");
        const_cast<std::string&>(SeedB_) = bob_client_.Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        const_cast<std::string&>(Alice_) = alice_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Alice", SeedA_, 0);
        const_cast<std::string&>(Bob_) = bob_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Bob", SeedB_, 0);
        const_cast<OTIdentifier&>(alice_nym_id_) = Identifier::Factory(Alice_);
        const_cast<OTIdentifier&>(bob_nym_id_) = Identifier::Factory(Bob_);
        const_cast<std::shared_ptr<const ServerContract>&>(server_contract_) =
            server_.Wallet().Server(server_id_);

        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_id_.empty());

        import_server_contract(*server_contract_, alice_client_);
        import_server_contract(*server_contract_, bob_client_);

        alice_ = &alice_client_;
        bob_ = &bob_client_;

        init_ = true;
    }

    static void process_server_reply(
        ServerReplyMap& map,
        const opentxs::network::zeromq::Message& incoming)
    {
        if (0 == incoming.Body().size()) { return; }

        const std::string replyType{incoming.Body().at(0)};
        otErr << "Received " << replyType << " ";
        Lock lock(server_reply_lock_);
        auto& counter = map[replyType];
        otErr << std::to_string(++counter) << std::endl;
    }

    void subscribe_sockets()
    {
        ASSERT_TRUE(alice_ui_update_listener_->Start(
            alice_client_.Endpoints().WidgetUpdate()));
        ASSERT_TRUE(bob_ui_update_listener_->Start(
            bob_client_.Endpoints().WidgetUpdate()));
        ASSERT_TRUE(alice_server_reply_listener_->Start(
            alice_client_.Endpoints().ServerReplyReceived()));
        ASSERT_TRUE(bob_server_reply_listener_->Start(
            bob_client_.Endpoints().ServerReplyReceived()));
    }

    bool ui()
    {
        Lock lock(callback_lock_);
        add_ui_widget(
            PROFILE,
            alice_client_.UI().Profile(alice_nym_id_).WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            PROFILE,
            bob_client_.UI().Profile(bob_nym_id_).WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            ACTIVITY_SUMMARY,
            alice_client_.UI().ActivitySummary(alice_nym_id_).WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            ACTIVITY_SUMMARY,
            bob_client_.UI().ActivitySummary(bob_nym_id_).WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            CONTACTS_LIST,
            alice_client_.UI().ContactList(alice_nym_id_).WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            CONTACTS_LIST,
            bob_client_.UI().ContactList(bob_nym_id_).WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            PAYABLE_LIST_BCH,
            alice_client_.UI()
                .PayableList(alice_nym_id_, proto::CITEMTYPE_BCH)
                .WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            PAYABLE_LIST_BCH,
            bob_client_.UI()
                .PayableList(bob_nym_id_, proto::CITEMTYPE_BCH)
                .WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            PAYABLE_LIST_BTC,
            alice_client_.UI()
                .PayableList(alice_nym_id_, proto::CITEMTYPE_BTC)
                .WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            PAYABLE_LIST_BTC,
            bob_client_.UI()
                .PayableList(bob_nym_id_, proto::CITEMTYPE_BTC)
                .WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            ACCOUNT_SUMMARY_BTC,
            alice_client_.UI()
                .AccountSummary(alice_nym_id_, proto::CITEMTYPE_BTC)
                .WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            ACCOUNT_SUMMARY_BTC,
            bob_client_.UI()
                .AccountSummary(bob_nym_id_, proto::CITEMTYPE_BTC)
                .WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            ACCOUNT_SUMMARY_BCH,
            alice_client_.UI()
                .AccountSummary(alice_nym_id_, proto::CITEMTYPE_BCH)
                .WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            ACCOUNT_SUMMARY_BCH,
            bob_client_.UI()
                .AccountSummary(bob_nym_id_, proto::CITEMTYPE_BCH)
                .WidgetID(),
            bob_updates_,
            bob_widget_map_);
        add_ui_widget(
            MESSAGABLE_LIST,
            alice_client_.UI().MessagableList(alice_nym_id_).WidgetID(),
            alice_updates_,
            alice_widget_map_);
        add_ui_widget(
            MESSAGABLE_LIST,
            bob_client_.UI().MessagableList(bob_nym_id_).WidgetID(),
            bob_updates_,
            bob_widget_map_);

        EXPECT_EQ(8, alice_updates_.size());
        EXPECT_EQ(8, bob_updates_.size());
        EXPECT_EQ(8, alice_widget_map_.size());
        EXPECT_EQ(8, bob_widget_map_.size());

        return true;
    }

    static void widget_updated_alice(
        const opentxs::network::zeromq::Message& incoming)
    {
        Lock lock(callback_lock_);
        const auto widgetID = Identifier::Factory(incoming.Body().at(0));

        ASSERT_NE("", widgetID->str().c_str());

        increment_counter(widgetID, alice_updates_, alice_widget_map_);
        const int counter =
            get_counter(widgetID, alice_updates_, alice_widget_map_);
        const auto& name = alice_widget_map_.at(widgetID);

        if (name == PROFILE) {
            check_alice_profile(counter);
        } else if (ACCOUNT_SUMMARY_BTC == name) {
            check_alice_account_summary_btc(counter);
        } else if (ACCOUNT_SUMMARY_BCH == name) {
            check_alice_account_summary_bch(counter);
        } else if (ACTIVITY_SUMMARY == name) {
            check_alice_activity_summary(counter);
        } else if (CONTACTS_LIST == name) {
            check_alice_contacts(counter);
        } else if (MESSAGABLE_LIST == name) {
            check_alice_messagable_list(counter);
        } else if (PAYABLE_LIST_BTC == name) {
            check_alice_payables_btc(counter);
        } else if (PAYABLE_LIST_BCH == name) {
            check_alice_payables_bch(counter);
        }
    }

    static void widget_updated_bob(
        const opentxs::network::zeromq::Message& incoming)
    {
        Lock lock(callback_lock_);
        const auto widgetID = Identifier::Factory(incoming.Body().at(0));

        ASSERT_NE("", widgetID->str().c_str());

        increment_counter(widgetID, bob_updates_, bob_widget_map_);
        const int counter =
            get_counter(widgetID, bob_updates_, bob_widget_map_);
        const auto& name = bob_widget_map_.at(widgetID);

        if (name == PROFILE) {
            check_bob_profile(counter);
        } else if (ACCOUNT_SUMMARY_BTC == name) {
            check_bob_account_summary_btc(counter);
        } else if (ACCOUNT_SUMMARY_BCH == name) {
            check_bob_account_summary_bch(counter);
        } else if (ACTIVITY_SUMMARY == name) {
            check_bob_activity_summary(counter);
        } else if (CONTACTS_LIST == name) {
            check_bob_contacts(counter);
        } else if (MESSAGABLE_LIST == name) {
            check_bob_messagable_list(counter);
        } else if (PAYABLE_LIST_BTC == name) {
            check_bob_payables_btc(counter);
        } else if (PAYABLE_LIST_BCH == name) {
            check_bob_payables_bch(counter);
        }
    }
};

const opentxs::ArgList Test_Basic::args_{
    {{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
const std::string Test_Basic::SeedA_{""};
const std::string Test_Basic::SeedB_{""};
const std::string Test_Basic::Alice_{""};
const std::string Test_Basic::Bob_{""};
const OTIdentifier Test_Basic::alice_nym_id_{Identifier::Factory()};
const OTIdentifier Test_Basic::bob_nym_id_{Identifier::Factory()};
const std::shared_ptr<const ServerContract> Test_Basic::server_contract_{
    nullptr};
Test_Basic::WidgetData Test_Basic::alice_updates_{};
Test_Basic::WidgetMap Test_Basic::alice_widget_map_{};
Test_Basic::WidgetData Test_Basic::bob_updates_{};
Test_Basic::WidgetMap Test_Basic::bob_widget_map_{};
Test_Basic::ServerReplyMap Test_Basic::alice_server_reply_map_{};
Test_Basic::ServerReplyMap Test_Basic::bob_server_reply_map_{};
OTZMQListenCallback Test_Basic::alice_ui_update_callback_{
    opentxs::network::zeromq::ListenCallback::Factory(
        [](const opentxs::network::zeromq::Message& incoming) -> void {
            widget_updated_alice(incoming);
        })};
OTZMQListenCallback Test_Basic::bob_ui_update_callback_{
    opentxs::network::zeromq::ListenCallback::Factory(
        [](const opentxs::network::zeromq::Message& incoming) -> void {
            widget_updated_bob(incoming);
        })};
OTZMQListenCallback Test_Basic::alice_server_reply_callback_{
    opentxs::network::zeromq::ListenCallback::Factory(
        [](const opentxs::network::zeromq::Message& incoming) -> void {
            process_server_reply(alice_server_reply_map_, incoming);
        })};
OTZMQListenCallback Test_Basic::bob_server_reply_callback_{
    opentxs::network::zeromq::ListenCallback::Factory(
        [](const opentxs::network::zeromq::Message& incoming) -> void {
            process_server_reply(bob_server_reply_map_, incoming);
        })};
const opentxs::api::client::Manager* Test_Basic::alice_{nullptr};
const opentxs::api::client::Manager* Test_Basic::bob_{nullptr};
std::mutex Test_Basic::callback_lock_{};
std::mutex Test_Basic::server_reply_lock_{};

// A test that simulates Alice and Bob both setting up clients for the first
// time up to the point at which the main screen is displayed.
TEST_F(Test_Basic, startClients)
{
    // Configuration
    alice_client_.Exec().SetZMQKeepAlive(30);
    alice_client_.Exec().SetConfig_long("Connection", "keep_alive", 60);
    alice_client_.Exec().SetConfig_long(
        "Connection", "preferred_address_type", 3);
    alice_client_.Exec().SetConfig_long("security", "master_key_timeout", -1);
    alice_client_.Exec().SetConfig_long("latency", "send_timeout", 1000);
    alice_client_.Exec().SetConfig_long("latency", "recv_timeout", 10000);
    alice_client_.ZMQ().SetSocksProxy("127.0.0.1:9045");

    bob_client_.Exec().SetZMQKeepAlive(30);
    bob_client_.Exec().SetConfig_long("Connection", "keep_alive", 60);
    bob_client_.Exec().SetConfig_long(
        "Connection", "preferred_address_type", 3);
    bob_client_.Exec().SetConfig_long("security", "master_key_timeout", -1);
    bob_client_.Exec().SetConfig_long("latency", "send_timeout", 1000);
    bob_client_.Exec().SetConfig_long("latency", "recv_timeout", 10000);
    bob_client_.ZMQ().SetSocksProxy("127.0.0.1:9045");

    // subscribe to socket
    // Server Connection (this is currently a workaround in SW for TOR)

    auto& serverID = alice_client_.Sync().IntroductionServer();

    ASSERT_FALSE(serverID.empty());

    auto& connection = alice_client_.ZMQ().Server(serverID.str());
    connection.ChangeAddressType(static_cast<proto::AddressType>(1));

    auto& serverID_ = bob_client_.Sync().IntroductionServer();

    ASSERT_FALSE(serverID_.empty());

    auto& connection2 = bob_client_.ZMQ().Server(serverID_.str());
    connection2.ChangeAddressType(static_cast<proto::AddressType>(1));
    // alice_client_.Sync().StartIntroductionServer(alice_nym_id_);
    // bob_client_.Sync().StartIntroductionServer(bob_nym_id_);
    auto alice = alice_client_.Wallet().mutable_Nym(alice_nym_id_);
    auto bob = bob_client_.Wallet().mutable_Nym(bob_nym_id_);

    EXPECT_EQ(proto::CITEMTYPE_INDIVIDUAL, alice.Type());
    EXPECT_EQ(proto::CITEMTYPE_INDIVIDUAL, bob.Type());

    auto aliceScopeSet =
        alice.SetScope(proto::CITEMTYPE_INDIVIDUAL, "Alice", true);
    auto bobScopeSet = bob.SetScope(proto::CITEMTYPE_INDIVIDUAL, "Bob", true);

    EXPECT_TRUE(aliceScopeSet);
    EXPECT_TRUE(bobScopeSet);

    // add payment codes
    std::cout << "Adding payment code\n";
    auto alice_paymentcode_ = alice_client_.Factory().PaymentCode(SeedA_, 0, 1);
    auto bob_paymentcode_ = bob_client_.Factory().PaymentCode(SeedB_, 0, 1);

    ASSERT_STRNE("", alice_paymentcode_->asBase58().c_str());
    ASSERT_STRNE("", bob_paymentcode_->asBase58().c_str());

    alice.AddPaymentCode(
        alice_paymentcode_->asBase58(),
        opentxs::proto::CITEMTYPE_BTC,
        true,
        true);
    bob.AddPaymentCode(
        bob_paymentcode_->asBase58(),
        opentxs::proto::CITEMTYPE_BTC,
        true,
        true);
    alice.AddPaymentCode(
        alice_paymentcode_->asBase58(),
        opentxs::proto::CITEMTYPE_BCH,
        true,
        true);
    bob.AddPaymentCode(
        bob_paymentcode_->asBase58(),
        opentxs::proto::CITEMTYPE_BCH,
        true,
        true);

    ASSERT_STRNE("", alice.PaymentCode(proto::CITEMTYPE_BTC).c_str());
    ASSERT_STRNE("", bob.PaymentCode(proto::CITEMTYPE_BTC).c_str());
    ASSERT_STRNE("", alice.PaymentCode(proto::CITEMTYPE_BCH).c_str());
    ASSERT_STRNE("", bob.PaymentCode(proto::CITEMTYPE_BCH).c_str());

    alice.Release();
    bob.Release();
    alice_client_.Sync().StartIntroductionServer(alice_nym_id_);
    bob_client_.Sync().StartIntroductionServer(bob_nym_id_);

    // get the first nym

    ASSERT_EQ(1, alice_client_.Exec().GetNymCount());

    auto alice_nym_ = alice_client_.Exec().GetNym_ID(0);

    ASSERT_STREQ(alice_nym_id_->str().c_str(), alice_nym_.c_str());
    ASSERT_EQ(1, bob_client_.Exec().GetNymCount());

    auto bob_nym_ = bob_client_.Exec().GetNym_ID(0);

    ASSERT_STREQ(bob_nym_id_->str().c_str(), bob_nym_.c_str());

    ui();

    while (true) {
        Log::Sleep(std::chrono::milliseconds(10));

        if (-1 == alice_updates_.at(ACCOUNT_SUMMARY_BTC).second) { continue; }
        if (-1 == bob_updates_.at(ACCOUNT_SUMMARY_BTC).second) { continue; }
        if (-1 == alice_updates_.at(ACCOUNT_SUMMARY_BCH).second) { continue; }
        if (-1 == bob_updates_.at(ACCOUNT_SUMMARY_BCH).second) { continue; }
        if (-1 == alice_updates_.at(ACTIVITY_SUMMARY).second) { continue; }
        if (-1 == bob_updates_.at(ACTIVITY_SUMMARY).second) { continue; }
        if (0 == alice_updates_.at(CONTACTS_LIST).second) { continue; }
        if (0 == bob_updates_.at(CONTACTS_LIST).second) { continue; }
        if (-1 == alice_updates_.at(MESSAGABLE_LIST).second) { continue; }
        if (-1 == bob_updates_.at(MESSAGABLE_LIST).second) { continue; }
        if (0 == alice_updates_.at(PAYABLE_LIST_BTC).second) { continue; }
        if (0 == bob_updates_.at(PAYABLE_LIST_BTC).second) { continue; }
        // TODO change -1 to 0
        if (-1 == alice_updates_.at(PAYABLE_LIST_BCH).second) { continue; }
        // TODO change -1 to 0
        if (-1 == bob_updates_.at(PAYABLE_LIST_BCH).second) { continue; }
        if (2 > alice_updates_.at(PROFILE).second) { continue; }
        if (2 > bob_updates_.at(PROFILE).second) { continue; }

        if (1 > get_reply("registerNym", alice_server_reply_map_)) { continue; }
        if (1 > get_reply("registerNym", bob_server_reply_map_)) { continue; }
        if (1 > get_reply("getNymbox", alice_server_reply_map_)) { continue; }
        if (1 > get_reply("getNymbox", bob_server_reply_map_)) { continue; }
        if (1 > get_reply("checkNym", alice_server_reply_map_)) { continue; }
        if (1 > get_reply("checkNym", bob_server_reply_map_)) { continue; }

        check_alice_activity_summary(0);
        check_bob_activity_summary(0);
        check_alice_account_summary_btc(0);
        check_bob_account_summary_btc(0);
        check_alice_account_summary_bch(0);
        check_bob_account_summary_bch(0);
        check_alice_messagable_list(0);
        check_bob_messagable_list(0);

        break;
    }
}
}  // namespace
