//
// Created by aszswaz on 2022-01-25
//

#include "KibanaManagerTest.h"
#include "kibana/kibana.h"
#include <iostream>
#include <fstream>
#include <utils/utils.h>
#include <pthread.h>

using namespace nlohmann;
using namespace std;

namespace favour {
    void KibanaManagerTest::initTest() {
        KibanaManager::init();
        while (1) {};
    }

    class DemoHandler : public KEHandler {
        public:
            void handler(KibanaLog *log) override {
                std::cout << log->message << std::endl;
            }
    };

    void KibanaManagerTest::registerHandlerTest() {
        KibanaManager::init();
        DemoHandler *handler = new DemoHandler;
        std::cout << __FILE__ << " " << __LINE__ << ": " << handler << std::endl;
        KibanaManager::registerHandler(handler);
        // 通过函数退出主线程，就可以让管理器继续运行
        pthread_exit(nullptr);
    }

    void KibanaManagerTest::toQuery() {
        std::string index = "logstash-spring-demo";
        size_t startTime = 1643334672001, endTime = 1643335572000;

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

        std::cout << o.dump(2) << std::endl;
    }

    /**
     * 解析日志数据
     */
    void KibanaManagerTest::parseData() {
        ifstream input("curl/local-kibana.json");
        json resBody;
        input >> resBody;
        input.close();

        json rawResponse = resBody["rawResponse"];
        json hits = rawResponse["hits"]["hits"];
        size_t len = hits.size();
        cout << "Get " << len << endl;

        auto logs = new vector<KibanaLog *>;
        for (size_t i = 0; i < len; i++) {
            json hit = hits[i];
            string id = hit["_id"].get<string>();
            json source = hit["_source"];

            auto log = new KibanaLog;
            log->id = id;
            log->message = source["LogStr"].get<string>();
            log->time = source["LogTime"].get<string>();
            string level = source["LogLevel"].get<string>();
            log->level = KibanaManager::toLevelEnum(level);
            logs->push_back(log);
        }
        for (auto item : *logs) {
            cout << item->time << ": " << item->id << ": " << item->level << ": " << item->message << endl;
            delete item;
        }
        delete logs;
    }
}
