//
// Created by aszswaz on 2022-01-25
//

#ifndef KIBANA_MANAGER_TEST_H
#define KIBANA_MANAGER_TEST_H

#include <string>

namespace favour {
    class KibanaManagerTest {
        public:
            /**
             * Kibana 管理器初始化测试
             */
            static void initTest();

            /**
             * KibanaManager 的事件处理器注册测试
             */
            static void registerHandlerTest();

            /**
             * 生成查询条件测试
             */
            static void toQuery();

            /**
             * 解析数据
             */
            static void parseData();
    };
}

#endif
