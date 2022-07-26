//
// 向 kibana 服务器请求日志
// Create by aszswaz on 2022-01-23
//

#ifndef KIBANA_H
#define KIBANA_H

#include <string>
#include <vector>
#include <thread>
#include "utils/utils.h"
#include <nlohmann/json.hpp>
#include "config/config.h"

namespace favour {
    /**
     * 日志等级
     */
    enum KIBANA_LOG_LEVEL {
        KIBANA_LOG_DEBUG = 0,
        KIBANA_LOG_INFO = 1,
        KIBANA_LOG_WARN = 2,
        KIBANA_LOG_ERROR = 3
    };

    /**
     * kibana 服务器搜索结果
     */
    struct KibanaLog {
        std::string id;
        /**
         * kibana 服务器名称
         */
        std::string serverName;
        /**
         * 项目名称
         */
        std::string project;
        /**
         * log 的索引名称
         */
        std::string index;
        /**
         * 索引对应的地址
         */
        std::string indexUrl;
        /**
         * 服务器的主页地址
         */
        std::string home;
        /**
         * 日志等级
         */
        KIBANA_LOG_LEVEL level;
        /**
         * 日志时间
         */
        std::string time;
        /**
         * 日志信息
         */
        std::string message;
    };


    /**
     * Kibana 事件处理器
     */
    class KEHandler {
        public:
            // 处理器的抽象函数
            virtual void handler(KibanaLog *kl) {}

            virtual ~KEHandler(){}
    };

    /**
     * Kibana 全局管理器，负责管理所有的访问 Kibana 的操作，并允许注册处理器，在发生特定的事件时，通过回调处理器，完成特定的操作
     */
    class KibanaManager {
        private:
            static std::vector<KEHandler *> *handlers;
            static CurlUtil *curlUtil;
            /**
             * 上一次查询时间，单位：毫秒
             */
            static size_t lastTime;
            /**
             * 缓存日志
             */
            static vector<KibanaLog *> *cacheLogs;
            /**
             * 缓存文件
             */
            static string cacheFile;

        public:
            /**
             * 初始化 Kibana 管理器
             */
            static void init();

            /**
             * 清理的 kibana 管理器的资源
             */
            static void close();

            /**
             * 注册处理器
             */
            static void registerHandler(KEHandler *handler);

            /**
             * 日志等级字符串转枚举
             */
            static KIBANA_LOG_LEVEL toLevelEnum(std::string &levelStr);

            /**
             * 日志等级枚举转字符串
             */
            static std::string toLevelStr(KIBANA_LOG_LEVEL level);

            /**
             * 程序运行期间获得的日志
             */
            static vector<KibanaLog *> *getCacheLogs() {
                return cacheLogs;
            }

        private:
            /**
             * 定时从 Kibana 服务器获取日志
             */
            static void polling(void);

            /**
             * 从 kibana 服务器请求日志
             */
            static std::vector<KibanaLog *> *requestLog();

            /**
             * 生成日志的查询条件
             */
            static nlohmann::json toQuery(std::string &index, size_t startTime, size_t endTime);

            /**
             * 解析日志数据
             */
            static std::vector<KibanaLog *> *parseLog(ServerConfig *config, KibanaIndex *kindex, nlohmann::json &data);
    };
}

#endif
