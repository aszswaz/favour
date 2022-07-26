//
// 工具
// Create by aszswaz on 2022-01-27
//

#ifndef UTILS_H
#define UTILS_H

#include <curl/curl.h>
#include <string>
#include <iostream>
#include <memory>

using namespace std;

namespace favour {
    /**
     * libcurl 封装
     */
    class CurlUtil {
        private:
            CURL *client;
            curl_slist *header;

        public:
            CurlUtil();

            ~CurlUtil() {
                curl_easy_cleanup(this->client);
                curl_slist_free_all(this->header);
            }

        public:
            /**
             * 初始化 libcurl
             */
            static void init();

            /*
             * 关闭所有 curl
             */
            static void close();

            /**
             * 发送带 json 的 post 请求
             *
             * @param url       HTTP URL
             * @param body      请求体
             */
            shared_ptr<string> post(const char *url, const char *body);

        private:
            /**
             * libcurl 读取响应的回调函数
             */
            static size_t responseCallback(char *buffer, size_t size, size_t nitems, shared_ptr<string> point);
    };

    /**
     * 获得当前时间的毫秒时间戳
     */
    size_t currentTimeMillis();

    /**
     * 毫秒时间戳格式化为字符串
     */
    std::string ftimestamp(size_t timeMill);
}

#endif
