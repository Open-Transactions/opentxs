// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OT_HPP
#define OPENTXS_OT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <chrono>
#include <map>
#include <string>

#define OPENTXS_ARG_BACKUP_DIRECTORY "backupdirectory"
#define OPENTXS_ARG_BINDIP "bindip"
#define OPENTXS_ARG_COMMANDPORT "commandport"
#define OPENTXS_ARG_EEP "eep"
#define OPENTXS_ARG_ENCRYPTED_DIRECTORY "encrypteddirectory"
#define OPENTXS_ARG_EXTERNALIP "externalip"
#define OPENTXS_ARG_GC "gc"
#define OPENTXS_ARG_INIT "only-init"
#define OPENTXS_ARG_INPROC "inproc"
#define OPENTXS_ARG_LISTENCOMMAND "listencommand"
#define OPENTXS_ARG_LISTENNOTIFY "listennotify"
#define OPENTXS_ARG_NAME "name"
#define OPENTXS_ARG_NOTIFICATIONPORT "notificationport"
#define OPENTXS_ARG_ONION "onion"
#define OPENTXS_ARG_PASSPHRASE "passphrase"
#define OPENTXS_ARG_STORAGE_PLUGIN "storageplugin"
#define OPENTXS_ARG_TERMS "terms"
#define OPENTXS_ARG_VERSION "version"
#define OPENTXS_ARG_WORDS "words"
#define OPENTXS_ARG_LOGENDPOINT "logendpoint"

namespace opentxs
{
/** \brief Static methods for starting up the native api.
 *  \ingroup native
 */
class OT
{
public:
    /** Native API accessor
     *
     *  Returns a reference to the native API singleton after it has been
     *  initialized.
     */
    static const api::Native& App();
    /** OT shutdown method
     *
     *  Call this when the application is closing, after all OT operations
     *  are complete.
     */
    static void Cleanup();
    static const api::client::Manager& ClientFactory(
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);
    static const api::client::Manager& RecoverClient(
        const ArgList& args,
        const std::string& words,
        const std::string& passphrase,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);
    static void Join();
    static const opentxs::Flag& Running();
    static const api::server::Manager& ServerFactory(
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);
    static const api::Native& Start(
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);

private:
    static api::Native* instance_pointer_;
    static OTFlag running_;

    OT() = delete;
    OT(const OT&) = delete;
    OT(OT&&) = delete;
    OT& operator=(const OT&) = delete;
    OT& operator=(OT&&) = delete;
};
}  // namespace opentxs
#endif
