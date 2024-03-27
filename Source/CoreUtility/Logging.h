#pragma once

#include <memory>
#include <string>

#pragma warning( push )
#pragma warning( disable : 4996 )
#pragma warning( disable : 6285 )
#include <spdlog/spdlog.h>
#pragma warning( pop )

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
    void Log(SeverityLevel level, std::string_view format_string, Args&&... args) {
        switch (level) {
            case SeverityLevel::TRACE: {
                logger_->trace(fmt::runtime(format_string), std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::INFO: {
                logger_->info(fmt::runtime(format_string), std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::WARN: {
                logger_->warn(fmt::runtime(format_string), std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::ERROR: {
                logger_->error(fmt::runtime(format_string), std::forward<Args>(args)...);
                break;
            }
            case SeverityLevel::FATAL: {
                logger_->critical(fmt::runtime(format_string), std::forward<Args>(args)...);
                break;
            }
        }
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
};

#define DEFINE_LOGGER(name, level) Logger logger_##name = Logger(#name, level) 
#define DEFINE_LOGGER_EXTERN(name) extern Logger logger_##name;
#define LOG(name, level, format_string, ...) logger_##name.Log(level, format_string, __VA_ARGS__)