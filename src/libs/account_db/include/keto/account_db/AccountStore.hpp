/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AccountStore.hpp
 * Author: ubuntu
 *
 * Created on March 5, 2018, 6:02 AM
 */

#ifndef ACCOUNTSTORE_HPP
#define ACCOUNTSTORE_HPP

#include "Account.pb.h"

#include "keto/asn1/HashHelper.hpp"
#include "keto/rocks_db/DBManager.hpp"
#include "keto/account_db/AccountResourceManager.hpp"
#include "keto/account_db/AccountGraphStoreManager.hpp"
#include "keto/transaction_common/TransactionMessageHelper.hpp"

namespace keto {
namespace account_db {


class AccountStore {
public:
    AccountStore(const AccountStore& orig) = delete;
    virtual ~AccountStore();
    
    static std::shared_ptr<AccountStore> init();
    static void fin();
    static std::shared_ptr<AccountStore> getInstance();
    
    bool getAccountInfo(const keto::asn1::HashHelper& accountHash,
        keto::proto::AccountInfo& result);
    void applyTransaction(
        const keto::transaction_common::TransactionMessageHelperPtr& transactionMessageHelper);
    
    
private:
    std::shared_ptr<keto::rocks_db::DBManager> dbManagerPtr;
    AccountGraphStoreManagerPtr accountGraphStoreManagerPtr;
    AccountResourceManagerPtr accountResourceManagerPtr;
    
    AccountStore();
    
};


}
}

#endif /* ACCOUNTSTORE_HPP */

