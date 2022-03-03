// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_Bitcoin_Providers : public ::testing::Test
{
public:
    static const bool have_hd_;

    const ot::api::session::Client& client_;
    ot::OTPasswordPrompt reason_;
    const ot::api::Crypto& crypto_;
    const ot::UnallocatedMap<ot::UnallocatedCString, ot::UnallocatedCString>
        base_58_{
            {"", ""},
            {"00010966776006953D5567439E5E39F86A0D273BEE",
             "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM"},
        };
    const ot::UnallocatedMap<ot::UnallocatedCString, ot::UnallocatedCString>
        ripemd160_{
            {"", "9c1185a5c5e9fc54612808977ee8f548b2258d31"},
            {"a", "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe"},
            {"abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"},
            {"message digest", "5d0689ef49d2fae572b881b123a85ffa21595f36"},
            {"abcdefghijklmnopqrstuvwxyz",
             "f71c27109c692c1b56bbdceb5b9d2865b3708dbc"},
            {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
             "12a053384a9c0c88e405a06c27dcf49ada62eb2b"},
            {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
             "b0e20b6e3116640286ed3a87a5713079b21f5189"},
            {"12345678901234567890123456789012345678901234567890123456789012345"
             "6789"
             "01234567890",
             "9b752e45573d4b39f4dbd3323cab82bf63326bfb"}};
    using Path = ot::UnallocatedVector<std::uint32_t>;
    using Bip32TestCase =
        std::tuple<Path, ot::UnallocatedCString, ot::UnallocatedCString>;
    using Bip32TestVector =
        std::pair<ot::UnallocatedCString, ot::UnallocatedVector<Bip32TestCase>>;
    const ot::UnallocatedVector<Bip32TestVector> bip_32_{
        {"0x000102030405060708090a0b0c0d0e0f",
         {
             {{},
              "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29E"
              "SFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8",
              "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVv"
              "vNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi"},
             {{
                  0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
              },
              "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhw"
              "BZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw",
              "xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rG"
              "L5hj6KCesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7"},
             {{0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED), 1},
              "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAW"
              "bWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ",
              "xprv9wTYmMFdV23N2TdNG573QoEsfRrWKQgWeibmLntzniatZvR9BmLnvSxqu53K"
              "w1UmYPxLgboyZQaXwTCg8MSY3H2EU4pWcQDnRnrVA1xe8fs"},
             {{0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               1,
               2 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED)},
              "xpub6D4BDPcP2GT577Vvch3R8wDkScZWzQzMMUm3PWbmWvVJrZwQY4VUNgqFJPMM"
              "3No2dFDFGTsxxpG5uJh7n7epu4trkrX7x7DogT5Uv6fcLW5",
              "xprv9z4pot5VBttmtdRTWfWQmoH1taj2axGVzFqSb8C9xaxKymcFzXBDptWmT7Fw"
              "uEzG3ryjH4ktypQSAewRiNMjANTtpgP4mLTj34bhnZX7UiM"},
             {{0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               1,
               2 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               2},
              "xpub6FHa3pjLCk84BayeJxFW2SP4XRrFd1JYnxeLeU8EqN3vDfZmbqBqaGJAyiLj"
              "TAwm6ZLRQUMv1ZACTj37sR62cfN7fe5JnJ7dh8zL4fiyLHV",
              "xprvA2JDeKCSNNZky6uBCviVfJSKyQ1mDYahRjijr5idH2WwLsEd4Hsb2Tyh8RfQ"
              "MuPh7f7RtyzTtdrbdqqsunu5Mm3wDvUAKRHSC34sJ7in334"},
             {{0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               1,
               2 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               2,
               1000000000},
              "xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFg"
              "JS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy",
              "xprvA41z7zogVVwxVSgdKUHDy1SKmdb533PjDz7J6N6mV6uS3ze1ai8FHa8kmHSc"
              "GpWmj4WggLyQjgPie1rFSruoUihUZREPSL39UNdE3BBDu76"},
         }},
        {"0xfffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9"
         "c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542",
         {
             {{},
              "xpub661MyMwAqRbcFW31YEwpkMuc5THy2PSt5bDMsktWQcFF8syAmRUapSCGu8ED"
              "9W6oDMSgv6Zz8idoc4a6mr8BDzTJY47LJhkJ8UB7WEGuduB",
              "xprv9s21ZrQH143K31xYSDQpPDxsXRTUcvj2iNHm5NUtrGiGG5e2DtALGdso3pGz"
              "6ssrdK4PFmM8NSpSBHNqPqm55Qn3LqFtT2emdEXVYsCzC2U"},
             {{
                  0,
              },
              "xpub69H7F5d8KSRgmmdJg2KhpAK8SR3DjMwAdkxj3ZuxV27CprR9LgpeyGmXUbC6"
              "wb7ERfvrnKZjXoUmmDznezpbZb7ap6r1D3tgFxHmwMkQTPH",
              "xprv9vHkqa6EV4sPZHYqZznhT2NPtPCjKuDKGY38FBWLvgaDx45zo9WQRUT3dKYn"
              "jwih2yJD9mkrocEZXo1ex8G81dwSM1fwqWpWkeS3v86pgKt"},
             {{0,
               2147483647 |
                   static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED)},
              "xpub6ASAVgeehLbnwdqV6UKMHVzgqAG8Gr6riv3Fxxpj8ksbH9ebxaEyBLZ85ySD"
              "hKiLDBrQSARLq1uNRts8RuJiHjaDMBU4Zn9h8LZNnBC5y4a",
              "xprv9wSp6B7kry3Vj9m1zSnLvN3xH8RdsPP1Mh7fAaR7aRLcQMKTR2vidYEeEg2m"
              "UCTAwCd6vnxVrcjfy2kRgVsFawNzmjuHc2YmYRmagcEPdU9"},
             {{0,
               2147483647 |
                   static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               1},
              "xpub6DF8uhdarytz3FWdA8TvFSvvAh8dP3283MY7p2V4SeE2wyWmG5mg5EwVvmdM"
              "VCQcoNJxGoWaU9DCWh89LojfZ537wTfunKau47EL2dhHKon",
              "xprv9zFnWC6h2cLgpmSA46vutJzBcfJ8yaJGg8cX1e5StJh45BBciYTRXSd25UEP"
              "VuesF9yog62tGAQtHjXajPPdbRCHuWS6T8XA2ECKADdw4Ef"},
             {{0,
               2147483647 |
                   static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               1,
               2147483646 |
                   static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED)},
              "xpub6ERApfZwUNrhLCkDtcHTcxd75RbzS1ed54G1LkBUHQVHQKqhMkhgbmJbZRkr"
              "gZw4koxb5JaHWkY4ALHY2grBGRjaDMzQLcgJvLJuZZvRcEL",
              "xprvA1RpRA33e1JQ7ifknakTFpgNXPmW2YvmhqLQYMmrj4xJXXWYpDPS3xz7iAxn"
              "8L39njGVyuoseXzU6rcxFLJ8HFsTjSyQbLYnMpCqE2VbFWc"},
             {{0,
               2147483647 |
                   static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               1,
               2147483646 |
                   static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
               2},
              "xpub6FnCn6nSzZAw5Tw7cgR9bi15UV96gLZhjDstkXXxvCLsUXBGXPdSnLFbdpq8"
              "p9HmGsApME5hQTZ3emM2rnY5agb9rXpVGyy3bdW6EEgAtqt",
              "xprvA2nrNbFZABcdryreWet9Ea4LvTJcGsqrMzxHx98MMrotbir7yrKCEXw7nadn"
              "HM8Dq38EGfSh6dqA9QWTyefMLEcBYJUuekgW4BYPJcr9E7j"},
         }},
        {"0x4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba4"
         "5d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be",
         {
             {{},
              "xpub661MyMwAqRbcEZVB4dScxMAdx6d4nFc9nvyvH3v4gJL378CSRZiYmhRoP7mB"
              "y6gSPSCYk6SzXPTf3ND1cZAceL7SfJ1Z3GC8vBgp2epUt13",
              "xprv9s21ZrQH143K25QhxbucbDDuQ4naNntJRi4KUfWT7xo4EKsHt2QJDu7KXp1A"
              "3u7Bi1j8ph3EGsZ9Xvz9dGuVrtHHs7pXeTzjuxBrCmmhgC6"},
             {{
                  0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
              },
              "xpub68NZiKmJWnxxS6aaHmn81bvJeTESw724CRDs6HbuccFQN9Ku14VQrADWgqbh"
              "hTHBaohPX4CjNLf9fq9MYo6oDaPPLPxSb7gwQN3ih19Zm4Y",
              "xprv9uPDJpEQgRQfDcW7BkF7eTya6RPxXeJCqCJGHuCJ4GiRVLzkTXBAJMu2qaMW"
              "PrS7AANYqdq6vcBcBUdJCVVFceUvJFjaPdGZ2y9WACViL4L"},
         }},
    };
    const ot::UnallocatedMap<
        ot::UnallocatedCString,
        std::tuple<
            ot::UnallocatedCString,
            ot::UnallocatedCString,
            ot::UnallocatedCString>>
        bip_39_{
            {"00000000000000000000000000000000",
             {"abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon abandon about",
              "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e5349553"
              "1f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b0"
              "4",
              "xprv9s21ZrQH143K3h3fDYiay8mocZ3afhfULfb5GX8kCBdno77K4HiA15Tg23wp"
              "beF1pLfs1c5SPmYHrEpTuuRhxMwvKDwqdKiGJS9XFKzUsAF"}},
            {"7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
             {"legal winner thank year wave sausage worth useful legal winner "
              "thank yellow",
              "2e8905819b8723fe2c1d161860e5ee1830318dbf49a83bd451cfb8440c28bd6f"
              "a457fe1296106559a3c80937a1c1069be3a3a5bd381ee6260e8d9739fce1f60"
              "7",
              "xprv9s21ZrQH143K2gA81bYFHqU68xz1cX2APaSq5tt6MFSLeXnCKV1RVUJt9FWN"
              "Tbrrryem4ZckN8k4Ls1H6nwdvDTvnV7zEXs2HgPezuVccsq"}},
            {"80808080808080808080808080808080",
             {"letter advice cage absurd amount doctor acoustic avoid letter "
              "advice cage above",
              "d71de856f81a8acc65e6fc851a38d4d7ec216fd0796d0a6827a3ad6ed5511a30"
              "fa280f12eb2e47ed2ac03b5c462a0358d18d69fe4f985ec81778c1b370b652a"
              "8",
              "xprv9s21ZrQH143K2shfP28KM3nr5Ap1SXjz8gc2rAqqMEynmjt6o1qboCDpxckq"
              "XavCwdnYds6yBHZGKHv7ef2eTXy461PXUjBFQg6PrwY4Gzq"}},
            {"ffffffffffffffffffffffffffffffff",
             {"zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong",
              "ac27495480225222079d7be181583751e86f571027b0497b5b5d11218e0a8a13"
              "332572917f0f8e5a589620c6f15b11c61dee327651a14c34e18231052e48c06"
              "9",
              "xprv9s21ZrQH143K2V4oox4M8Zmhi2Fjx5XK4Lf7GKRvPSgydU3mjZuKGCTg7UPi"
              "BUD7ydVPvSLtg9hjp7MQTYsW67rZHAXeccqYqrsx8LcXnyd"}},
            {"000000000000000000000000000000000000000000000000",
             {"abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon agent",
              "035895f2f481b1b0f01fcf8c289c794660b289981a78f8106447707fdd9666ca"
              "06da5a9a565181599b79f53b844d8a71dd9f439c52a3d7b3e8a79c906ac845f"
              "a",
              "xprv9s21ZrQH143K3mEDrypcZ2usWqFgzKB6jBBx9B6GfC7fu26X6hPRzVjzkqkP"
              "vDqp6g5eypdk6cyhGnBngbjeHTe4LsuLG1cCmKJka5SMkmU"}},
            {"7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
             {"legal winner thank year wave sausage worth useful legal winner "
              "thank year wave sausage worth useful legal will",
              "f2b94508732bcbacbcc020faefecfc89feafa6649a5491b8c952cede496c214a"
              "0c7b3c392d168748f2d4a612bada0753b52a1c7ac53c1e93abd5c6320b9e95d"
              "d",
              "xprv9s21ZrQH143K3Lv9MZLj16np5GzLe7tDKQfVusBni7toqJGcnKRtHSxUwbKU"
              "yUWiwpK55g1DUSsw76TF1T93VT4gz4wt5RM23pkaQLnvBh7"}},
            {"808080808080808080808080808080808080808080808080",
             {"letter advice cage absurd amount doctor acoustic avoid letter "
              "advice cage absurd amount doctor acoustic avoid letter always",
              "107d7c02a5aa6f38c58083ff74f04c607c2d2c0ecc55501dadd72d025b751bc2"
              "7fe913ffb796f841c49b1d33b610cf0e91d3aa239027f5e99fe4ce9e5088cd6"
              "5",
              "xprv9s21ZrQH143K3VPCbxbUtpkh9pRG371UCLDz3BjceqP1jz7XZsQ5EnNkYAEk"
              "feZp62cDNj13ZTEVG1TEro9sZ9grfRmcYWLBhCocViKEJae"}},
            {"ffffffffffffffffffffffffffffffffffffffffffffffff",
             {"zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo "
              "zoo when",
              "0cd6e5d827bb62eb8fc1e262254223817fd068a74b5b449cc2f667c3f1f985a7"
              "6379b43348d952e2265b4cd129090758b3e3c2c49103b5051aac2eaeb890a52"
              "8",
              "xprv9s21ZrQH143K36Ao5jHRVhFGDbLP6FCx8BEEmpru77ef3bmA928BxsqvVM27"
              "WnvvyfWywiFN8K6yToqMaGYfzS6Db1EHAXT5TuyCLBXUfdm"}},
            {"0000000000000000000000000000000000000000000000000000000000000000",
             {"abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon abandon abandon abandon abandon abandon art",
              "bda85446c68413707090a52022edd26a1c9462295029f2e60cd7c4f2bbd30971"
              "70af7a4d73245cafa9c3cca8d561a7c3de6f5d4a10be8ed2a5e608d68f92fcc"
              "8",
              "xprv9s21ZrQH143K32qBagUJAMU2LsHg3ka7jqMcV98Y7gVeVyNStwYS3U7yVVoD"
              "Z4btbRNf4h6ibWpY22iRmXq35qgLs79f312g2kj5539ebPM"}},
            {"7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
             {"legal winner thank year wave sausage worth useful legal winner "
              "thank year wave sausage worth useful legal winner thank year "
              "wave sausage worth title",
              "bc09fca1804f7e69da93c2f2028eb238c227f2e9dda30cd63699232578480a40"
              "21b146ad717fbb7e451ce9eb835f43620bf5c514db0f8add49f5d121449d3e8"
              "7",
              "xprv9s21ZrQH143K3Y1sd2XVu9wtqxJRvybCfAetjUrMMco6r3v9qZTBeXiBZkS8"
              "JxWbcGJZyio8TrZtm6pkbzG8SYt1sxwNLh3Wx7to5pgiVFU"}},
            {"8080808080808080808080808080808080808080808080808080808080808080",
             {"letter advice cage absurd amount doctor acoustic avoid letter "
              "advice cage absurd amount doctor acoustic avoid letter advice "
              "cage absurd amount doctor acoustic bless",
              "c0c519bd0e91a2ed54357d9d1ebef6f5af218a153624cf4f2da911a0ed8f7a09"
              "e2ef61af0aca007096df430022f7a2b6fb91661a9589097069720d015e4e982"
              "f",
              "xprv9s21ZrQH143K3CSnQNYC3MqAAqHwxeTLhDbhF43A4ss4ciWNmCY9zQGvAKUS"
              "qVUf2vPHBTSE1rB2pg4avopqSiLVzXEU8KziNnVPauTqLRo"}},
            {"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
             {"zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo "
              "zoo zoo zoo zoo zoo zoo zoo vote",
              "dd48c104698c30cfe2b6142103248622fb7bb0ff692eebb00089b32d22484e16"
              "13912f0a5b694407be899ffd31ed3992c456cdf60f5d4564b8ba3f05a69890a"
              "d",
              "xprv9s21ZrQH143K2WFF16X85T2QCpndrGwx6GueB72Zf3AHwHJaknRXNF37ZmDr"
              "tHrrLSHvbuRejXcnYxoZKvRquTPyp2JiNG3XcjQyzSEgqCB"}},
            {"9e885d952ad362caeb4efe34a8e91bd2",
             {"ozone drill grab fiber curtain grace pudding thank cruise elder "
              "eight picnic",
              "274ddc525802f7c828d8ef7ddbcdc5304e87ac3535913611fbbfa986d0c9e547"
              "6c91689f9c8a54fd55bd38606aa6a8595ad213d4c9c9f9aca3fb217069a4102"
              "8",
              "xprv9s21ZrQH143K2oZ9stBYpoaZ2ktHj7jLz7iMqpgg1En8kKFTXJHsjxry1JbK"
              "H19YrDTicVwKPehFKTbmaxgVEc5TpHdS1aYhB2s9aFJBeJH"}},
            {"6610b25967cdcca9d59875f5cb50b0ea75433311869e930b",
             {"gravity machine north sort system female filter attitude volume "
              "fold club stay feature office ecology stable narrow fog",
              "628c3827a8823298ee685db84f55caa34b5cc195a778e52d45f59bcf75aba68e"
              "4d7590e101dc414bc1bbd5737666fbbef35d1f1903953b66624f910feef245a"
              "c",
              "xprv9s21ZrQH143K3uT8eQowUjsxrmsA9YUuQQK1RLqFufzybxD6DH6gPY7NjJ5G"
              "3EPHjsWDrs9iivSbmvjc9DQJbJGatfa9pv4MZ3wjr8qWPAK"}},
            {"68a79eaca2324873eacc50cb9c6eca8cc68ea5d936f98787c60c7ebc74e6ce7c",
             {"hamster diagram private dutch cause delay private meat slide "
              "toddler razor book happy fancy gospel tennis maple dilemma loan "
              "word shrug inflict delay length",
              "64c87cde7e12ecf6704ab95bb1408bef047c22db4cc7491c4271d170a1b213d2"
              "0b385bc1588d9c7b38f1b39d415665b8a9030c9ec653d75e65f847d8fc1fc44"
              "0",
              "xprv9s21ZrQH143K2XTAhys3pMNcGn261Fi5Ta2Pw8PwaVPhg3D8DWkzWQwjTJfs"
              "kj8ofb81i9NP2cUNKxwjueJHHMQAnxtivTA75uUFqPFeWzk"}},
            {"c0ba5a8e914111210f2bd131f3d5e08d",
             {"scheme spot photo card baby mountain device kick cradle pact "
              "join borrow",
              "ea725895aaae8d4c1cf682c1bfd2d358d52ed9f0f0591131b559e2724bb234fc"
              "a05aa9c02c57407e04ee9dc3b454aa63fbff483a8b11de949624b9f1831a961"
              "2",
              "xprv9s21ZrQH143K3FperxDp8vFsFycKCRcJGAFmcV7umQmcnMZaLtZRt13QJDso"
              "S5F6oYT6BB4sS6zmTmyQAEkJKxJ7yByDNtRe5asP2jFGhT6"}},
            {"6d9be1ee6ebd27a258115aad99b7317b9c8d28b6d76431c3",
             {"horn tenant knee talent sponsor spell gate clip pulse soap "
              "slush warm silver nephew swap uncle crack brave",
              "fd579828af3da1d32544ce4db5c73d53fc8acc4ddb1e3b251a31179cdb71e853"
              "c56d2fcb11aed39898ce6c34b10b5382772db8796e52837b54468aeb312cfc3"
              "d",
              "xprv9s21ZrQH143K3R1SfVZZLtVbXEB9ryVxmVtVMsMwmEyEvgXN6Q84LKkLRmf4"
              "ST6QrLeBm3jQsb9gx1uo23TS7vo3vAkZGZz71uuLCcywUkt"}},
            {"9f6a2878b2520799a44ef18bc7df394e7061a224d2c33cd015b157d746869863",
             {"panda eyebrow bullet gorilla call smoke muffin taste mesh "
              "discover soft ostrich alcohol speed nation flash devote level "
              "hobby quick inner drive ghost inside",
              "72be8e052fc4919d2adf28d5306b5474b0069df35b02303de8c1729c9538dbb6"
              "fc2d731d5f832193cd9fb6aeecbc469594a70e3dd50811b5067f3b88b28c3e8"
              "d",
              "xprv9s21ZrQH143K2WNnKmssvZYM96VAr47iHUQUTUyUXH3sAGNjhJANddnhw3i3"
              "y3pBbRAVk5M5qUGFr4rHbEWwXgX4qrvrceifCYQJbbFDems"}},
            {"23db8160a31d3e0dca3688ed941adbf3",
             {"cat swing flag economy stadium alone churn speed unique patch "
              "report train",
              "deb5f45449e615feff5640f2e49f933ff51895de3b4381832b3139941c57b592"
              "05a42480c52175b6efcffaa58a2503887c1e8b363a707256bdd2b587b46541f"
              "5",
              "xprv9s21ZrQH143K4G28omGMogEoYgDQuigBo8AFHAGDaJdqQ99QKMQ5J6fYTMfA"
              "NTJy6xBmhvsNZ1CJzRZ64PWbnTFUn6CDV2FxoMDLXdk95DQ"}},
            {"8197a4a47f0425faeaa69deebc05ca29c0a5b5cc76ceacc0",
             {"light rule cinnamon wrap drastic word pride squirrel upgrade "
              "then income fatal apart sustain crack supply proud access",
              "4cbdff1ca2db800fd61cae72a57475fdc6bab03e441fd63f96dabd1f183ef5b7"
              "82925f00105f318309a7e9c3ea6967c7801e46c8a58082674c860a37b93eda0"
              "2",
              "xprv9s21ZrQH143K3wtsvY8L2aZyxkiWULZH4vyQE5XkHTXkmx8gHo6RUEfH3Jyr"
              "6NwkJhvano7Xb2o6UqFKWHVo5scE31SGDCAUsgVhiUuUDyh"}},
            {"066dca1a2bb7e8a1db2832148ce9933eea0f3ac9548d793112d9a95c9407efad",
             {"all hour make first leader extend hole alien behind guard "
              "gospel lava path output census museum junior mass reopen famous "
              "sing advance salt reform",
              "26e975ec644423f4a4c4f4215ef09b4bd7ef924e85d1d17c4cf3f136c2863cf6"
              "df0a475045652c57eb5fb41513ca2a2d67722b77e954b4b3fc11f7590449191"
              "d",
              "xprv9s21ZrQH143K3rEfqSM4QZRVmiMuSWY9wugscmaCjYja3SbUD3KPEB1a7QXJ"
              "oajyR2T1SiXU7rFVRXMV9XdYVSZe7JoUXdP4SRHTxsT1nzm"}},
            {"f30f8c1da665478f49b001d94c5fc452",
             {"vessel ladder alter error federal sibling chat ability sun "
              "glass valve picture",
              "2aaa9242daafcee6aa9d7269f17d4efe271e1b9a529178d7dc139cd18747090b"
              "f9d60295d0ce74309a78852a9caadf0af48aae1c6253839624076224374bc63"
              "f",
              "xprv9s21ZrQH143K2QWV9Wn8Vvs6jbqfF1YbTCdURQW9dLFKDovpKaKrqS3SEWsX"
              "Cu6ZNky9PSAENg6c9AQYHcg4PjopRGGKmdD313ZHszymnps"}},
            {"c10ec20dc3cd9f652c7fac2f1230f7a3c828389a14392f05",
             {"scissors invite lock maple supreme raw rapid void congress "
              "muscle digital elegant little brisk hair mango congress clump",
              "7b4a10be9d98e6cba265566db7f136718e1398c71cb581e1b2f464cac1ceedf4"
              "f3e274dc270003c670ad8d02c4558b2f8e39edea2775c9e232c7cb798b069e8"
              "8",
              "xprv9s21ZrQH143K4aERa2bq7559eMCCEs2QmmqVjUuzfy5eAeDX4mqZffkYwpzG"
              "QRE2YEEeLVRoH4CSHxianrFaVnMN2RYaPUZJhJx8S5j6puX"}},
            {"f585c11aec520db57dd353c69554b21a89b20fb0650966fa0a9d6f74fd989d8f",
             {"void come effort suffer camp survey warrior heavy shoot primary "
              "clutch crush open amazing screen patrol group space point ten "
              "exist slush involve unfold",
              "01f5bced59dec48e362f2c45b5de68b9fd6c92c6634f44d6d40aab69056506f0"
              "e35524a518034ddc1192e1dacd32c1ed3eaa3c3b131c88ed8e7e54c49a5d099"
              "8",
              "xprv9s21ZrQH143K39rnQJknpH1WEPFJrzmAqqasiDcVrNuk926oizzJDDQkdiTv"
              "NPr2FYDYzWgiMiC63YmfPAa2oPyNB23r2g7d1yiK6WpqaQS"}},
        };

    Test_Bitcoin_Providers()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , crypto_(client_.Crypto())
    {
    }

    bool test_base58_encode()
    {
        for (const auto& [key, value] : base_58_) {
            auto input = client_.Factory().Data();
            input->DecodeHex(key);
            const auto output = crypto_.Encode().IdentifierEncode(input);

            EXPECT_EQ(value, output);
        }

        return true;
    }

    bool test_base58_decode()
    {
        for (const auto& [key, value] : base_58_) {
            auto expected = client_.Factory().Data();
            expected->DecodeHex(key);
            const auto decoded = crypto_.Encode().IdentifierDecode(value);
            const auto output =
                client_.Factory().Data(decoded, ot::StringStyle::Raw);

            EXPECT_EQ(expected.get(), output.get());
        }

        return true;
    }

    bool test_ripemd160()
    {
        for (const auto& [preimage, hash] : ripemd160_) {
            auto input = client_.Factory().Data();
            auto output = client_.Factory().Data();

            EXPECT_TRUE(input->DecodeHex(hash));
            EXPECT_TRUE(crypto_.Hash().Digest(
                ot::crypto::HashType::Ripemd160,
                preimage,
                output->WriteInto()));
            EXPECT_EQ(output.get(), input.get());
        }

        return true;
    }

    auto get_seed(const ot::UnallocatedCString& hex) const -> ot::OTSecret
    {
        auto data = client_.Factory().Data();
        data->DecodeHex(hex);
        auto output = client_.Factory().SecretFromBytes(data->Bytes());

        return output;
    }

    bool test_bip32_seed(const ot::crypto::Bip32& library)
    {
        for (const auto& [hex, value] : bip_39_) {
            [[maybe_unused]] const auto& [words, node, xprv] = value;
            auto data = client_.Factory().Data();

            EXPECT_TRUE(data->DecodeHex(node));

            const auto seed = client_.Factory().SecretFromBytes(data->Bytes());
            const auto seedID = library.SeedID(seed->Bytes())->str();
            const auto serialized =
                library.DeriveKey(ot::EcdsaCurve::secp256k1, seed, {});
            auto pKey = client_.Crypto().Asymmetric().InstantiateKey(
                ot::crypto::key::asymmetric::Algorithm::Secp256k1,
                seedID,
                serialized,
                ot::crypto::key::asymmetric::Role::Sign,
                ot::crypto::key::EllipticCurve::DefaultVersion,
                reason_);

            EXPECT_TRUE(pKey);

            if (false == bool(pKey)) { return false; }

            const auto& key = *pKey;

            EXPECT_TRUE(key.HasPrivate());
            EXPECT_TRUE(key.HasPublic());
            EXPECT_EQ(
                ot::crypto::key::asymmetric::Algorithm::Secp256k1,
                key.keyType());
            EXPECT_TRUE(compare_private(library, xprv, key.Xprv(reason_)));
        }

        return true;
    }

    bool compare_private(
        const ot::crypto::Bip32& library,
        const ot::UnallocatedCString& lhs,
        const ot::UnallocatedCString& rhs) const
    {
        bool output{true};
        ot::Bip32Network lNetwork{}, rNetwork{};
        ot::Bip32Depth lDepth{}, rDepth{};
        ot::Bip32Fingerprint lParent{}, rParent{};
        ot::Bip32Index lIndex{}, rIndex{};
        auto lChainCode = client_.Factory().Data();
        auto rChainCode = client_.Factory().Data();
        auto lKey = client_.Factory().Secret(0);
        auto rKey = client_.Factory().Secret(0);
        output &= library.DeserializePrivate(
            lhs, lNetwork, lDepth, lParent, lIndex, lChainCode, lKey);

        EXPECT_TRUE(output);

        output &= library.DeserializePrivate(
            rhs, rNetwork, rDepth, rParent, rIndex, rChainCode, rKey);
        auto lKeyData = client_.Factory().Data(lKey->Bytes());
        auto rKeyData = client_.Factory().Data(rKey->Bytes());

        EXPECT_TRUE(output);
        EXPECT_EQ(lNetwork, rNetwork);
        EXPECT_EQ(lDepth, rDepth);
        EXPECT_EQ(lParent, rParent);
        EXPECT_EQ(lIndex, rIndex);
        EXPECT_EQ(lChainCode.get(), rChainCode.get());
        EXPECT_EQ(lKeyData.get(), rKeyData.get());

        output &= (lNetwork == rNetwork);
        output &= (lDepth == rDepth);
        output &= (lParent == rParent);
        output &= (lIndex == rIndex);
        output &= (lChainCode.get() == rChainCode.get());
        output &= (lKeyData.get() == rKeyData.get());

        return output;
    }

    bool compare_public(
        const ot::crypto::Bip32& library,
        const ot::UnallocatedCString& lhs,
        const ot::UnallocatedCString& rhs) const
    {
        bool output{true};
        ot::Bip32Network lNetwork{}, rNetwork{};
        ot::Bip32Depth lDepth{}, rDepth{};
        ot::Bip32Fingerprint lParent{}, rParent{};
        ot::Bip32Index lIndex{}, rIndex{};
        auto lChainCode = client_.Factory().Data();
        auto rChainCode = client_.Factory().Data();
        auto lKey = client_.Factory().Data();
        auto rKey = client_.Factory().Data();

        output &= library.DeserializePublic(
            lhs, lNetwork, lDepth, lParent, lIndex, lChainCode, lKey);

        EXPECT_TRUE(output);

        output &= library.DeserializePublic(
            rhs, rNetwork, rDepth, rParent, rIndex, rChainCode, rKey);

        EXPECT_TRUE(output);
        EXPECT_EQ(lNetwork, rNetwork);
        EXPECT_EQ(lDepth, rDepth);
        EXPECT_EQ(lParent, rParent);
        EXPECT_EQ(lIndex, rIndex);
        EXPECT_EQ(lChainCode.get(), rChainCode.get());
        EXPECT_EQ(lKey.get(), rKey.get());

        output &= (lNetwork == rNetwork);
        output &= (lDepth == rDepth);
        output &= (lParent == rParent);
        output &= (lIndex == rIndex);
        output &= (lChainCode.get() == rChainCode.get());
        output &= (lKey.get() == rKey.get());

        return output;
    }

    bool test_bip32_child_key(const ot::crypto::Bip32& library)
    {
        for (const auto& testVector : bip_32_) {
            const auto& [hex, cases] = testVector;

            for (const auto& testCase : cases) {
                const auto& [rawPath, expectPub, expectPrv] = testCase;
                const auto pSeed = get_seed(hex);
                const auto& seed = pSeed.get();
                const auto seedID = library.SeedID(seed.Bytes())->str();
                const auto serialized =
                    library.DeriveKey(ot::EcdsaCurve::secp256k1, seed, rawPath);
                auto pKey = client_.Crypto().Asymmetric().InstantiateKey(
                    ot::crypto::key::asymmetric::Algorithm::Secp256k1,
                    seedID,
                    serialized,
                    ot::crypto::key::asymmetric::Role::Sign,
                    ot::crypto::key::EllipticCurve::DefaultVersion,
                    reason_);

                EXPECT_TRUE(pKey);

                if (false == bool(pKey)) { continue; }

                const auto& key = *pKey;

                EXPECT_TRUE(key.HasPrivate());
                EXPECT_TRUE(key.HasPublic());
                EXPECT_EQ(
                    ot::crypto::key::asymmetric::Algorithm::Secp256k1,
                    key.keyType());
                EXPECT_TRUE(
                    compare_private(library, expectPrv, key.Xprv(reason_)));
                EXPECT_TRUE(
                    compare_public(library, expectPub, key.Xpub(reason_)));
            }
        }

        return true;
    }

    bool test_bip39(const ot::crypto::Bip32& library)
    {
        for (const auto& [hex, value] : bip_39_) {
            const auto& [words, seed, xprv] = value;
            auto data = ot::Data::Factory();

            EXPECT_TRUE(data->DecodeHex(hex));

            const auto entropy =
                client_.Factory().SecretFromBytes(data->Bytes());
            const auto passphrase = client_.Factory().SecretFromText("TREZOR");
            const auto targetWords = client_.Factory().SecretFromText(words);

            EXPECT_TRUE(data->DecodeHex(seed));

            const auto targetRoot =
                client_.Factory().SecretFromBytes(data->Bytes());
            auto calculatedWords = client_.Factory().Secret(0);
            auto calculatedRoot = client_.Factory().Secret(0);

            EXPECT_TRUE(crypto_.BIP39().SeedToWords(
                entropy, calculatedWords, ot::crypto::Language::en));

            crypto_.BIP39().WordsToSeed(
                client_,
                ot::crypto::SeedStyle::BIP39,
                ot::crypto::Language::en,
                calculatedWords,
                calculatedRoot,
                passphrase);

            EXPECT_EQ(targetWords, calculatedWords);
            EXPECT_EQ(targetRoot, calculatedRoot);
        }

        return true;
    }
};

const bool Test_Bitcoin_Providers::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1)

};

TEST_F(Test_Bitcoin_Providers, Common)
{
    EXPECT_TRUE(test_base58_encode());
    EXPECT_TRUE(test_base58_decode());
    EXPECT_TRUE(test_ripemd160());
    EXPECT_TRUE(test_bip39(crypto_.BIP32()));

    if (have_hd_) {
        EXPECT_TRUE(test_bip32_seed(crypto_.BIP32()));
        EXPECT_TRUE(test_bip32_child_key(crypto_.BIP32()));
    }
}
}  // namespace ottest
