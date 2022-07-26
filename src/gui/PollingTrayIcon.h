//
// Created by aszswaz on 2022/1/17.
//

#ifndef FAVOUR_POLLINGTRAYICON_H
#define FAVOUR_POLLINGTRAYICON_H

#include <QSystemTrayIcon>
#include <QApplication>
#include <QTimer>
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include "kibana/kibana.h"

using namespace nlohmann;

namespace favour {

    class PollingTrayIcon : public QSystemTrayIcon, public KEHandler {
        private:
            /**
             * info、warning、error 等级的日志获取统计（暂无去重）
             */
            unsigned int info = 0, warning = 0, error = 0;

        public:
            PollingTrayIcon();

            /**
             * 解析日志
             */
            void handler(KibanaLog *log) override;

            ~PollingTrayIcon() override;

        private:
            /**
             * 根据 LOG 等级，判断是否需要提示用户
             */
            void hint(KibanaLog *log);

            /**
             * 更新提示信息
             */
            void updateToolTip();
    };
}


#endif //FAVOUR_POLLINGTRAYICON_H
