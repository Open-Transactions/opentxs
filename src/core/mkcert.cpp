/* Certificate creation. Demonstrates some certificate related
* operations.
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ANDROID
#include <openssl/bn.h>
#endif

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
}
#endif

#include <opentxs/core/crypto/mkcert.hpp>
#include <cassert>

bool safe_strcpy(char* dest, const char* src, size_t dest_size,
                 bool bZeroSource = false) // if true, initializes
                                           // the source buffer to
                                           // zero after the
                                           // copying is done.
{
    // Make sure they don't overlap.
    //
    assert(false == ((src > dest) && (src < (dest + dest_size))));

    const size_t src_length = strlen(src);

    assert(dest_size > src_length);

#ifdef _WIN32
    bool bSuccess = (0 == strcpy_s(dest, dest_size, src));
#else
    size_t src_cpy_length = strlcpy(dest, src, dest_size);
    bool bSuccess = (src_length == src_cpy_length);
#endif

    // Notice: we don't zero out the source unless we were successful (AND
    // unless we were asked to.)
    //
    if (bSuccess && bZeroSource) {
        memset(const_cast<char*>(src), 0, src_length);
    }

    return bSuccess;
}

#ifdef __cplusplus
extern "C" {
#endif

int32_t add_ext(X509* cert, int32_t nid, char* value);

static void callback(int32_t p, int32_t, void*)
{
    char c = 'B';

    if (p == 0) c = '.';
    if (p == 1) c = '+';
    if (p == 2) c = '*';
    if (p == 3) c = '\n';
    fputc(c, stderr);
}

int32_t mkcert(X509** x509p, EVP_PKEY** pkeyp, int32_t bits, int32_t serial,
               int32_t days)
{
    bool bCreatedKey = false;
    bool bCreatedX509 = false;
    X509* x = nullptr;
    EVP_PKEY* pk = nullptr;
    RSA* rsa = nullptr;
    X509_NAME* name = nullptr;

    if ((pkeyp == nullptr) || (*pkeyp == nullptr)) {
        if ((pk = EVP_PKEY_new()) == nullptr) {
            abort();
        }
        bCreatedKey = true;
    }
    else
        pk = *pkeyp;
    if ((x509p == nullptr) || (*x509p == nullptr)) {
        if ((x = X509_new()) == nullptr) {
            if (bCreatedKey) {
                EVP_PKEY_free(pk);
            }
            return (0);
        }

        bCreatedX509 = true;
    }
    else
        x = *x509p;

#ifdef ANDROID
    rsa = RSA_new();
    BIGNUM* e1 = BN_new();

    if ((nullptr == rsa) || (nullptr == e1)) abort(); // todo

    BN_set_word(e1, RSA_F4);

    if (!RSA_generate_key_ex(rsa, bits, e1, nullptr)) abort(); // todo

    BN_free(e1);
#else
    rsa = RSA_generate_key(bits, RSA_F4, callback, nullptr);
#endif
    if (!EVP_PKEY_assign_RSA(pk, rsa)) {
        abort();
    }
    rsa = nullptr;

    X509_set_version(x, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x), serial);
    X509_gmtime_adj(X509_get_notBefore(x), 0);
    X509_gmtime_adj(X509_get_notAfter(x),
                    static_cast<int64_t>(60 * 60 * 24 * days));
    X509_set_pubkey(x, pk);

    name = X509_get_subject_name(x);

    /* This function creates and adds the entry, working out the
     * correct string type and performing checks on its length.
     * Normally we'd check the return value for errors...
     */
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                               reinterpret_cast<const uint8_t*>("UK"), -1, -1,
                               0);
    X509_NAME_add_entry_by_txt(
        name, "CN", MBSTRING_ASC,
        reinterpret_cast<const uint8_t*>("OpenSSL Group"), -1, -1, 0);

    /* Its self signed so set the issuer name to be the same as the
     * subject.
     */
    X509_set_issuer_name(x, name);
    /* Add various extensions: standard extensions */

    char* szConstraints = new char[100]();
    char* szKeyUsage = new char[100]();
    char* szSubjectKeyID = new char[100]();
    char* szCertType = new char[100]();
    char* szComment = new char[100]();
    safe_strcpy(szConstraints, "critical,CA:TRUE", 99);
    safe_strcpy(szKeyUsage, "critical,keyCertSign,cRLSign", 99);
    safe_strcpy(szSubjectKeyID, "hash", 99);
    safe_strcpy(szCertType, "sslCA", 99);
    safe_strcpy(szComment, "example comment extension", 99);
    add_ext(x, NID_basic_constraints, szConstraints);
    add_ext(x, NID_key_usage, szKeyUsage);
    add_ext(x, NID_subject_key_identifier, szSubjectKeyID);
    add_ext(x, NID_netscape_cert_type,
            szCertType); // Some Netscape specific extensions
    add_ext(x, NID_netscape_comment,
            szComment); // Some Netscape specific extensions
    delete[] szConstraints;
    szConstraints = nullptr;
    delete[] szKeyUsage;
    szKeyUsage = nullptr;
    delete[] szSubjectKeyID;
    szSubjectKeyID = nullptr;
    delete[] szCertType;
    szCertType = nullptr;
    delete[] szComment;
    szComment = nullptr;

#ifdef CUSTOM_EXT
    // Maybe even add our own extension based on existing
    {
        int32_t nid;
        nid = OBJ_create("1.2.3.4", "MyAlias", "My Test Alias Extension");
        X509V3_EXT_add_alias(nid, NID_netscape_comment);
        add_ext(x, nid, "example comment alias");
    }
#endif
    if (!X509_sign(x, pk, EVP_md5()) || // TODO security:  md5 ???
        (nullptr == x509p) || (nullptr == pkeyp)) {
        // ERROR
        //
        if (bCreatedX509) X509_free(x);

        // NOTE: not sure if x owns pk, in which case pk is already freed above.
        // Todo: find out and then determine whether or not to uncomment this.
        // (Presumably this would be a rare occurrence anyway.)
        //
        //            if (bCreatedKey)
        //                EVP_PKEY_free(pk);

        x = nullptr;
        pk = nullptr;

        return 0;
    }
    *x509p = x;
    *pkeyp = pk;

    return (1);
}

/* Add extension using V3 code: we can set the config file as nullptr
 * because we won't reference any other sections.
 */

int32_t add_ext(X509* cert, int32_t nid, char* value)
{
    X509_EXTENSION* ex;
    X509V3_CTX ctx;
    /* This sets the 'context' of the extensions. */
    /* No configuration database */
    X509V3_set_ctx_nodb(&ctx);
    /* Issuer and subject certs: both the target since it is self signed,
     * no request and no CRL
     */
    X509V3_set_ctx(&ctx, cert, cert, nullptr, nullptr, 0);
    ex = X509V3_EXT_conf_nid(nullptr, &ctx, nid, value);

    if (!ex) return 0;

    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    return 1;
}

#ifdef __cplusplus
} // closing brace for extern "C"
#endif
