//
// Created by aszswaz on 2022/1/21.
//

#include "config.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#define GET_CWD(path) \
    char buff[BUFSIZ]{}; \
    if (!getcwd(buff, BUFSIZ)) \
        throw runtime_error(strerror(errno)); \
    else \
        path.append(buff);

using namespace nlohmann;
using namespace std;

namespace favour {
    const GlobalConfig *globalConfig = nullptr;

    /**
     * 根据环境，选择不同的配置文件夹
     */
    static string configDir() {
        #if PROFILE == DEV
        string configDir;
        GET_CWD(configDir);
        configDir.append("/sources");
        return configDir;
        #elif PROFILE == PRO
        string configPath;
        configPath.append(getenv("HOME"));
        configPath.append("/.config/favour");
        return configPath;
        #else
        throw runtime_error("unknown profile: " PROFILE);
        #endif
    }

    /**
     * 程序的缓存文件夹
     */
    static string cacheDir() {
        string cacheDir;

        #if PROFILE == DEV
        GET_CWD(cacheDir);
        cacheDir.append("/cache");
        #elif PROFILE == PRO
        cacheDir.append(getenv("HOME"));
        cacheDir.append("/.cache/favour");
        #else
        throw runtime_error("Unknown profile " PROFILE);
        #endif

        // 如果文件夹不存在，创建文件夹
        if (access(cacheDir.c_str(), F_OK)) {
            // 创建文件夹，该文件所有者拥有读，写和执行操作的权限
            if (mkdir(cacheDir.c_str(), S_IRWXU)) {
                // 文件创建失败
                throw runtime_error(strerror(errno));
            } else {
                cout << "create folder " << cacheDir << " successed." << endl;
            }
        } else {
            cout << "Folder " << cacheDir << " already exists." << endl;
        }

        return cacheDir;
    }

    /**
     * 读取 kibana 服务器配置
     */
    static vector<ServerConfig *> *parseServers(json &configJson) {
        auto servers = new vector<ServerConfig *>;

        // 解析服务器配置
        json jservers = configJson["servers"];
        for (int i = 0; i < jservers.size(); i++) {
            json jsc = jservers[i];
            auto sc = new ServerConfig;
            sc->name = jsc["name"].get<std::string>();
            sc->home = jsc["home"].get<std::string>();
            sc->api = jsc["api"].get<std::string>();

            // 解析 index
            auto indices = new std::vector<KibanaIndex *>;
            json jindices = jsc["indices"];
            for (int j = 0; j < jindices.size(); j++) {
                // 通过复制字符串的方式，把元素添加到集合
                auto kindex = new KibanaIndex;
                json element = jindices[j];
                kindex->project = element["project"].get<string>();
                kindex->index = element["index"].get<string>();
                kindex->indexUrl = element["url"].get<string>();
                indices->push_back(kindex);
            }
            sc->indices = indices;
            servers->push_back(sc);
        }
        return servers;
    }

    /**
     * 读取配置文件，解析服务器配置
     */
    static void parseConfig() {
        GlobalConfig *gc = new GlobalConfig;
        gc->configDir = configDir();
        std::cout << "set config dir " << gc->configDir << std::endl;

        string configJsonPath = gc->configDir;
        configJsonPath.append("/config.json");
        if (access(configJsonPath.c_str(), R_OK)) {
            throw runtime_error("File " + configJsonPath + " cannot be read.");
        }
        std::cout << "reading config file: " << configJsonPath << std::endl;
        json configJson;
        std::ifstream input(configJsonPath);
        input >> configJson;
        input.close();

        // 解析 kibana 服务器配置
        gc->servers = parseServers(configJson);
        // 定时时间间隔
        if (configJson.contains("timer"))
            gc->timer = configJson["timer"].get<unsigned int>();
        else
            gc->timer = 60;
        std::cout << "set timer " << gc->timer << std::endl;
        // 缓存文件夹
        if (configJson.contains("cacheDir"))
            gc->cacheDir = configJson["cacheDir"].get<string>();
        else
            gc->cacheDir = cacheDir();
        std::cout << "set cache dir " << gc->cacheDir << std::endl;


        // 更改全局配置指针
        *((size_t *) &globalConfig) = (size_t) gc;
        cout << "The configuration file " << configJsonPath << " is parsed successfully." << endl;
    }

    /**
     * 回收内存
     */
    void freeConfig() {
        delete globalConfig;
    }

    /**
     * 初始化程序
     */
    void configInit() {
        // 读取配置文件，加载到全局内存
        parseConfig();

        // 初始化程序退出事件的钩子
        hookInit();
        registerHook(&freeConfig);
    }
}
