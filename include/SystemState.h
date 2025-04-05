// 删除main.cpp中的结构体定义，保留SystemState.h中的唯一定义
// SystemState.h
#pragma once
struct SystemState {
    uint8_t active_channel = 0;
    uint8_t mode_a = 0;
    bool screen_mode = false;
    bool active_changed = false;  // 添加状态变更标志
    bool mode_changed = false;
};