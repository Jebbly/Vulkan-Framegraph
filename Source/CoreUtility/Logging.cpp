#include "Logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

Logger::Logger(const std::string& logger_name, SeverityLevel level) {
    logger_ = spdlog::stdout_color_mt(logger_name);

    switch (level) {
        case SeverityLevel::TRACE: {
            logger_->set_level(spdlog::level::trace);
            break;
        }
        case SeverityLevel::INFO: {
            logger_->set_level(spdlog::level::info);
            break;
        }
        case SeverityLevel::WARN: {
            logger_->set_level(spdlog::level::warn);
            break;
        }
        case SeverityLevel::ERROR: {
            logger_->set_level(spdlog::level::err);
            break;
        }
        case SeverityLevel::FATAL: {
            logger_->set_level(spdlog::level::critical);
            break;
        }
    }

    logger_->set_pattern("%^[%T] %n: %v%$");
}