//
// Created by aszswaz on 2022/1/21.
//

#ifndef FAVOUR_CONFIG_H
#define FAVOUR_CONFIG_H

#include <vector>
#include <string>
#include "hook.h"
#include <memory>

#define DEV 0
#define PRO 1

using namespace std;

namespace favour {

    void configInit();

    /**
     * 索引配置对象
     */
    struct KibanaIndex {
        // 项目名称
        string project;
        // 索引名称
        string index;
        // 与索引相对应的 URL
        string indexUrl;
    };

    /**
     * 服务器配置对象
     */
    struct ServerConfig {
        // 服务器名称
        string name;
        // 服务器 web 首页
        string home;
        // 查询 log 的 http 接口
        string api;
        // LOG 索引
        vector<KibanaIndex *> *indices;

        ~ServerConfig() {
            for (auto item : *this->indices) {
                delete item;
            }
            delete this->indices;
        }
    };

    /**
     * 从 json 文件读取的全局配置
     */
    struct GlobalConfig {
        /**
         * 所有的服务器配置
         */
        vector<ServerConfig *> *servers;
        /**
         * 每隔 N 秒，从服务器获取一次日志
         */
        unsigned int timer;
        /**
         * 配置文件路径
         */
        string configDir;
        /**
         * 缓存文件路径
         */
        string cacheDir;

        ~GlobalConfig() {
            for (auto item : *this->servers) {
                delete item;
            }
            delete this->servers;
        }
    };

    extern const GlobalConfig *globalConfig;
}

#endif //FAVOUR_CONFIG_H
