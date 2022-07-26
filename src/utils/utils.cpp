//
// Created by aszswaz on 2022-01-27
//

#include "utils/utils.h"
#include <exception>
#include <iostream>
#include <cstring>
#include "config/hook.h"
#include <ctime>

using namespace std;

namespace favour {
    void CurlUtil::init() {
        cout << "libcurl init..." << endl;
        // 初始化 libcurl
        CURLcode curlCode = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (curlCode) {
            string message = "libcurl init failed: ";
            message.append(curl_easy_strerror(curlCode));
            throw std::runtime_error(message);
        }
        // 程序退出时关闭所有的 curl 句柄和连接
        registerHook(CurlUtil::close);
        cout << "libcurl init success." << endl;
    }

    CurlUtil::CurlUtil() {
       CURL *client = curl_easy_init();
       if (!client) {
           throw std::runtime_error("libcurl init failed.");
       }

        // 启用压缩
        curl_easy_setopt(client, CURLOPT_ACCEPT_ENCODING, "deflate, gzip, br, zstd");
        // 设置读取响应的回调函数
        curl_easy_setopt(client, CURLOPT_WRITEFUNCTION, CurlUtil::responseCallback);
        // 设置请求头，libcurl 会管理这个指针，所以不要对它进行回收
        struct curl_slist *header = nullptr;
        header = curl_slist_append(header, "Content-Type: application/json");
        header = curl_slist_append(header, "kbn-version: 7.7.1");
        header = curl_slist_append(header, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:96.0) Gecko/20100101 Firefox/96.0");
        header = curl_slist_append(header, "Referer: http://kibana.irybd.com/app/kibana");
        header = curl_slist_append(header, "Origin: http://kibana.irybd.com");
        curl_easy_setopt(client, CURLOPT_HTTPHEADER, header);

       this->client = client;
       this->header = header;
    }

    /**
     * 发送 post 请求
     */
    shared_ptr<string> CurlUtil::post(const char *url, const char *body) {
        auto responseBoy = make_shared<string>();

        CURL *client = this->client;
        curl_easy_setopt(client, CURLOPT_URL, url);
        // 设置请求方法为 POST
        curl_easy_setopt(client, CURLOPT_POST, nullptr);
        // 设置请求要发送的数据
        if (body) curl_easy_setopt(client, CURLOPT_POSTFIELDS, body);
        // 设置用于读取响应数据的句柄
        curl_easy_setopt(client, CURLOPT_WRITEDATA, &responseBoy);

        // 发送请求并解析响应数据
        CURLcode ccode = curl_easy_perform(client);
        if (ccode) throw runtime_error(curl_easy_strerror(ccode));
        // 获取 HTTP 响应码，必须是 long
        long hcode;
        ccode = curl_easy_getinfo(client, CURLINFO_RESPONSE_CODE, &hcode);
        if (ccode) throw runtime_error(curl_easy_strerror(ccode));
        if (hcode != 200) {
            string message = "reponse code: " + to_string(hcode);
            message.append(", response body: ");
            message.append(*responseBoy);
            throw runtime_error(message);
        }
        // 转换为共享式智能指针，并返回
        return responseBoy;
    }

    /**
     * libcurl 读取响应的回调函数
     */
    size_t CurlUtil::responseCallback(char *buffer, size_t size, size_t nitems, shared_ptr<string> point) {
        size *= nitems;
        point->append(buffer, size);
        return size;
    }

    /**
     * 清理全局的 curl
     */
    void CurlUtil::close() {
        curl_global_cleanup();
        cout << "curl global cleanup" << endl;
    }

    /**
     * 获得当前时间的毫秒时间戳
     */
    size_t currentTimeMillis() {
        struct timeval currentTime;
        gettimeofday(&currentTime, nullptr);
        return currentTime.tv_sec * 1000 + currentTime.tv_usec / 1000;
    }

    /**
     * 格式化毫秒时间戳
     */
    string ftimestamp(size_t timeMill) {
        char *buff = new char[BUFSIZ];
        time_t t = (time_t)(timeMill / 1000);
        // 按照格林威治时间进行格式化
        size_t len = strftime(buff, BUFSIZ, "%Y-%m-%dT%H:%M:%S", gmtime(&t));
        string timestr(buff, len);

        // 追加毫秒
        memset(buff, 0, BUFSIZ);
        len = sprintf(buff, ".%03luZ", timeMill % 1000);
        timestr.append(buff, len);

        delete[] buff;
        return timestr;
    }
}
