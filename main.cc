/*
 * main.cc
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */
#include "GConf.h"
#include <iostream>

int main(void) {
    std::vector<GConf *> gConfVec;
    for (int i =0; i< 100;i++) {
        std::vector<std::string> ip_list = {std::string("0.0.0.0"), std::string("1.1.1.1")};
        GConf *gConf = new GConf("cluster-name" + std::to_string(i), std::move(ip_list));
        gConfVec.emplace_back(gConf);
    }
    for (auto &&gConf:gConfVec) {
        std::cout << gConf->cluster_name_ << std::endl;
        std::cout << gConf->ip_list_[0]<< std::endl;
    }
}



