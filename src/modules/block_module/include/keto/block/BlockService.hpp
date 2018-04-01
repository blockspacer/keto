/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BlockService.hpp
 * Author: ubuntu
 *
 * Created on March 8, 2018, 3:04 AM
 */

#ifndef BLOCKSERVICE_HPP
#define BLOCKSERVICE_HPP

#include <memory>

#include "keto/event/Event.hpp"

namespace keto {
namespace block {

class BlockService {
public:
    BlockService();
    BlockService(const BlockService& orig) = delete;
    virtual ~BlockService();
    
    static std::shared_ptr<BlockService> init();
    static void fin();
    static std::shared_ptr<BlockService> getInstance();
    
    void genesis();
    
    keto::event::Event blockMessage(const keto::event::Event& event);
private:

};


}
}

#endif /* BLOCKSERVICE_HPP */

