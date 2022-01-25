// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Logging.h"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/android_sink.h"
#include <iostream>

#include <QString>
#include <QtGlobal>

namespace {

std::shared_ptr<nhlog::os_specific_logger> db_logger     = nullptr;
std::shared_ptr<nhlog::os_specific_logger> net_logger    = nullptr;
std::shared_ptr<nhlog::os_specific_logger> crypto_logger = nullptr;
std::shared_ptr<nhlog::os_specific_logger> ui_logger     = nullptr;
std::shared_ptr<nhlog::os_specific_logger> dev_logger    = nullptr;

constexpr auto MAX_FILE_SIZE = 1024 * 1024 * 6;
constexpr auto MAX_LOG_FILES = 3;

void
devMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::string localMsg = msg.toStdString();
    const char *file     = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    if (
      // The default style has the point size set. If you use pixel size anywhere, you get
      // that warning, which is useless, since sometimes you need the pixel size to match the
      // text to the size of the outer element for example. This is done in the avatar and
      // without that you get one warning for every Avatar displayed, which is stupid!
      msg.endsWith(QStringLiteral("Both point size and pixel size set. Using pixel size.")))
        return;

    switch (type) {
    case QtDebugMsg:
        nhlog::dev()->debug("{} ({}:{}, {})", localMsg, file, context.line, function);
        break;
    case QtInfoMsg:
        nhlog::dev()->info("{} ({}:{}, {})", localMsg, file, context.line, function);
        break;
    case QtWarningMsg:
        nhlog::dev()->warn("{} ({}:{}, {})", localMsg, file, context.line, function);
        break;
    case QtCriticalMsg:
        nhlog::dev()->critical("{} ({}:{}, {})", localMsg, file, context.line, function);
        break;
    case QtFatalMsg:
        nhlog::dev()->critical("{} ({}:{}, {})", localMsg, file, context.line, function);
        break;
    }
}
}

namespace nhlog {

void
init(const std::string &file_path, bool enable_logger, bool enable_debug_log)
{
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      file_path, MAX_FILE_SIZE, MAX_LOG_FILES);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(file_sink);
    sinks.push_back(console_sink);

    net_logger    = std::make_shared<nhlog::os_specific_logger>("net", std::begin(sinks), std::end(sinks));
    ui_logger     = std::make_shared<nhlog::os_specific_logger>("ui", std::begin(sinks), std::end(sinks));
    db_logger     = std::make_shared<nhlog::os_specific_logger>("db", std::begin(sinks), std::end(sinks));
    crypto_logger = std::make_shared<nhlog::os_specific_logger>("crypto", std::begin(sinks), std::end(sinks));
    dev_logger    = std::make_shared<nhlog::os_specific_logger>("dev", std::begin(sinks), std::end(sinks));
    
    spdlog::level::level_enum logLevel = spdlog::level::off;
    if(enable_logger){
        logLevel = spdlog::level::info;
    } 
    if (enable_debug_log) {
        logLevel = spdlog::level::trace;
    } 

    db_logger->set_level(logLevel);
    ui_logger->set_level(logLevel);
    crypto_logger->set_level(logLevel);
    net_logger->set_level(logLevel);
    dev_logger->set_level(logLevel);

    qInstallMessageHandler(devMessageHandler);
}

std::shared_ptr<nhlog::os_specific_logger>
ui()
{
    return ui_logger;
}

std::shared_ptr<nhlog::os_specific_logger>
net()
{
    return net_logger;
}

std::shared_ptr<nhlog::os_specific_logger>
db()
{
    return db_logger;
}

std::shared_ptr<nhlog::os_specific_logger>
crypto()
{
    return crypto_logger;
}

std::shared_ptr<nhlog::os_specific_logger>
dev()
{
    return dev_logger;
}
}
