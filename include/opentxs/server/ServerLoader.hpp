#ifndef OPENTXS_SERVER_SERVERLOADER_HPP
#define OPENTXS_SERVER_SERVERLOADER_HPP

#include "OTServer.hpp"
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/OTDataFolder.hpp>
#include <opentxs/core/OTLog.hpp>
#include <syslog.h>

#define SERVER_CONFIG_KEY "server"

namespace opentxs
{

class ServerLoader
{
public:
    ServerLoader()
        : server_(nullptr)
    {
// This is optional! (I, of course, am using it in this test app...)
#if defined(OT_SIGNAL_HANDLING)
        OTLog::SetupSignalHandler();
#endif
        // I instantiate this here (instead of globally) so that I am
        // assured that any globals and other
        // setup is already done before we instantiate the server object
        // itself.
        OT_ASSERT_MSG(nullptr == server_,
                      "server main(): ASSERT: nullptr == server_.");
        server_ = new OTServer;

        OT_ASSERT_MSG(
            nullptr != server_,
            "server main(): ASSERT: Unable to instantiate OT server.\n");

        {
            bool setupPathsSuccess = false;
            if (!OTDataFolder::Init(SERVER_CONFIG_KEY)) {
                OT_FAIL;
            }
            else {
                setupPathsSuccess = true;
            }
            OT_ASSERT_MSG(setupPathsSuccess,
                          "main(): Assert failed: Failed to set OT Path");

            if (!OTDataFolder::IsInitialized()) {
                OT_FAIL;
            }
            // Performance log initialization
            static char szIdent[80] = "";
            sprintf(szIdent, "opentxs-notary %s,", OTLog::Version());

            // Configure log entries so they all start with:
            // "opentxs-notary <version>,PID,eventTimeinMicroseconds,"
            // Each type of probe will then append its own comma separated data
            // fields
            openlog(szIdent, LOG_PID, LOG_USER);
        }
        OTCrypto::It()->Init();

        // OTServer::Init loads up server's nym so it can decrypt messages sent
        // in envelopes. It also does various other initialization work.
        //
        // NOTE: Envelopes prove that ONLY someone who actually had the server
        // contract, and had loaded it into his wallet, could ever connect to
        // the server or communicate with it. And if that person is following
        // the contract, there is only one server he can connect to, and one
        // key he can use to talk to it.
        //
        // Keys, etc are loaded here. Assumes main path is set!
        server_->Init();

        // A heartbeat for recurring transactions, such as markets, payment
        // plans, and smart contracts.
        server_->ActivateCron();
    }

    ~ServerLoader()
    {
        OTLog::vOutput(0, "\n\n Shutting down and cleaning up.\n");

        if (server_) {
            delete server_;
            server_ = nullptr;
        }
        closelog(); // close the performance log
        OTCachedKey::Cleanup();
        OTCrypto::It()->Cleanup();
    }

    OTServer* getServer()
    {
        return server_;
    }

    int getPort() const
    {
        // We're going to listen on the same port that is listed in our server
        // contract. The hostname of this server, according to its own contract.
        OTString hostname;
        int port = 0;
        bool connectInfo = server_->GetConnectInfo(hostname, port);

        OT_ASSERT_MSG(connectInfo,
                      "server main: Unable to find my own connect "
                      "info (which SHOULD be in my server contract, "
                      "BTW.) Perhaps you failed trying to open that "
                      "contract? Have you tried the test password? "
                      "(\"test\")\n");
        return port;
    }

private:
    ServerLoader(const ServerLoader&);
    ServerLoader& operator=(const ServerLoader&);

    OTServer* server_;
};

} // namespace opentxs

#endif // OPENTXS_SERVER_SERVERLOADER_HPP
