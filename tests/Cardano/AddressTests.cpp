// Copyright © 2017-2019 Trust.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Cardano/Address.h"

#include "HexCoding.h"
#include "PublicKey.h"
#include "PrivateKey.h"
#include "Base58.h"
#include "HDWallet.h"

#include <gtest/gtest.h>
#include <vector>

using namespace TW;
using namespace TW::Cardano;
using namespace std;

TEST(CardanoAddress, Validation) {
    // valid V1 address
    ASSERT_TRUE(Address::isValid("DdzFFzCqrhssmYoG5Eca1bKZFdGS8d6iag1mU4wbLeYcSPVvBNF2wRG8yhjzQqErbg63N6KJA4DHqha113tjKDpGEwS5x1dT2KfLSbSJ"));
    // valid V2 address
    ASSERT_TRUE(Address::isValid("Ae2tdPwUPEZ18ZjTLnLVr9CEvUEUX4eW1LBHbxxxJgxdAYHrDeSCSbCxrvx"));
    // valid V2 address
    ASSERT_TRUE(Address::isValid("Ae2tdPwUPEZ6RUCnjGHFqi59k5WZLiv3HoCCNGCW8SYc5H9srdTzn1bec4W"));

    // invalid checksum
    ASSERT_FALSE(Address::isValid("Ae2tdPwUPEZ18ZjTLnLVr9CEvUEUX4eW1LBHbxxxJgxdAYHrDeSCSbCxrvm"));
    // random
    ASSERT_FALSE(Address::isValid("hasoiusaodiuhsaijnnsajnsaiussai"));
    // empty
    ASSERT_FALSE(Address::isValid(""));
}

TEST(CardanoAddress, FromString) {
    {
        auto address = Address("Ae2tdPwUPEZ18ZjTLnLVr9CEvUEUX4eW1LBHbxxxJgxdAYHrDeSCSbCxrvx");
        ASSERT_EQ(address.string(), "Ae2tdPwUPEZ18ZjTLnLVr9CEvUEUX4eW1LBHbxxxJgxdAYHrDeSCSbCxrvx");
    }
    {
        auto address = Address("DdzFFzCqrhssmYoG5Eca1bKZFdGS8d6iag1mU4wbLeYcSPVvBNF2wRG8yhjzQqErbg63N6KJA4DHqha113tjKDpGEwS5x1dT2KfLSbSJ");
        ASSERT_EQ(address.string(), "DdzFFzCqrhssmYoG5Eca1bKZFdGS8d6iag1mU4wbLeYcSPVvBNF2wRG8yhjzQqErbg63N6KJA4DHqha113tjKDpGEwS5x1dT2KfLSbSJ");
    }
}

TEST(CardanoAddress, MnemonicToKeys) {
    {
        // Test from cardano-crypto.js; Test wallet
        auto mnemonic = "cost dash dress stove morning robust group affair stomach vacant route volume yellow salute laugh";
        auto wallet = HDWallet(mnemonic, "");

        PrivateKey masterPrivKey = wallet.getMasterKey(TWCurve::TWCurveED25519Cardano);
        PrivateKey masterPrivKeyExt = wallet.getMasterKeyExtension(TWCurve::TWCurveED25519Cardano);
        ASSERT_EQ("a018cd746e128a0be0782b228c275473205445c33b9000a33dd5668b430b5744", hex(masterPrivKey.bytes));
        ASSERT_EQ("26877cfe435fddda02409b839b7386f3738f10a30b95a225f4b720ee71d2505b", hex(masterPrivKeyExt.bytes));

        {
            string addr = wallet.deriveAddress(TWCoinType::TWCoinTypeCardano);
            ASSERT_EQ("Ae2tdPwUPEZ6RUCnjGHFqi59k5WZLiv3HoCCNGCW8SYc5H9srdTzn1bec4W", addr);
        }
        {
            PrivateKey privKey0 = wallet.getKey(DerivationPath("m/44'/1815'/0'/0/0"));
            cerr << "privKey0 " << hex(privKey0.bytes) << endl;
            PublicKey pubKey0 = privKey0.getPublicKey(TWPublicKeyTypeED25519Ext);
            cerr << "pubKey0 " << hex(pubKey0.bytes) << endl;
            Address addr0 = Address(pubKey0);
            cerr << "addr0 " << addr0.string() << endl;
            ASSERT_EQ("Ae2tdPwUPEZ6RUCnjGHFqi59k5WZLiv3HoCCNGCW8SYc5H9srdTzn1bec4W", addr0.string());
        }
        {
            PrivateKey privKey1 = wallet.getKey(DerivationPath("m/44'/1815'/0'/0/1"));
            PublicKey pubKey1 = privKey1.getPublicKey(TWPublicKeyTypeED25519Ext);
            Address addr1 = Address(pubKey1);
            ASSERT_EQ("Ae2tdPwUPEZ7dnds6ZyhQdmgkrDFFPSDh8jG9RAhswcXt1bRauNw5jczjpV", addr1.string());
        }
    }
    {
        // Tested agains AdaLite
        auto mnemonicPlay1 = "youth away raise north opinion slice dash bus soldier dizzy bitter increase saddle live champion";
        auto wallet = HDWallet(mnemonicPlay1, "");
        string addr = wallet.deriveAddress(TWCoinType::TWCoinTypeCardano);
        ASSERT_EQ("Ae2tdPwUPEZJYT9g1JgQWtLveUHavyRxQGi6hVzoQjct7yyCLGgk3pCyx7h", addr);
    }
}

TEST(CardanoAddress, KeyHash) {
    auto xpub = parse_hex("e6f04522f875c1563682ca876ddb04c2e2e3ae718e3ff9f11c03dd9f9dccf69869272d81c376382b8a87c21370a7ae9618df8da708d1a9490939ec54ebe43000");
    auto hash = Address::keyHash(xpub);
    ASSERT_EQ("a1eda96a9952a56c983d9f49117f935af325e8a6c9d38496e945faa8", hex(hash));
}

TEST(CardanoAddress, FromPublicKey) {
    {
        // caradano-crypto.js test
        auto publicKey = PublicKey(parse_hex("e6f04522f875c1563682ca876ddb04c2e2e3ae718e3ff9f11c03dd9f9dccf69869272d81c376382b8a87c21370a7ae9618df8da708d1a9490939ec54ebe43000"), TWPublicKeyTypeED25519Ext);
        auto address = Address(publicKey);
        ASSERT_EQ(address.string(), "Ae2tdPwUPEZCxt4UV1Uj2AMMRvg5pYPypqZowVptz3GYpK4pkcvn3EjkuNH");
    }
    {
        // Adalite test account addr1
        auto publicKey = PublicKey(parse_hex("57fd54be7b38bb8952782c2f59aa276928a4dcbb66c8c62ce44f9d623ecd5a03bf36a8fa9f5e11eb7a852c41e185e3969d518e66e6893c81d3fc7227009952d4"), TWPublicKeyTypeED25519Ext);
        auto address = Address(publicKey);
        ASSERT_EQ(address.string(), "Ae2tdPwUPEZ6RUCnjGHFqi59k5WZLiv3HoCCNGCW8SYc5H9srdTzn1bec4W");
    }
    {
        // Adalite test account addr2
        auto publicKey = PublicKey(parse_hex("25af99056d600f7956312406bdd1cd791975bb1ae91c9d034fc65f326195fcdb247ee97ec351c0820dd12de4ca500232f73a35fe6f86778745bcd57f34d1048d"), TWPublicKeyTypeED25519Ext);
        auto address = Address(publicKey);
        ASSERT_EQ(address.string(), "Ae2tdPwUPEZ7dnds6ZyhQdmgkrDFFPSDh8jG9RAhswcXt1bRauNw5jczjpV");
    }
    {
        // Play1 addr1
        auto publicKey = PublicKey(parse_hex("7cee0f30b9d536a786547dd77b35679b6830e945ffde768eb4f2a061b9dba016e513fa1290da1d22e83a41f17eed72d4489483b561fff36b9555ffdb91c430e2"), TWPublicKeyTypeED25519Ext);
        auto address = Address(publicKey);
        ASSERT_EQ(address.string(), "Ae2tdPwUPEZJYT9g1JgQWtLveUHavyRxQGi6hVzoQjct7yyCLGgk3pCyx7h");
    }
}

/*
TEST(CardanoAddress, FromPrivateKey) {
    auto privateKey = PrivateKey(parse_hex("526d96fffdbfe787b2f00586298538f9a019e97f6587964dc61aae9ad1d7fa23"));
    auto address = Address(privateKey.getPublicKey(TWPublicKeyTypeED25519));
    ASSERT_EQ(address.string(), "JBCQYJ2FREG667NAN7BFKH4RFIKPT7CYDQJNW3SNN5Z7F7ILFLKQ346TSU");
}
*/
