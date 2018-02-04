/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HashHelper.cpp
 * Author: Brett Chaldecott
 * 
 * Created on February 2, 2018, 4:03 AM
 */

#include <stdlib.h>

#include <botan/hex.h>
#include <botan/base64.h>

#include "keto/asn1/HashHelper.hpp"
#include "keto/asn1/Exception.hpp"

namespace keto {
namespace asn1 {
    
HashHelper::HashHelper() {
}

HashHelper::HashHelper(const Botan::secure_vector<uint8_t>& hash) {
    this->hash = hash;
}

HashHelper::HashHelper(const Hash_t& hash) {
    copyHashToVector(hash,this->hash);
}


HashHelper::HashHelper(const std::string& hash,keto::common::StringEncoding stringEncoding) {
    if (stringEncoding == keto::common::HEX) {
        this->hash = Botan::hex_decode_locked(hash,true);
    } else if (stringEncoding == keto::common::BASE64) {
        this->hash = Botan::base64_decode(hash,true);
    } else {
        BOOST_THROW_EXCEPTION(keto::asn1::UnsupportedStringFormatException());
    }
}


HashHelper::~HashHelper() {
}

HashHelper& HashHelper::operator=(const Hash_t* hash) {
    this->hash.clear();
    copyHashToVector(*hash,this->hash);
    return (*this);
}


HashHelper& HashHelper::operator=(const Hash_t& hash) {
    this->hash.clear();
    copyHashToVector(hash,this->hash);
    return (*this);
}

HashHelper::operator Hash_t() const {
    Hash_t hashT;
    hashT.buf = (uint8_t*)malloc(this->hash.size());
    hashT.size = this->hash.size();
    for (int index = 0; index < this->hash.size(); index++) {
        hashT.buf[index] = this->hash[index];
    }
    return hashT;
}


HashHelper& HashHelper::operator =(const Botan::secure_vector<uint8_t>& hash) {
    this->hash = hash;
    return (*this);
}


HashHelper::operator Botan::secure_vector<uint8_t>() const {
    return this->hash;
}


HashHelper& HashHelper::setHash(const std::string& hash,keto::common::StringEncoding stringEncoding) {
    if (stringEncoding == keto::common::HEX) {
        this->hash = Botan::hex_decode_locked(hash,true);
    } else if (stringEncoding == keto::common::BASE64) {
        this->hash = Botan::base64_decode(hash,true);
    } else {
        BOOST_THROW_EXCEPTION(keto::asn1::UnsupportedStringFormatException());
    }
    return (*this);
}


std::string HashHelper::getHash(keto::common::StringEncoding stringEncoding) {
    if (stringEncoding == keto::common::HEX) {
        return Botan::hex_encode(this->hash,true);
    } else if (stringEncoding == keto::common::BASE64) {
        return Botan::base64_encode(this->hash);
    } else {
        BOOST_THROW_EXCEPTION(keto::asn1::UnsupportedStringFormatException());
    }
}


void HashHelper::copyHashToVector(const Hash_t& hash,Botan::secure_vector<uint8_t>& vector) {
    for (int index = 0; index < hash.size; index++) {
        vector.push_back(hash.buf[index]);
    }
}


}
}