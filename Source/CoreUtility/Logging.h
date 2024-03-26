#pragma once

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

class Logger {
public:
    enum SeverityLevel {
        TRACE,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    Logger(const std::string& logger_name, SeverityLevel level);
    ~Logger() = default;

    template<typename... Args>
    void Log(SeverityLevel level, Args&&... args) {
        switch (level) {
            case SeverityLevel::TRACE: {
                logger_->trace(std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::INFO: {
                logger_->info(std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::WARN: {
                logger_->warn(std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::ERROR: {
                logger_->error(std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::FATAL: {
                logger_->critical(std::forward<Args>(args)...);
                break;
            }
        }
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
};

#define DEFINE_LOGGER(name, level) Logger logger_##name = Logger(#name, level) 
#define DEFINE_LOGGER_EXTERN(name, level) extern Logger logger_##name;
#define LOG(name, level, ...) logger_##name.Log(level, __VA_ARGS__)