//
// 程序的钩子管理
// Create by aszswaz on 2022-01-23
//

#include "hook.h"
#include <vector>
#include <csignal>
#include <cstdlib>
#include <iostream>

using namespace std;

namespace favour {
    const auto HOOKS = new vector<void (*)(void)>;

    int exitCode = 0;

    /**
     * 程序退出钩子
     */
    void exitHook() {
        cout << "stoped ..." << endl;

        if (!HOOKS || !HOOKS->size()) {
            return;
        }

        for (auto item : *HOOKS) {
            try {
                item();
            } catch (std::exception &e) {
                cerr << e.what() << endl;
            }
        }

        delete HOOKS;
        // 指针改为空指针
        *((size_t *) &HOOKS) = (size_t) nullptr;
        exit(exitCode);
    }

    void exitHookSignal(int code) {
        exitCode = code;
        exit(code);
    }

    void hookInit() {
        // 程序正常退出
        atexit(exitHook);
        // 程序异常退出
        signal(SIGTERM, exitHookSignal);
        signal(SIGINT, exitHookSignal);
        signal(SIGBUS, exitHookSignal);
        signal(SIGSEGV, exitHookSignal);
        signal(SIGFPE, exitHookSignal);
        signal(SIGABRT, exitHookSignal);
    }

    void registerHook(void (*hookFun)(void)) {
        HOOKS->push_back(hookFun);
    }
}
