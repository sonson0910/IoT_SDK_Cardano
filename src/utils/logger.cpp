/**
 * @file logger.cpp
 * @brief Implementation of logging utility for Cardano IoT SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include "cardano_iot/utils/logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <thread>

namespace cardano_iot::utils
{

    class Logger::Impl
    {
    public:
        LogLevel current_level_ = LogLevel::INFO;
        bool console_enabled_ = true;
        std::string file_path_;
        std::ofstream log_file_;
        std::mutex log_mutex_;

        std::string get_timestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }

        std::string get_level_string(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::TRACE:
                return "TRACE";
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO ";
            case LogLevel::WARNING:
                return "WARN ";
            case LogLevel::ERROR:
                return "ERROR";
            case LogLevel::FATAL:
                return "FATAL";
            default:
                return "UNKNOWN";
            }
        }

        std::string get_level_color(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::TRACE:
                return "\033[37m"; // White
            case LogLevel::DEBUG:
                return "\033[36m"; // Cyan
            case LogLevel::INFO:
                return "\033[32m"; // Green
            case LogLevel::WARNING:
                return "\033[33m"; // Yellow
            case LogLevel::ERROR:
                return "\033[31m"; // Red
            case LogLevel::FATAL:
                return "\033[35m"; // Magenta
            default:
                return "\033[0m"; // Reset
            }
        }

        std::string get_cyberpunk_prefix(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::TRACE:
                return "🔍";
            case LogLevel::DEBUG:
                return "🐛";
            case LogLevel::INFO:
                return "ℹ️ ";
            case LogLevel::WARNING:
                return "⚠️ ";
            case LogLevel::ERROR:
                return "❌";
            case LogLevel::FATAL:
                return "💀";
            default:
                return "🤖";
            }
        }
    };

    Logger &Logger::instance()
    {
        static Logger instance;
        static std::once_flag initialized;

        std::call_once(initialized, []()
                       { instance.pimpl_ = std::make_unique<Impl>(); });

        return instance;
    }

    void Logger::log(LogLevel level, const std::string &module, const std::string &message)
    {
        if (!pimpl_ || level < pimpl_->current_level_)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(pimpl_->log_mutex_);

        std::string timestamp = pimpl_->get_timestamp();
        std::string level_str = pimpl_->get_level_string(level);
        std::string thread_id = std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000);

        // Format log message
        std::stringstream log_stream;
        log_stream << "[" << timestamp << "] "
                   << "[" << level_str << "] "
                   << "[" << std::setw(8) << std::left << module << "] "
                   << "[T-" << std::setw(4) << std::setfill('0') << thread_id << "] "
                   << message;

        std::string log_line = log_stream.str();

        // Console output with cyberpunk styling
        if (pimpl_->console_enabled_)
        {
            std::string color = pimpl_->get_level_color(level);
            std::string prefix = pimpl_->get_cyberpunk_prefix(level);
            std::string reset = "\033[0m";

            std::cout << color << prefix << " " << log_line << reset << std::endl;
        }

        // File output
        if (pimpl_->log_file_.is_open())
        {
            pimpl_->log_file_ << log_line << std::endl;
            pimpl_->log_file_.flush();
        }
    }

    void Logger::set_level(LogLevel level)
    {
        if (pimpl_)
        {
            pimpl_->current_level_ = level;
        }
    }

    void Logger::enable_console(bool enable)
    {
        if (pimpl_)
        {
            pimpl_->console_enabled_ = enable;
        }
    }

    void Logger::set_file_path(const std::string &path)
    {
        if (!pimpl_)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(pimpl_->log_mutex_);

        if (pimpl_->log_file_.is_open())
        {
            pimpl_->log_file_.close();
        }

        pimpl_->file_path_ = path;

        if (!path.empty())
        {
            pimpl_->log_file_.open(path, std::ios::app);
            if (!pimpl_->log_file_.is_open())
            {
                std::cerr << "Failed to open log file: " << path << std::endl;
            }
        }
    }

} // namespace cardano_iot::utils
