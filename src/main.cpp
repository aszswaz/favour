#include <QApplication>
#include "gui/PollingTrayIcon.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include "config/config.h"

using namespace std;
using namespace favour;

int main(int argc, char **argv) {
    std::cout << "Program initialization succeeded." << std::endl;

    QApplication application(argc, argv);

    // 初始化 KibanaManager
    favour::KibanaManager::init();

    // 打开系统托盘图标，自动从服务器获得日志
    auto trayIcon = new favour::PollingTrayIcon;
    string iconPath = globalConfig->configDir;
    iconPath.append("/TrayIcon.jpg");
    std::cout << "set icon path: " << iconPath << std::endl;
    trayIcon->setIcon(QIcon(iconPath.c_str()));
    trayIcon->setVisible(true);
    trayIcon->setToolTip(QApplication::tr("暂无信息"));
    favour::KibanaManager::registerHandler(trayIcon);
    return QApplication::exec();
}
