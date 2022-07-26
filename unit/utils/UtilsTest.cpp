//
// util 测试
//
// Created by aszswaz on 2022-01-27
//

#include "UtilsTest.h"
#include <iostream>
#include "config/hook.h"

namespace favour {

    void curlUtilTest() {
        hookInit();
        CurlUtil::init();

        CurlUtil curl;
        shared_ptr<string> buffer;
        buffer = curl.post("https://example.com", nullptr);
        std::cout << *buffer << std::endl;
    }

}
