//
// Kibana 管理器
// Created by aszswaz on 2022-01-24
//

#include "kibana.h"
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "config/config.h"
#include <unistd.h>

using namespace std;
using namespace nlohmann;

namespace favour {
    // Kibana 处理器队列
    vector<KEHandler *> *KibanaManager::handlers = nullptr;
    // curl util
    CurlUtil *KibanaManager::curlUtil = nullptr;
    // 上一次查询时间，单位：毫秒
    size_t KibanaManager::lastTime = 0;
    // 日志缓存队列
    vector<KibanaLog *> *KibanaManager::cacheLogs = new vector<KibanaLog *>;
    // 缓存文件
    string KibanaManager::cacheFile;

    /**
     * 初始化 Kibana 管理器
     */
    void KibanaManager::init() {
        cout << "Init kibana manager..." << endl;
        // 初始化程序和libcurl配置
        configInit();
        // curl 初始化
        CurlUtil::init();
        curlUtil = new CurlUtil;

        // 从缓存中读取上一次查询的日志结束时间。
        string cache = globalConfig->cacheDir;
        cache.append("/KibanaManager.txt");
        if (!access(cache.c_str(), R_OK)) {
            FILE *f = fopen(cache.c_str(), "r");
            fread(&KibanaManager::lastTime, sizeof(KibanaManager::lastTime), 1, f);
            fclose(f);
            cout << "read cache file " << cache << " successed." << endl;
        } else {
            KibanaManager::lastTime = time(nullptr) * 1000;
        }
        KibanaManager::cacheFile.append(cache);
        cout << "Set last time with " << KibanaManager::lastTime << endl;

        // 其他的初始化工作完成后，初始化管理器队列
        handlers = new vector<KEHandler *>;
        // 初始化并启动管理器的线程
        std::thread mt(polling);
        // mt 对象在函数结束后会被隐式 delete，需要通过 detach() 将线程与对象分离
        mt.detach();

        // 程序退出事件注册
        registerHook(KibanaManager::close);
        cout << "Init kibana manager successed." << endl;
    }

    /**
     * 清理管理器的资源
     */
    void KibanaManager::close() {
        cout << "Kibana Manager closing..." << endl;
        for (auto item : *handlers) delete item;
        delete handlers;
        for (auto item : *cacheLogs) delete item;
        delete cacheLogs;

        // 保存上次日志查询时间到文件
        FILE *f = fopen(KibanaManager::cacheFile.c_str(), "w");
        fwrite(&KibanaManager::lastTime, sizeof(KibanaManager::lastTime), 1, f);
        fclose(f);
        cout << "Last time " << KibanaManager::lastTime << ", updated " << KibanaManager::cacheFile << " successed." << endl;

        cout << "Kibana Manager closed." << endl;
    }

    /**
     * 注册处理器
     */
    void KibanaManager::registerHandler(KEHandler *handler) {
        for (auto item : *handlers) {
            // 防止同一个对象多次注册
            if (item == handler) return;
        }
        handlers->push_back(handler);
    }

    /**
     * 定时从 Kibana 服务器获取日志
     */
    void KibanaManager::polling(void) {
        while (1) {
            // 向服务器请求日志
            vector<KibanaLog *> *results = requestLog();
            for (auto h : *handlers) {
                for (auto item : *results) {
                    h->handler(item);
                }
            }
            delete results;
            this_thread::sleep_for(chrono::seconds(globalConfig->timer));
        }
    }

    /**
     * 从 kibana 服务器获取日志
     */
    vector<KibanaLog *> *KibanaManager::requestLog() {
        auto results = new vector<KibanaLog *>;
        size_t currentTime = currentTimeMillis();
        // 向 Kibana 请求日志数据
        for (auto config : *globalConfig->servers) {
            cout << "server name: " << config->name << ", api: " << config->api << endl;
            auto indices = config->indices;
            for (auto index : *indices) {
                // 生成日志的查询条件
                json reqBody = toQuery(index->index, lastTime, currentTime);
                shared_ptr<string> resBody;
                // 发送 HTTP POST 请求
                resBody = curlUtil->post(config->api.c_str(), reqBody.dump().c_str());
                // 解析日志数据
                json j = json::parse(*resBody);
                auto logs = parseLog(config, index, j);
                // 去除重复的 log
                for (auto item : *logs) {
                    for (auto cacheLog : *cacheLogs) {
                        if (item->index == cacheLog->index && item->id == cacheLog->id) {
                            delete item;
                            goto repeat;
                        }
                    }
                    results->push_back(item);
                    cacheLogs->push_back(item);

                    // 存在重复，goto 到这里
                repeat:
                    ;
                }
                delete logs;
            }
        }
        // 由于日志的记录存在延迟，所以每次查询都需要额外查询1分钟的日志数据。
        lastTime = currentTime - 60000;
        return results;
    }

    /**
     * 生成查询条件
     */
    json KibanaManager::toQuery(string &index, size_t startTime, size_t endTime) {
        json o = json::object();
        o["serverStrategy"] = "es";

        json params = json::object();
        params["ignoreThrottled"] = true;
        params["preference"] = currentTimeMillis();
        params["index"] = index;
        params["ignore_throttled"] = true;
        params["rest_total_hits_as_int"] = true;
        params["ignore_unavailable"] = true;
        params["timeout"] = "30000ms";

        json body = json::object();
        body["version"] = true;
        body["size"] = 500;

        json sort = json::array();
        json sort01 = json::object();

        json timestamp = json::object();
        timestamp["order"] = "desc";
        timestamp["unmapped_type"] = "boolean";

        sort01["@timestamp"] = timestamp;
        sort[0] = sort01;
        body["sort"] = sort;

        json aggs = json::object();
        json two = json::object();
        json dateHistogram = json::object();
        dateHistogram["field"] = "@timestamp";
        dateHistogram["fixed_interval"] = "30s";
        dateHistogram["time_zone"] = "Asia/Shanghai";
        dateHistogram["min_doc_count"] = 1;

        two["date_histogram"] = dateHistogram;
        aggs["2"] = two;
        body["aggs"] = aggs;

        json storedFields = json::array();
        storedFields[0] = "*";

        body["stored_fields"] = storedFields;
        body["script_fields"] = json::object();

        json docvalueFields = json::array();
        json df00 = json::object();
        df00["field"] = "@timestamp";
        df00["format"] = "date_time";
        json df01 = json::object();
        df01["field"] = "created";
        df01["format"] = "date_time";

        docvalueFields[0] = df00;
        docvalueFields[1] = df01;
        body["docvalue_fields"] = docvalueFields;

        json source = json::object();
        source["excludes"] = json::array();
        body["_source"] = source;

        json query = json::object();
        json bq = json::object();
        bq["must"] = json::array();
        json filter = json::array();
        json filter00 = json::object();
        filter00["match_all"] = json::object();
        json filter01 = json::object();
        json range = json::object();

        timestamp = json::object();
        timestamp["gte"] = ftimestamp(startTime);
        timestamp["lte"] = ftimestamp(endTime);
        timestamp["format"] = "strict_date_optional_time";

        filter[0] = filter00;
        range["@timestamp"] = timestamp;
        filter01["range"] = range;
        filter[1] = filter01;
        bq["filter"] = filter;
        query["bool"] = bq;
        body["query"] = query;

        json highlight = json::object();
        highlight["fragment_size"] = 2147483647;

        json preTags = json::array();
        preTags[0] = "@kibana-highlighted-field@";

        json postTags = json::array();
        postTags[0] = "@/kibana-highlighted-field@";

        json fields = json::object();
        fields["*"] = json::object();

        highlight["pre_tags"] = preTags;
        highlight["post_tags"] = postTags;
        highlight["fields"] = fields;
        body["highlight"] = highlight;

        params["body"] = body;
        o["params"] = params;
        return o;
    }

    /**
     * 日志等级的字符串转枚举
     */
    KIBANA_LOG_LEVEL KibanaManager::toLevelEnum(std::string &levelStr) {
        if ("DEBUG" == levelStr) {
            return KIBANA_LOG_DEBUG;
        } else if ("INFO" == levelStr) {
            return KIBANA_LOG_INFO;
        } else if ("WARN" == levelStr || "WARNING" == levelStr) {
            return KIBANA_LOG_WARN;
        } else {
            return KIBANA_LOG_ERROR;
        }
    }

    /**
     * 日志等级枚举转字符串
     */
    string KibanaManager::toLevelStr(KIBANA_LOG_LEVEL level) {
        switch (level) {
        case KIBANA_LOG_DEBUG:
            return "DEBUG";
        case KIBANA_LOG_INFO:
            return "INFO";
        case KIBANA_LOG_WARN:
            return "WARN";
        default:
            return "ERROR";
        }
    }

    /**
     * 解析日志数据
     */
    std::vector<KibanaLog *> *KibanaManager::parseLog(ServerConfig *config, KibanaIndex *kindex, json &data) {
        json rawResponse = data["rawResponse"];
        json hits = rawResponse["hits"]["hits"];
        size_t len = hits.size();
        cout << "Get " << len << " log." << endl;

        auto logs = new vector<KibanaLog *>;
        for (size_t i = 0; i < len; i++) {
            json hit = hits[i];
            string id = hit["_id"].get<string>();
            json source = hit["_source"];

            auto log = new KibanaLog;
            log->project = kindex->project;
            log->index = kindex->index;
            log->id = id;
            log->serverName = config->name;
            log->home = config->home;
            log->indexUrl = kindex->indexUrl;
            // 日志的格式错误，默认当作不是日志
            if (source.contains("LogStr")) {
                log->message = source["LogStr"].get<string>();
                log->time = source["LogTime"].get<string>();
                string level = source["LogLevel"].get<string>();
                log->level = KibanaManager::toLevelEnum(level);
            } else {
                // 日志为其他格式，则直接打印全文
                log->message = source.dump();
                log->time = source["@timestamp"].get<string>();
                log->level = KIBANA_LOG_WARN;
            }
            logs->push_back(log);
        }
        return logs;
    }
}
