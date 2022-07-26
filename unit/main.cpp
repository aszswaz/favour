//
// Created by aszswaz on 2022/1/18.
//

#include "gui/PollingTrayIcon.h"
#include <iostream>
#include <ctime>
#include <fstream>
#include "config/config.h"
#include "config/hook.h"
#include "kibana/KibanaManagerTest.h"
#include "utils/UtilsTest.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
using namespace nlohmann;
using namespace std;
using namespace favour;

/**
 * C++ 时间格式化测试
 */
void timeFormatTest() {
    // 获得当前时间的秒的时间戳
    time_t currentTime = time(nullptr);
    cout << currentTime << endl;
    // 格式化时间戳
    char buff[BUFSIZ];
    // 按照 UTC 时间格式化
    strftime(reinterpret_cast<char *>(&buff), BUFSIZ, "%Y-%m-%d %H:%M:%S.000", gmtime(&currentTime));
    cout << buff << endl;
    memset(buff, 0, BUFSIZ);
    // 按照本地时间格式化
    strftime(reinterpret_cast<char *>(&buff), BUFSIZ, "%Y-%m-%d %H:%M:%S.000", localtime(&currentTime));
    cout << buff << endl;
}

/**
 * 显示弹窗信息
 */
void showMessage(int argc, char **argv) {
    QApplication application(argc, argv);

    PollingTrayIcon trayIcon;
    trayIcon.setIcon(QIcon("sources/TryIcon.jpg"));
    trayIcon.setVisible(true);
    trayIcon.setToolTip(QApplication::tr("暂无信息"));
    trayIcon.showMessage(QApplication::tr("demo"), QApplication::tr("demo"));
    QApplication::exec();
}

/**
 * 配置的读取测试
 */
void configInitTest() {
    favour::configInit();
    const favour::GlobalConfig *gc = favour::globalConfig;
    auto servers = gc->servers;
    for (auto item : *servers) {
        cout << item->indices->size() << endl;
        for (auto index : *item->indices) {
            cout << "url: " << item->api << ", index: " << index << endl;
        }
    }
    delete gc;
}

    // favour::KibanaManagerTest::toQuery();
int main(int argc, char **argv) {
    // requestBodyTest();
    // timeFormatTest();
    // showMessage(argc, argv);
    configInitTest();
    // favour::KibanaManagerTest::initTest();
    // favour::KibanaManagerTest::registerHandlerTest();
    // favour::curlUtilTest();
    // favour::KibanaManagerTest::parseData();
    return 0;
}
