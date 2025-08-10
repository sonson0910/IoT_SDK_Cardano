/**
 * @file config.h
 * @brief Configuration management for Cardano IoT SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_CONFIG_H
#define CARDANO_IOT_CONFIG_H

#include <string>
#include <map>
#include <memory>

namespace cardano_iot::utils
{

    /**
     * @brief Configuration manager for the SDK
     */
    class Config
    {
    public:
        /**
         * @brief Constructor
         */
        Config();

        /**
         * @brief Destructor
         */
        ~Config();

        /**
         * @brief Load configuration from file
         * @param file_path Path to configuration file
         * @return true if successful
         */
        bool load_from_file(const std::string &file_path);

        /**
         * @brief Load configuration from JSON string
         * @param json_str JSON configuration string
         * @return true if successful
         */
        bool load_from_string(const std::string &json_str);

        /**
         * @brief Save configuration to file
         * @param file_path Path to save configuration
         * @return true if successful
         */
        bool save_to_file(const std::string &file_path) const;

        /**
         * @brief Get string value
         * @param key Configuration key
         * @param default_value Default value if key not found
         * @return Configuration value
         */
        std::string get_string(const std::string &key, const std::string &default_value = "") const;

        /**
         * @brief Get integer value
         * @param key Configuration key
         * @param default_value Default value if key not found
         * @return Configuration value
         */
        int get_int(const std::string &key, int default_value = 0) const;

        /**
         * @brief Get boolean value
         * @param key Configuration key
         * @param default_value Default value if key not found
         * @return Configuration value
         */
        bool get_bool(const std::string &key, bool default_value = false) const;

        /**
         * @brief Get double value
         * @param key Configuration key
         * @param default_value Default value if key not found
         * @return Configuration value
         */
        double get_double(const std::string &key, double default_value = 0.0) const;

        /**
         * @brief Set string value
         * @param key Configuration key
         * @param value Value to set
         */
        void set_string(const std::string &key, const std::string &value);

        /**
         * @brief Set integer value
         * @param key Configuration key
         * @param value Value to set
         */
        void set_int(const std::string &key, int value);

        /**
         * @brief Set boolean value
         * @param key Configuration key
         * @param value Value to set
         */
        void set_bool(const std::string &key, bool value);

        /**
         * @brief Set double value
         * @param key Configuration key
         * @param value Value to set
         */
        void set_double(const std::string &key, double value);

        /**
         * @brief Check if key exists
         * @param key Configuration key
         * @return true if key exists
         */
        bool has_key(const std::string &key) const;

        /**
         * @brief Get all configuration as map
         * @return Configuration map
         */
        std::map<std::string, std::string> get_all() const;

        /**
         * @brief Clear all configuration
         */
        void clear();

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace cardano_iot::utils

#endif // CARDANO_IOT_CONFIG_H
