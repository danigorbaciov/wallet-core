// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Extrinsic.h"
#include "ScaleCodec.h"

using namespace TW;
using namespace TW::Polkadot;

static constexpr uint8_t signedBit = 128;
static constexpr uint8_t sigType = 0x00;

Data Extrinsic::encodeEraNonceTip() const {
    Data data;
    // immortal era
    append(data, encodeCompact(0));
    // nonce
    append(data, encodeCompact(nonce));
    // tip
    append(data, encodeCompact(tip));
    return data;
}

Data Extrinsic::encodeCall() const {
    Data data;
    switch (type) {
    case BalanceTransfer:
        // call index
        append(data, {0x04, 0x00});
        // destination
        append(data, encodeAddress(address));
        // value
        append(data, encodeCompact(value));
    }
    return data;
}

Data Extrinsic::encodePayload() const {
    Data data;
    // call
    append(data, encodeCall());
    // era/nonce/tip
    append(data, encodeEraNonceTip());
    // specVersion
    encode32LE(specVersion, data);
    // genesis hash
    append(data, genesisHash);
    // block hash
    append(data, blockHash);
    return data;
}

Data Extrinsic::encodeSignature(const PublicKey& signer, const Data& signature) const {
    Data data;
    // version header
    append(data, Data{ static_cast<uint8_t>(version | signedBit)});
    // signer public key
    append(data, encodeAddress(signer));
    // signature type
    append(data, sigType);
    // signature
    append(data, signature);
    // immortal era
    append(data, encodeCompact(0));
    // nonce
    append(data, encodeCompact(nonce));
    // tip
    append(data, encodeCompact(tip));
    // call
    append(data, encodeCall());
    // append length
    encodeLengthPrefix(data);
    return data;
}
