/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_CRYPTO_OTCACHEDKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTCACHEDKEY_HPP

#include "opentxs/core/String.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace opentxs
{

class Identifier;
class OTASCIIArmor;
class OTCachedKey;
class OTPassword;
class OTSymmetricKey;

/**
This class handles the functionality of caching the master key for X seconds as
an OTPassword, and then deleting it. It also caches the encrypted version in an
OTSymmetricKey, which can be unlocked to an OTPassword again for X more seconds
(by entering the passphrase...)

How does OTCachedKey work, in a nutshell?

 -- It's a singleton. There's only one "master key" for the application and it's
available globally like this: OTCachedKey::It(). For example:
OTCachedKey::It()->IsGenerated()
 -- m_pSymmetricKey contains a symmetric key in ENCRYPTED form, which loads up
from the wallet, and which is always kept in RAM (as on the hard drive) ONLY in
encrypted form.
 -- m_pMasterPassword is the DECRYPTED version of m_pSymmetricKey. When a user
enters his password, a key derivation hash is used to transform it, and the
result is used to get an unlocked copy of m_pSymmetricKey, putting the cleartext
version into m_pMasterPassword.
 -- Why store a decrypted version in memory? So we can set a timer using another
thread, and thereby return X seconds later, and destroy that decrypted version.
In the meantime, until it is destroyed, it will be used automatically by OT,
every time a password is needed until those X seconds are over.
 -- That's still not good enough: the decrypted version should still be stored
in protected memory, using some standard API such as gpg-agent or Windows
VirtualProtect. And whatever is stored there, should ALSO be encrypted to a
session key which is re-generated for every run of the application. Furthermore,
the decrypted version being stored isn't the user's actual passphrase, nor is it
the key derived from it via some password hash algorithm, but instead is a
random number that was generated in order to be used as the master key.
 -- This means that the actual password the user enters, is never stored
anywhere. Neither is the key derived from that password, since both can
instantly be erased after using them to unlock the master key. Furthermore the
master key is encrypted to a temporary session key, which is different for every
run of the application, so it is never stored cleartext in memory, either, nor
is it thus available to anyone with backdoors into protected memory.
 -- Not only does the session key change after each run of the application, but
it is possible to replace the master key without forcing a change of the
password itself. Likewise it is also possible to change the password, without
changing the underlying key that it opens. Adding the session key will further
ensure that neither password nor master key ever appears in RAM (for very
int64_t) nor on the harddrive in cleartext form.
 -- Adding the use of a standard protected memory API such as gpg-agent means
there is no risk of swapping or core dumps revealing vital information from
within OT itself. Nevertheless, the OTPassword class takes pains to prevent core
dumps and swapping, and also uses tricks to zero sensitive memory after it has
been used.
 -- The protected memory API has no access to what it is storing, due to the
session key. I will make this configurable so that users can choose their own
authentication mechanisms.
 -- Since a thread is used to implement a timer for wiping the master key after
X seconds, a mutex appears in the class below, which must be locked by any
functions in order to use the master key. (They do this behind the scenes
automatically.)
 -- An OT operation will behave in this way:  The user tries to withdraw cash,
and thus must sign a withdrawal request. An OpenSSL call is used to sign the
request, and the OT password callback is passed to the OpenSSL call, which
OpenSSL will call in the event that it needs a password. When that happens, OT
will, instead of popping up the password dialog to the user, will notice that
the master key password has been decrypted already and is within the X seconds
window, so it will use that password to unlock the master key, which it passes
to OpenSSL instead of asking the user to enter his passphrase. If the timer had
already been expired by that time, then the master key password would have been
destroyed already, meaning OT has no way to open the master key, so OT would
then pop up the passphrase dialog and ask the user to enter his passphrase, use
that to derive a key, and use that to unlock the master key password, and then
use that to unlock the master key and pass that back to OpenSSL via the callback
function.
 -- OT might do several operations within a SINGLE USE CASE. For example, for a
cash withdrawal, OT might sign the transaction item, sign the balance agreement
item, sign the main transaction containing those items, AND sign the message
containing that transaction. Perhaps OT has to use that private key 4 or 5 times
in a row, all within a fraction of a second. Should the user therefore have to
enter his passphrase 4 or 5 times, for a cash withdrawal? Isn't once enough? But
it's not that simple, because each operation opens the master key just int64_t
enough for OpenSSL to perform the requested operation, and then destroys it
again immediately after the operation is completed. (Even when the timeout is
active, what's being stored isn't the master key itself, but the master key
password used to open it JUST LONG ENOUGH to use it, and then destroy it again.)
The critical point to understand is that, especially with the introduction of
the session key, EVEN though a timeout is happening, the master key itself is
STILL destroyed after each operation, even 4 or 5 times in a row really fast,
for some single use case transaction to occur.
 -- If you set your timeout to -1, then the master key passphrase will never
expire. But if your timeout is set to 0, then you will have to type your
password 4 or 5 times for a single transaction. Get it? If you set it to 300
(seconds) then you will only have to type your passphrase once and then your
application will work for 5 minutes without having to type it again, not even
once more, until that 5 minutes are up, and then you will definitely have to
type that password again before doing another operation.
 -- This is an area where much can be gained through the study of code
obfuscation, randomizing memory locations, zeroing memory after use, using
protected memory APIs, using session keys, using outsourced authentication
systems such as ssh-agent instead of handling it internally, stack smashing
techniques such as canaries, etc etc. This is always a moving target. */

// This is only the hard-coded default; it's also configurable in the opt file.
#define OT_MASTER_KEY_TIMEOUT 300

typedef std::map<std::string, std::shared_ptr<OTCachedKey>> mapOfCachedKeys;

class OTCachedKey
{
private:
    /** Now we have many "master keys," mapped by their symmetric key ID. These
     * are actually temps, just so we can safely cache the passphrases for
     * various symmetric keys, between uses of that symmetric key. Such as
     * Pop'ing tokens off of a purse, over and over again. Normally in the API,
     * this would have to load the key each time. By caching here, we can
     * exploit all the cool master key code, with its security, and threads, and
     * timeouts, etc for every symmetric key we use. Just pass an ID into It()
     * and if it's on the map, a pointer will be returned. Pass nullptr into
     * It() (no arguments) to get a pointer to the global Master Key (for Nyms.)
     */
    static mapOfCachedKeys s_mapCachedKeys;
    static std::mutex s_mutexCachedKeys;
    static std::shared_ptr<OTCachedKey> singleton_;

    /** Mutex used for serializing access to this instance. */
    mutable std::mutex m_Mutex;
    mutable std::atomic<bool> shutdown_;
    /** if set to true, then additionally use the local OS's standard API for
     * storing/retrieving secrets. (Store the master key here whenever it's
     * decrypted, and try to retrieve from here whenever it's needed, before
     * resorting to asking the user to type his passphrase.) This is
     * configurable in the config file. */
    mutable std::atomic<bool> use_system_keyring_;

    /** If you want to force the old system, PAUSE the master key (REMEMBER to
     * Unpause when done!) */
    mutable std::atomic<bool> paused_;
    mutable std::atomic<std::time_t> time_;

    /** The master password will be stored internally for X seconds, and then
     * destroyed. */
    mutable std::atomic<std::uint64_t> timeout_;

    /** The thread used for destroying the password after the timeout period. */
    std::unique_ptr<std::thread> thread_;

    /** Created when password is passed in; destroyed by Timer after X seconds.
     */
    std::unique_ptr<OTPassword> master_password_;

    /** Encrypted form of the master key. Serialized by OTWallet or OTServer. */
    std::unique_ptr<OTSymmetricKey> key_;

    String secret_id_;

    std::int32_t GetTimeoutSeconds() const;
    void init(const std::int32_t& timeout = 0, const bool useKeyring = false)
        const;
    void LowLevelReleaseThread();

    /** If you actually want to create a new key, and a new passphrase, then use
     * this to destroy every last vestige of the old one. (Which will cause a
     * new one to be automatically generated the next time OT requests the
     * master key.)
     * NOTE: Make SURE you have all your Nyms loaded up and unlocked before you
     * call this. Then Save them all again so they will be properly stored with
     * the new master key. */
    void ResetMasterPassword(const std::lock_guard<std::mutex>& lock);
    void ResetTimer() const;

    /** The cleartext version (m_pMasterPassword) is deleted and set nullptr
     * after a Timer of X seconds. (Timer thread calls this.) The INSTANCE that
     * owns the thread also passes a pointer to ITSELF. (So we can access
     * password, mutex, timeout value, etc.) This function calls
     * DestroyMasterPassword. */
    void ThreadTimeout();

    OTCachedKey(std::int32_t nTimeoutSeconds = OT_MASTER_KEY_TIMEOUT);

public:
    EXPORT OTCachedKey(const OTASCIIArmor& ascCachedKey);
    EXPORT OTCachedKey(const OTCachedKey&);

    /** if you pass in a master key ID, it will look it up on an existing cached
     * map of master keys. Otherwise it will use "the" global Master Key (the
     * one used for the Nyms.) */
    EXPORT static std::shared_ptr<OTCachedKey> It(
        Identifier* pIdentifier = nullptr);

    /** if you pass in a master key, it will look it up on an existing cached
     * map of master keys, based on the ID of the master key passed in. If not
     * there, it copies the one passed in, and returns a pointer to the copy.
     * (Do NOT delete it.) */
    EXPORT static std::shared_ptr<OTCachedKey> It(OTCachedKey& theSourceKey);

    /** Call on application shutdown. Called in CleanupOTAPI and also in
     * OTServer wherever it cleans up. */
    EXPORT static void Cleanup();
    EXPORT bool GetIdentifier(Identifier& theIdentifier) const;
    EXPORT bool GetIdentifier(String& strIdentifier) const;
    EXPORT bool IsGenerated();
    EXPORT bool HasHashCheck();
    EXPORT bool IsUsingSystemKeyring() const;
    /** Start using system keyring. */
    EXPORT void UseSystemKeyring(const bool bUsing = true) const;
    EXPORT bool Pause() const;
    EXPORT bool Unpause() const;
    EXPORT bool isPaused() const;
    EXPORT bool SerializeTo(OTASCIIArmor& ascOutput);
    EXPORT bool SerializeFrom(const OTASCIIArmor& ascInput);

    /* These two functions are used by the OTServer or OTWallet that
     * actually keeps the master key. The owner sets the master key pointer on
     * initialization, and then later when the password callback code in
     * OTAsymmetricKey needs to access the master key, it can use
     * GetMasterPassword to access it. */

    /** OTServer/OTWallet calls this, I instantiate. */
    EXPORT void SetCachedKey(const OTASCIIArmor& ascCachedKey);

    /** So we can load from the config file. */
    EXPORT void SetTimeoutSeconds(std::int64_t nTimeoutSeconds);

    /** For Nyms, which have a global master key serving as their "passphrase"
     * (for that wallet), The password callback uses OTCachedKey::It() to get
     * the instance, and then GetMasterPassword to get the passphrase for any
     * individual Nym. Otherwise, OTCachedKey::It(OTSymmetricKey *) looks up a
     * cached master key based on the ID of the key passed in. For example,
     * OTPurse has a symmetric key and master key (optionally, vs using a Nym.)
     * The symmetric key contains the actual key for the tokens, and the master
     * key is used for the passphrase, which may be cached, or may have timed
     * out, and then re-retrieved from the user (either way). The purse, rather
     * than using the global master key to get the passphrase, (which _would_
     * happen if the purse is encrypted to a nym) will instead use its own
     * internal master key to get its passphrase (also retrieving from the user
     * if necessary.) */
    EXPORT bool GetMasterPassword(
        std::shared_ptr<OTCachedKey>& mySharedPtr,
        OTPassword& theOutput,
        const char* szDisplay = nullptr,
        bool bVerifyTwice = false);

    EXPORT static std::shared_ptr<OTCachedKey> CreateMasterPassword(
        OTPassword& theOutput,
        const char* szDisplay = nullptr,
        std::int32_t nTimeoutSeconds = OT_MASTER_KEY_TIMEOUT);

    /** GetMasterPassword USES the User Passphrase to decrypt the cached key and
     * return a decrypted plaintext of that cached symmetric key. Whereas
     * ChangeUserPassphrase CHANGES the User Passphrase that's used to encrypt
     * that cached key. The cached key itself is not changed, nor returned. It
     * is merely re-encrypted. */
    EXPORT bool ChangeUserPassphrase();

    EXPORT ~OTCachedKey();
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_OTCACHEDKEY_HPP
