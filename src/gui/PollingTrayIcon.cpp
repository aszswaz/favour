//
// Created by aszswaz on 2022/1/17.
//

#include <curl/curl.h>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include "exception/CurlException.hpp"
#include <ctime>
#include "PollingTrayIcon.h"
#include "config/config.h"

using namespace nlohmann;
using namespace std;

namespace favour {

    PollingTrayIcon::PollingTrayIcon() : QSystemTrayIcon(nullptr), KEHandler() {
    }

    /**
     * 处理日志信息
     */
    void PollingTrayIcon::handler(KibanaLog *log) {
        // 根据 LOG 等级，发出托盘消息
        this->hint(log);
        // 更新托盘提示信息
        this->updateToolTip();
    }

    /**
     * 根据 LOG 等级，判断是否需要提示用户
     */
    void PollingTrayIcon::hint(KibanaLog *log) {
        // 如果日志的级别达到 WARN 以上，显示消息弹窗
        if (log->level >= KIBANA_LOG_WARN) {
            string levelStr = KibanaManager::toLevelStr(log->level);
            string title;
            title.append(levelStr).append(": ").append(log->project);
            string content;
            content.append("server: ").append("<a href='").append(log->home).append("'>").append(log->serverName).append("</a>\n")
            .append("index: ").append(log->index).append("\n")
            .append("time: ").append(log->time).append("\n")
            .append(log->message);

            // 弹窗显示时间
            int time;
            // 谈传信息图标
            QSystemTrayIcon::MessageIcon mi;
            switch (log->level) {
            case KIBANA_LOG_WARN:
                mi = QSystemTrayIcon::Warning;
                time = 60;
                break;
            default:
                mi = QSystemTrayIcon::Critical;
                time = 0;
                break;
            }

            this->showMessage(
                tr(title.c_str()),
                tr(content.c_str()),
                mi, time
            );
        }
    }

    /**
     * 更新提示信息
     */
    void PollingTrayIcon::updateToolTip() {
        // 显示托盘图标的提示信息，目前内存中接收了多少日志
        string message;
        auto servers = globalConfig->servers;
        auto cacheLogs = KibanaManager::getCacheLogs();
        int info = 0, warn = 0, error = 0;
        for (auto cacheLog : *cacheLogs) {
            switch (cacheLog->level) {
            case KIBANA_LOG_INFO:
                info++;
                break;
            case KIBANA_LOG_WARN:
                warn++;
                break;
            default:
                error++;
                break;
            }
        }
        message.append("info: ").append(to_string(info))
        .append(", warn: ").append(to_string(warn))
        .append(", ERROR: ").append(to_string(error));
        this->setToolTip(tr(message.c_str()));
    }

    PollingTrayIcon::~PollingTrayIcon() {

    }

}

