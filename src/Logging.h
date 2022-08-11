// SPDX-FileCopyrightText: 2021 Nheko Contributors
// SPDX-FileCopyrightText: 2022 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace nhlog {
void
init(const std::string &file, bool enable_logger = true, bool enable_debug_log = false);

std::shared_ptr<spdlog::logger>
ui();

std::shared_ptr<spdlog::logger>
net();

std::shared_ptr<spdlog::logger>
db();

std::shared_ptr<spdlog::logger>
crypto();

std::shared_ptr<spdlog::logger>
dev();

}
