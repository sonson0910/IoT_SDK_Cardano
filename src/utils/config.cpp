/**
 * @file config.cpp
 * @brief Implementation of configuration management for Cardano IoT SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include "cardano_iot/utils/config.h"
#include "cardano_iot/utils/logger.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace cardano_iot::utils
{

    class Config::Impl
    {
    public:
        json config_data_;

        json get_nested_value(const std::string &key) const
        {
            std::stringstream ss(key);
            std::string item;
            json current = config_data_;

            while (std::getline(ss, item, '.'))
            {
                if (current.is_object() && current.contains(item))
                {
                    current = current[item];
                }
                else
                {
                    return json(); // Return null json if key not found
                }
            }

            return current;
        }

        void set_nested_value(const std::string &key, const json &value)
        {
            std::stringstream ss(key);
            std::string item;
            json *current = &config_data_;
            std::vector<std::string> path;

            // Build path
            while (std::getline(ss, item, '.'))
            {
                path.push_back(item);
            }

            // Navigate to the parent of the target key
            for (size_t i = 0; i < path.size() - 1; ++i)
            {
                if (!current->is_object())
                {
                    *current = json::object();
                }
                current = &((*current)[path[i]]);
            }

            // Set the final value
            if (!current->is_object())
            {
                *current = json::object();
            }
            (*current)[path.back()] = value;
        }
    };

    Config::Config() : pimpl_(std::make_unique<Impl>()) {}

    Config::~Config() = default;

    bool Config::load_from_file(const std::string &file_path)
    {
        try
        {
            std::ifstream file(file_path);
            if (!file.is_open())
            {
                Logger::instance().log(LogLevel::ERROR, "Config",
                                       "Failed to open configuration file: " + file_path);
                return false;
            }

            file >> pimpl_->config_data_;

            Logger::instance().log(LogLevel::INFO, "Config",
                                   "Configuration loaded from: " + file_path);
            return true;
        }
        catch (const std::exception &e)
        {
            Logger::instance().log(LogLevel::ERROR, "Config",
                                   "Failed to parse configuration file: " + std::string(e.what()));
            return false;
        }
    }

    bool Config::load_from_string(const std::string &json_str)
    {
        try
        {
            pimpl_->config_data_ = json::parse(json_str);

            Logger::instance().log(LogLevel::DEBUG, "Config",
                                   "Configuration loaded from string");
            return true;
        }
        catch (const std::exception &e)
        {
            Logger::instance().log(LogLevel::ERROR, "Config",
                                   "Failed to parse configuration string: " + std::string(e.what()));
            return false;
        }
    }

    bool Config::save_to_file(const std::string &file_path) const
    {
        try
        {
            std::ofstream file(file_path);
            if (!file.is_open())
            {
                Logger::instance().log(LogLevel::ERROR, "Config",
                                       "Failed to open file for writing: " + file_path);
                return false;
            }

            file << pimpl_->config_data_.dump(4); // Pretty print with 4 spaces

            Logger::instance().log(LogLevel::INFO, "Config",
                                   "Configuration saved to: " + file_path);
            return true;
        }
        catch (const std::exception &e)
        {
            Logger::instance().log(LogLevel::ERROR, "Config",
                                   "Failed to save configuration: " + std::string(e.what()));
            return false;
        }
    }

    std::string Config::get_string(const std::string &key, const std::string &default_value) const
    {
        json value = pimpl_->get_nested_value(key);
        if (value.is_string())
        {
            return value.get<std::string>();
        }
        return default_value;
    }

    int Config::get_int(const std::string &key, int default_value) const
    {
        json value = pimpl_->get_nested_value(key);
        if (value.is_number_integer())
        {
            return value.get<int>();
        }
        return default_value;
    }

    bool Config::get_bool(const std::string &key, bool default_value) const
    {
        json value = pimpl_->get_nested_value(key);
        if (value.is_boolean())
        {
            return value.get<bool>();
        }
        return default_value;
    }

    double Config::get_double(const std::string &key, double default_value) const
    {
        json value = pimpl_->get_nested_value(key);
        if (value.is_number())
        {
            return value.get<double>();
        }
        return default_value;
    }

    void Config::set_string(const std::string &key, const std::string &value)
    {
        pimpl_->set_nested_value(key, value);
    }

    void Config::set_int(const std::string &key, int value)
    {
        pimpl_->set_nested_value(key, value);
    }

    void Config::set_bool(const std::string &key, bool value)
    {
        pimpl_->set_nested_value(key, value);
    }

    void Config::set_double(const std::string &key, double value)
    {
        pimpl_->set_nested_value(key, value);
    }

    bool Config::has_key(const std::string &key) const
    {
        json value = pimpl_->get_nested_value(key);
        return !value.is_null();
    }

    std::map<std::string, std::string> Config::get_all() const
    {
        std::map<std::string, std::string> result;

        // Flatten JSON to key-value pairs
        std::function<void(const json &, const std::string &)> flatten;
        flatten = [&](const json &obj, const std::string &prefix)
        {
            for (auto it = obj.begin(); it != obj.end(); ++it)
            {
                std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();

                if (it.value().is_object())
                {
                    flatten(it.value(), key);
                }
                else
                {
                    result[key] = it.value().dump();
                }
            }
        };

        if (pimpl_->config_data_.is_object())
        {
            flatten(pimpl_->config_data_, "");
        }

        return result;
    }

    void Config::clear()
    {
        pimpl_->config_data_ = json::object();
        Logger::instance().log(LogLevel::DEBUG, "Config", "Configuration cleared");
    }

} // namespace cardano_iot::utils
