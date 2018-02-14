/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HttpEndPoints.hpp
 * Author: ubuntu
 *
 * Created on February 13, 2018, 11:48 AM
 */

#ifndef HTTPENDPOINTS_HPP
#define HTTPENDPOINTS_HPP

namespace keto {
namespace common {

class HttpEndPoints {
public:
    HttpEndPoints() = delete;
    HttpEndPoints(const HttpEndPoints& orig) = delete;
    virtual ~HttpEndPoints() = delete;
    
    
    static constexpr const char* HAND_SHAKE = "/hand_shake";
    
};
    
    
}
}

#endif /* HTTPENDPOINTS_HPP */

