/**
 * @file realtime_dashboard.h
 * @brief Real-time Monitoring Dashboard for Cardano IoT SDK
 *
 * This module provides a comprehensive real-time dashboard with web interface,
 * metrics visualization, alerts, and monitoring capabilities.
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_REALTIME_DASHBOARD_H
#define CARDANO_IOT_REALTIME_DASHBOARD_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>

namespace cardano_iot::monitoring
{

    /**
     * @brief Widget types for dashboard
     */
    enum class WidgetType
    {
        LINE_CHART,       // Time series line chart
        BAR_CHART,        // Bar chart
        PIE_CHART,        // Pie chart
        GAUGE,            // Gauge meter
        METRIC_CARD,      // Single metric display
        STATUS_INDICATOR, // Status light/indicator
        DATA_TABLE,       // Tabular data
        MAP_VIEW,         // Geographic map
        HEATMAP,          // Heat map
        ALERT_LIST,       // List of alerts
        LOG_VIEWER,       // Log viewer
        CUSTOM            // Custom widget
    };

    /**
     * @brief Alert severity levels
     */
    enum class AlertSeverity
    {
        INFO,    // Informational
        LOW,     // Low priority
        MEDIUM,  // Medium priority
        HIGH,    // High priority
        CRITICAL // Critical alert
    };

    /**
     * @brief Dashboard theme
     */
    enum class DashboardTheme
    {
        LIGHT,     // Light theme
        DARK,      // Dark theme
        CYBERPUNK, // Cyberpunk theme
        CUSTOM     // Custom theme
    };

    /**
     * @brief Real-time data point
     */
    struct DataPoint
    {
        uint64_t timestamp;
        double value;
        std::map<std::string, std::string> tags;
        std::string unit;
        std::string description;
    };

    /**
     * @brief Time series data
     */
    struct TimeSeries
    {
        std::string metric_name;
        std::string device_id;
        std::vector<DataPoint> data_points;
        std::string color;
        std::string line_style;
        bool visible;
    };

    /**
     * @brief Dashboard widget configuration
     */
    struct WidgetConfig
    {
        std::string widget_id;
        std::string title;
        WidgetType type;
        uint32_t width;      // Grid width (1-12)
        uint32_t height;     // Grid height (1-12)
        uint32_t x_position; // X position in grid
        uint32_t y_position; // Y position in grid

        // Data configuration
        std::vector<std::string> data_sources;
        std::string aggregation_func;
        uint32_t time_range_minutes;
        uint32_t refresh_interval_ms;

        // Visualization configuration
        std::map<std::string, std::string> chart_options;
        std::vector<std::string> colors;
        double min_value;
        double max_value;
        std::string unit;

        // Interaction configuration
        bool is_interactive;
        bool show_legend;
        bool show_tooltip;
        bool enable_zoom;
        bool enable_export;

        // Alert configuration
        std::vector<std::pair<double, AlertSeverity>> alert_thresholds;
        std::string alert_message_template;
    };

    /**
     * @brief Dashboard alert
     */
    struct DashboardAlert
    {
        std::string alert_id;
        std::string title;
        std::string message;
        AlertSeverity severity;
        std::string device_id;
        std::string metric_name;
        double current_value;
        double threshold_value;
        uint64_t triggered_time;
        uint64_t acknowledged_time;
        bool is_acknowledged;
        std::string acknowledged_by;
        std::map<std::string, std::string> metadata;
    };

    /**
     * @brief Dashboard layout configuration
     */
    struct DashboardLayout
    {
        std::string layout_id;
        std::string name;
        std::string description;
        DashboardTheme theme;
        uint32_t grid_columns;
        uint32_t grid_rows;
        std::vector<WidgetConfig> widgets;
        std::map<std::string, std::string> custom_css;
        std::map<std::string, std::string> custom_js;
    };

    /**
     * @brief User session information
     */
    struct UserSession
    {
        std::string session_id;
        std::string user_id;
        std::string username;
        std::vector<std::string> permissions;
        uint64_t login_time;
        uint64_t last_activity;
        std::string ip_address;
        std::string user_agent;
        bool is_authenticated;
    };

    /**
     * @brief Real-time Dashboard
     *
     * Provides a comprehensive web-based real-time monitoring dashboard with:
     * - Real-time data visualization
     * - Customizable widgets and layouts
     * - Alert management and notifications
     * - Multi-user support with authentication
     * - REST API for external integrations
     */
    class RealtimeDashboard
    {
    public:
        using DataUpdateCallback = std::function<void(const std::string &, const DataPoint &)>;
        using AlertCallback = std::function<void(const DashboardAlert &)>;
        using UserEventCallback = std::function<void(const std::string &, const std::string &)>;

        /**
         * @brief Constructor
         */
        RealtimeDashboard();

        /**
         * @brief Destructor
         */
        ~RealtimeDashboard();

        /**
         * @brief Initialize dashboard server
         * @param config Configuration parameters
         * @return true if initialization successful
         */
        bool initialize(const std::map<std::string, std::string> &config = {});

        /**
         * @brief Start dashboard server
         * @param port HTTP server port
         * @param interface Network interface to bind (empty for all)
         * @return true if server started successfully
         */
        bool start_server(uint16_t port = 8080, const std::string &interface = "");

        /**
         * @brief Stop dashboard server
         */
        void stop_server();

        /**
         * @brief Shutdown dashboard
         */
        void shutdown();

        // Data Management
        /**
         * @brief Add data source
         * @param source_id Data source identifier
         * @param source_name Human-readable name
         * @param source_type Type of data source
         * @return true if source added successfully
         */
        bool add_data_source(const std::string &source_id,
                             const std::string &source_name,
                             const std::string &source_type);

        /**
         * @brief Remove data source
         * @param source_id Data source identifier
         * @return true if source removed successfully
         */
        bool remove_data_source(const std::string &source_id);

        /**
         * @brief Push real-time data
         * @param source_id Data source identifier
         * @param data_point Data point to add
         * @return true if data pushed successfully
         */
        bool push_data(const std::string &source_id, const DataPoint &data_point);

        /**
         * @brief Push batch of data points
         * @param source_id Data source identifier
         * @param data_points Vector of data points
         * @return Number of successfully pushed data points
         */
        uint32_t push_data_batch(const std::string &source_id,
                                 const std::vector<DataPoint> &data_points);

        /**
         * @brief Get time series data
         * @param source_id Data source identifier
         * @param start_time Start timestamp
         * @param end_time End timestamp
         * @param max_points Maximum number of points to return
         * @return Time series data
         */
        TimeSeries get_time_series(const std::string &source_id,
                                   uint64_t start_time = 0,
                                   uint64_t end_time = 0,
                                   uint32_t max_points = 1000) const;

        // Widget Management
        /**
         * @brief Create widget
         * @param layout_id Dashboard layout identifier
         * @param widget_config Widget configuration
         * @return true if widget created successfully
         */
        bool create_widget(const std::string &layout_id, const WidgetConfig &widget_config);

        /**
         * @brief Update widget
         * @param layout_id Dashboard layout identifier
         * @param widget_id Widget identifier
         * @param widget_config Updated widget configuration
         * @return true if widget updated successfully
         */
        bool update_widget(const std::string &layout_id,
                           const std::string &widget_id,
                           const WidgetConfig &widget_config);

        /**
         * @brief Delete widget
         * @param layout_id Dashboard layout identifier
         * @param widget_id Widget identifier
         * @return true if widget deleted successfully
         */
        bool delete_widget(const std::string &layout_id, const std::string &widget_id);

        /**
         * @brief Get widget configuration
         * @param layout_id Dashboard layout identifier
         * @param widget_id Widget identifier
         * @return Widget configuration
         */
        WidgetConfig get_widget_config(const std::string &layout_id,
                                       const std::string &widget_id) const;

        // Layout Management
        /**
         * @brief Create dashboard layout
         * @param layout Dashboard layout configuration
         * @return true if layout created successfully
         */
        bool create_layout(const DashboardLayout &layout);

        /**
         * @brief Update dashboard layout
         * @param layout_id Layout identifier
         * @param layout Updated layout configuration
         * @return true if layout updated successfully
         */
        bool update_layout(const std::string &layout_id, const DashboardLayout &layout);

        /**
         * @brief Delete dashboard layout
         * @param layout_id Layout identifier
         * @return true if layout deleted successfully
         */
        bool delete_layout(const std::string &layout_id);

        /**
         * @brief Get dashboard layout
         * @param layout_id Layout identifier
         * @return Dashboard layout configuration
         */
        DashboardLayout get_layout(const std::string &layout_id) const;

        /**
         * @brief List all layouts
         * @return Vector of layout identifiers
         */
        std::vector<std::string> list_layouts() const;

        /**
         * @brief Clone layout
         * @param source_layout_id Source layout identifier
         * @param new_layout_id New layout identifier
         * @param new_name New layout name
         * @return true if layout cloned successfully
         */
        bool clone_layout(const std::string &source_layout_id,
                          const std::string &new_layout_id,
                          const std::string &new_name);

        // Alert Management
        /**
         * @brief Create alert rule
         * @param rule_id Alert rule identifier
         * @param source_id Data source identifier
         * @param condition Alert condition expression
         * @param severity Alert severity
         * @param message_template Alert message template
         * @return true if alert rule created successfully
         */
        bool create_alert_rule(const std::string &rule_id,
                               const std::string &source_id,
                               const std::string &condition,
                               AlertSeverity severity,
                               const std::string &message_template);

        /**
         * @brief Update alert rule
         * @param rule_id Alert rule identifier
         * @param condition Updated condition
         * @param severity Updated severity
         * @param message_template Updated message template
         * @return true if alert rule updated successfully
         */
        bool update_alert_rule(const std::string &rule_id,
                               const std::string &condition,
                               AlertSeverity severity,
                               const std::string &message_template);

        /**
         * @brief Delete alert rule
         * @param rule_id Alert rule identifier
         * @return true if alert rule deleted successfully
         */
        bool delete_alert_rule(const std::string &rule_id);

        /**
         * @brief Get active alerts
         * @param severity_filter Minimum severity level
         * @param limit Maximum number of alerts to return
         * @return Vector of active alerts
         */
        std::vector<DashboardAlert> get_active_alerts(AlertSeverity severity_filter = AlertSeverity::INFO,
                                                      uint32_t limit = 100) const;

        /**
         * @brief Acknowledge alert
         * @param alert_id Alert identifier
         * @param user_id User acknowledging the alert
         * @param comment Optional comment
         * @return true if alert acknowledged successfully
         */
        bool acknowledge_alert(const std::string &alert_id,
                               const std::string &user_id,
                               const std::string &comment = "");

        /**
         * @brief Clear acknowledged alerts
         * @param older_than_hours Clear alerts older than specified hours
         * @return Number of alerts cleared
         */
        uint32_t clear_acknowledged_alerts(uint32_t older_than_hours = 24);

        // User Management
        /**
         * @brief Authenticate user
         * @param username Username
         * @param password Password
         * @return Session information if authentication successful
         */
        std::unique_ptr<UserSession> authenticate_user(const std::string &username,
                                                       const std::string &password);

        /**
         * @brief Create user session
         * @param user_id User identifier
         * @param permissions User permissions
         * @param ip_address Client IP address
         * @param user_agent Client user agent
         * @return Session information
         */
        std::unique_ptr<UserSession> create_session(const std::string &user_id,
                                                    const std::vector<std::string> &permissions,
                                                    const std::string &ip_address = "",
                                                    const std::string &user_agent = "");

        /**
         * @brief Validate session
         * @param session_id Session identifier
         * @return true if session is valid
         */
        bool validate_session(const std::string &session_id);

        /**
         * @brief End user session
         * @param session_id Session identifier
         * @return true if session ended successfully
         */
        bool end_session(const std::string &session_id);

        /**
         * @brief Get active sessions
         * @return Vector of active user sessions
         */
        std::vector<UserSession> get_active_sessions() const;

        // Customization
        /**
         * @brief Set custom CSS
         * @param layout_id Layout identifier
         * @param css_content CSS content
         * @return true if CSS set successfully
         */
        bool set_custom_css(const std::string &layout_id, const std::string &css_content);

        /**
         * @brief Set custom JavaScript
         * @param layout_id Layout identifier
         * @param js_content JavaScript content
         * @return true if JavaScript set successfully
         */
        bool set_custom_javascript(const std::string &layout_id, const std::string &js_content);

        /**
         * @brief Add custom theme
         * @param theme_id Theme identifier
         * @param theme_config Theme configuration
         * @return true if theme added successfully
         */
        bool add_custom_theme(const std::string &theme_id,
                              const std::map<std::string, std::string> &theme_config);

        // REST API
        /**
         * @brief Enable REST API
         * @param enable Enable or disable REST API
         * @param api_prefix API URL prefix (e.g., "/api/v1")
         * @return true if successful
         */
        bool enable_rest_api(bool enable, const std::string &api_prefix = "/api/v1");

        /**
         * @brief Add custom API endpoint
         * @param method HTTP method ("GET", "POST", "PUT", "DELETE")
         * @param path API path
         * @param handler Request handler function
         * @return true if endpoint added successfully
         */
        bool add_api_endpoint(const std::string &method,
                              const std::string &path,
                              std::function<std::string(const std::map<std::string, std::string> &)> handler);

        /**
         * @brief Set API authentication
         * @param auth_type Authentication type ("none", "basic", "bearer", "api_key")
         * @param auth_config Authentication configuration
         * @return true if authentication set successfully
         */
        bool set_api_authentication(const std::string &auth_type,
                                    const std::map<std::string, std::string> &auth_config);

        // WebSocket Support
        /**
         * @brief Enable WebSocket support
         * @param enable Enable or disable WebSocket
         * @param ws_path WebSocket endpoint path
         * @return true if successful
         */
        bool enable_websocket(bool enable, const std::string &ws_path = "/ws");

        /**
         * @brief Broadcast message to all WebSocket clients
         * @param message Message to broadcast
         * @param channel Optional channel filter
         * @return Number of clients that received the message
         */
        uint32_t broadcast_websocket_message(const std::string &message,
                                             const std::string &channel = "");

        /**
         * @brief Send message to specific WebSocket client
         * @param session_id Session identifier
         * @param message Message to send
         * @return true if message sent successfully
         */
        bool send_websocket_message(const std::string &session_id, const std::string &message);

        // Export and Import
        /**
         * @brief Export dashboard configuration
         * @param layout_id Layout identifier
         * @param file_path Output file path
         * @param format Export format ("json", "yaml")
         * @return true if export successful
         */
        bool export_dashboard(const std::string &layout_id,
                              const std::string &file_path,
                              const std::string &format = "json") const;

        /**
         * @brief Import dashboard configuration
         * @param file_path Input file path
         * @param format Import format ("json", "yaml")
         * @return true if import successful
         */
        bool import_dashboard(const std::string &file_path,
                              const std::string &format = "json");

        /**
         * @brief Export data
         * @param source_id Data source identifier
         * @param file_path Output file path
         * @param format Export format ("json", "csv", "parquet")
         * @param start_time Start timestamp
         * @param end_time End timestamp
         * @return true if export successful
         */
        bool export_data(const std::string &source_id,
                         const std::string &file_path,
                         const std::string &format = "json",
                         uint64_t start_time = 0,
                         uint64_t end_time = 0) const;

        // Event Handling
        /**
         * @brief Set data update callback
         * @param callback Function to call when data is updated
         */
        void set_data_update_callback(DataUpdateCallback callback);

        /**
         * @brief Set alert callback
         * @param callback Function to call when alert is triggered
         */
        void set_alert_callback(AlertCallback callback);

        /**
         * @brief Set user event callback
         * @param callback Function to call for user events
         */
        void set_user_event_callback(UserEventCallback callback);

        // Configuration and Status
        /**
         * @brief Get server status
         * @return Server status information
         */
        std::map<std::string, std::string> get_server_status() const;

        /**
         * @brief Get dashboard statistics
         * @return Dashboard statistics
         */
        std::map<std::string, uint64_t> get_statistics() const;

        /**
         * @brief Update configuration
         * @param config New configuration parameters
         * @return true if update successful
         */
        bool update_configuration(const std::map<std::string, std::string> &config);

        /**
         * @brief Get current configuration
         * @return Current configuration parameters
         */
        std::map<std::string, std::string> get_configuration() const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

    /**
     * @brief Dashboard builder utility
     */
    class DashboardBuilder
    {
    public:
        /**
         * @brief Create new dashboard builder
         * @param layout_id Layout identifier
         * @param name Layout name
         */
        DashboardBuilder(const std::string &layout_id, const std::string &name);

        /**
         * @brief Set dashboard theme
         * @param theme Dashboard theme
         * @return Reference to builder for chaining
         */
        DashboardBuilder &set_theme(DashboardTheme theme);

        /**
         * @brief Set grid dimensions
         * @param columns Number of columns
         * @param rows Number of rows
         * @return Reference to builder for chaining
         */
        DashboardBuilder &set_grid_size(uint32_t columns, uint32_t rows);

        /**
         * @brief Add line chart widget
         * @param title Widget title
         * @param data_sources Data sources for the chart
         * @param x_pos X position
         * @param y_pos Y position
         * @param width Widget width
         * @param height Widget height
         * @return Reference to builder for chaining
         */
        DashboardBuilder &add_line_chart(const std::string &title,
                                         const std::vector<std::string> &data_sources,
                                         uint32_t x_pos, uint32_t y_pos,
                                         uint32_t width = 6, uint32_t height = 4);

        /**
         * @brief Add gauge widget
         * @param title Widget title
         * @param data_source Data source for the gauge
         * @param min_value Minimum value
         * @param max_value Maximum value
         * @param x_pos X position
         * @param y_pos Y position
         * @param width Widget width
         * @param height Widget height
         * @return Reference to builder for chaining
         */
        DashboardBuilder &add_gauge(const std::string &title,
                                    const std::string &data_source,
                                    double min_value, double max_value,
                                    uint32_t x_pos, uint32_t y_pos,
                                    uint32_t width = 3, uint32_t height = 3);

        /**
         * @brief Add metric card widget
         * @param title Widget title
         * @param data_source Data source for the metric
         * @param unit Value unit
         * @param x_pos X position
         * @param y_pos Y position
         * @param width Widget width
         * @param height Widget height
         * @return Reference to builder for chaining
         */
        DashboardBuilder &add_metric_card(const std::string &title,
                                          const std::string &data_source,
                                          const std::string &unit,
                                          uint32_t x_pos, uint32_t y_pos,
                                          uint32_t width = 3, uint32_t height = 2);

        /**
         * @brief Build dashboard layout
         * @return Complete dashboard layout
         */
        DashboardLayout build();

    private:
        DashboardLayout layout_;
        uint32_t next_widget_id_;
    };

    // Utility Functions
    /**
     * @brief Convert widget type to string
     * @param type Widget type
     * @return String representation
     */
    std::string widget_type_to_string(WidgetType type);

    /**
     * @brief Convert alert severity to string
     * @param severity Alert severity
     * @return String representation
     */
    std::string alert_severity_to_string(AlertSeverity severity);

    /**
     * @brief Convert dashboard theme to string
     * @param theme Dashboard theme
     * @return String representation
     */
    std::string dashboard_theme_to_string(DashboardTheme theme);

    /**
     * @brief Generate widget ID
     * @param prefix Widget prefix
     * @return Unique widget identifier
     */
    std::string generate_widget_id(const std::string &prefix = "widget");

    /**
     * @brief Validate widget configuration
     * @param config Widget configuration to validate
     * @return true if configuration is valid
     */
    bool validate_widget_config(const WidgetConfig &config);

} // namespace cardano_iot::monitoring

#endif // CARDANO_IOT_REALTIME_DASHBOARD_H
