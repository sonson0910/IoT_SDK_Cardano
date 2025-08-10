/**
 * @file logger.h
 * @brief Logging utility for Cardano IoT SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_LOGGER_H
#define CARDANO_IOT_LOGGER_H

#include <string>
#include <memory>

namespace cardano_iot::utils
{

    enum class LogLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        FATAL = 5
    };

    class Logger
    {
    public:
        static Logger &instance();

        void log(LogLevel level, const std::string &module, const std::string &message);
        void set_level(LogLevel level);
        void enable_console(bool enable);
        void set_file_path(const std::string &path);

    private:
        Logger() = default;
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace cardano_iot::utils

#endif // CARDANO_IOT_LOGGER_H
