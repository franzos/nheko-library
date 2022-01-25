// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace nhlog {

#ifdef Q_OS_ANDROID
typedef spdlog::android_logger_mt os_specific_logger;
#else
typedef spdlog::logger os_specific_logger;
#endif

void
init(const std::string &file, bool enable_logger = true, bool enable_debug_log = false);

std::shared_ptr<os_specific_logger>
ui();

std::shared_ptr<os_specific_logger>
net();

std::shared_ptr<os_specific_logger>
db();

std::shared_ptr<os_specific_logger>
crypto();

std::shared_ptr<os_specific_logger>
dev();

}
