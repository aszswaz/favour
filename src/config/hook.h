//
// 程序钩子管理
// Create by aszswaz on 2022-01-23
//

#ifndef HOOK_H
#define HOOK_H

namespace favour {
    /**
     * 初始化钩子
     */
    void hookInit();

    /**
     * 注册钩子
    */
    void registerHook(void (*hookFun)(void));
}

#endif
