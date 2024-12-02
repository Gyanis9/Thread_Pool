//
// Created by guo on 24-12-2.
//
#include "../lib/Task.h"

void Task::exec() {
    if (m_res != nullptr) {
        m_res->setAny(run());
    }
}
