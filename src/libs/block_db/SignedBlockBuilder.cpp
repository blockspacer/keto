/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SignedBlockBuilder.cpp
 * Author: ubuntu
 * 
 * Created on March 14, 2018, 2:51 PM
 */

#include "keto/block_db/SignedBlockBuilder.hpp"

#include "keto/asn1/SerializationHelper.hpp"
#include "keto/asn1/BerEncodingHelper.hpp"
#include "keto/crypto/SignatureGenerator.hpp"
#include "keto/crypto/HashGenerator.hpp"

#include "keto/block_db/Exception.hpp"

namespace keto {
namespace block_db {


SignedBlockBuilder::SignedBlockBuilder() {
    this->signedBlock = (SignedBlock_t*)calloc(1, sizeof *signedBlock);
    this->signedBlock->date = keto::asn1::TimeHelper();
}

SignedBlockBuilder::SignedBlockBuilder(
    const keto::asn1::PrivateKeyHelper& privateKeyHelper) 
        : privateKeyHelper(privateKeyHelper) {
    this->signedBlock = (SignedBlock_t*)calloc(1, sizeof *signedBlock);
    this->signedBlock->date = keto::asn1::TimeHelper();
}

SignedBlockBuilder::SignedBlockBuilder(Block_t* block,
    const keto::asn1::PrivateKeyHelper& privateKeyHelper) 
        : privateKeyHelper(privateKeyHelper) {
    this->signedBlock = (SignedBlock_t*)calloc(1, sizeof *signedBlock);
    this->signedBlock->date = keto::asn1::TimeHelper();
    this->signedBlock->block = *block;
    this->signedBlock->parent = block->parent;
    this->signedBlock->hash = getBlockHash(block);
    free(block);
}

SignedBlockBuilder::~SignedBlockBuilder() {
    if (this->signedBlock) {
        ASN_STRUCT_FREE(asn_DEF_SignedBlock, signedBlock);
        signedBlock = NULL;
    }
}


SignedBlockBuilder& SignedBlockBuilder::setPrivateKey(
        const keto::asn1::PrivateKeyHelper& privateKeyHelper) {
    this->privateKeyHelper = privateKeyHelper;
    return (*this);
}

SignedBlockBuilder& SignedBlockBuilder::setBlock(Block_t* block) {
    if (!this->signedBlock) {
        BOOST_THROW_EXCEPTION(keto::block_db::SignedBlockReleasedException());
    }
    this->signedBlock->block = *block;
    this->signedBlock->parent = block->parent;
    this->signedBlock->hash = getBlockHash(block);
    free(block);
    return (*this);
}

SignedBlockBuilder& SignedBlockBuilder::sign() {
    if (!this->signedBlock) {
        BOOST_THROW_EXCEPTION(keto::block_db::SignedBlockReleasedException());
    }
    keto::asn1::BerEncodingHelper key = this->privateKeyHelper.getKey();
    keto::crypto::SignatureGenerator generator((keto::crypto::SecureVector)key);
    keto::asn1::HashHelper hashHelper(this->signedBlock->hash);
    keto::asn1::SignatureHelper signatureHelper(generator.sign(hashHelper));
    this->signedBlock->signature = signatureHelper;
    return (*this);
    
}


SignedBlockBuilder::operator SignedBlock_t*() {
    SignedBlock_t* result = this->signedBlock;
    this->signedBlock = 0;
    return result;
}

SignedBlockBuilder::operator SignedBlock_t&() {
    return *this->signedBlock;
}


keto::asn1::HashHelper SignedBlockBuilder::getBlockHash(Block_t* block) {
    keto::asn1::SerializationHelper<Block_t> serializationHelper(block, &asn_DEF_Block);
    return keto::asn1::HashHelper(
        keto::crypto::HashGenerator().generateHash(
        serializationHelper.operator std::vector<uint8_t>&()));
}


}
}