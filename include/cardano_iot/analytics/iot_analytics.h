/**
 * @file iot_analytics.h
 * @brief Advanced Analytics Module for Cardano IoT SDK
 *
 * This module provides real-time analytics, anomaly detection,
 * predictive analytics, and business intelligence for IoT data.
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_ANALYTICS_H
#define CARDANO_IOT_ANALYTICS_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>
#include <chrono>

namespace cardano_iot::analytics
{

    /**
     * @brief Anomaly detection types
     */
    enum class AnomalyType
    {
        STATISTICAL_OUTLIER, // Statistical deviation
        PATTERN_BREAK,       // Pattern change
        THRESHOLD_VIOLATION, // Threshold exceeded
        TEMPORAL_ANOMALY,    // Time-based anomaly
        CORRELATION_BREAK,   // Correlation broken
        PREDICTIVE_DEVIATION // Prediction mismatch
    };

    /**
     * @brief Analytics time granularity
     */
    enum class TimeGranularity
    {
        REAL_TIME, // Sub-second
        SECOND,    // Per second
        MINUTE,    // Per minute
        HOUR,      // Per hour
        DAY,       // Per day
        WEEK,      // Per week
        MONTH      // Per month
    };

    /**
     * @brief Data point for analytics
     */
    struct DataPoint
    {
        std::string device_id;
        std::string metric_name;
        double value;
        uint64_t timestamp;
        std::map<std::string, std::string> tags;
        std::map<std::string, double> metadata;
    };

    /**
     * @brief Anomaly detection result
     */
    struct AnomalyResult
    {
        bool is_anomaly;
        AnomalyType type;
        double confidence_score; // 0.0 - 1.0
        double severity_level;   // 0.0 - 1.0
        std::string description;
        std::string recommendation;
        std::map<std::string, double> context_data;
        uint64_t detection_time;
    };

    /**
     * @brief Predictive forecast result
     */
    struct ForecastResult
    {
        std::string metric_name;
        std::vector<std::pair<uint64_t, double>> predictions; // timestamp, value
        double confidence_interval;
        std::string model_type;
        std::map<std::string, double> model_metrics;
    };

    /**
     * @brief Analytics insight
     */
    struct AnalyticsInsight
    {
        std::string insight_type;
        std::string title;
        std::string description;
        double impact_score; // Business impact 0.0 - 1.0
        std::vector<std::string> affected_devices;
        std::map<std::string, std::string> recommendations;
        uint64_t generated_time;
    };

    /**
     * @brief Real-time dashboard metrics
     */
    struct DashboardMetrics
    {
        // Device metrics
        uint32_t total_devices;
        uint32_t active_devices;
        uint32_t offline_devices;
        uint32_t error_devices;

        // Data metrics
        uint64_t total_data_points;
        double data_throughput_per_second;
        uint64_t blockchain_transactions;

        // Performance metrics
        double avg_response_time_ms;
        double cpu_usage_percent;
        double memory_usage_mb;
        double network_bandwidth_mbps;

        // Business metrics
        uint32_t active_automations;
        uint32_t anomalies_detected;
        double energy_efficiency_score;
        double cost_savings_usd;

        uint64_t last_updated;
    };

    /**
     * @brief Advanced IoT Analytics Engine
     *
     * Provides comprehensive analytics capabilities including:
     * - Real-time anomaly detection
     * - Predictive analytics and forecasting
     * - Pattern recognition and classification
     * - Business intelligence and insights
     * - Performance monitoring and optimization
     */
    class IoTAnalytics
    {
    public:
        using AnomalyCallback = std::function<void(const AnomalyResult &)>;
        using InsightCallback = std::function<void(const AnalyticsInsight &)>;
        using MetricsCallback = std::function<void(const DashboardMetrics &)>;

        /**
         * @brief Constructor
         */
        IoTAnalytics();

        /**
         * @brief Destructor
         */
        ~IoTAnalytics();

        /**
         * @brief Initialize analytics engine
         * @param config Configuration parameters
         * @return true if initialization successful
         */
        bool initialize(const std::map<std::string, std::string> &config = {});

        /**
         * @brief Shutdown analytics engine
         */
        void shutdown();

        // Data Ingestion
        /**
         * @brief Add data point for analysis
         * @param data_point Data point to analyze
         * @return true if successfully added
         */
        bool ingest_data_point(const DataPoint &data_point);

        /**
         * @brief Add batch of data points
         * @param data_points Vector of data points
         * @return Number of successfully ingested points
         */
        uint32_t ingest_data_batch(const std::vector<DataPoint> &data_points);

        // Anomaly Detection
        /**
         * @brief Configure anomaly detection
         * @param device_id Device to monitor (empty for all devices)
         * @param metric_name Metric to monitor (empty for all metrics)
         * @param sensitivity Sensitivity level (0.0 - 1.0, higher = more sensitive)
         * @return true if configuration successful
         */
        bool configure_anomaly_detection(const std::string &device_id = "",
                                         const std::string &metric_name = "",
                                         double sensitivity = 0.8);

        /**
         * @brief Detect anomalies in real-time
         * @param data_point Current data point
         * @return Anomaly detection result
         */
        AnomalyResult detect_anomaly(const DataPoint &data_point);

        /**
         * @brief Get historical anomalies
         * @param device_id Device filter (empty for all)
         * @param start_time Start timestamp
         * @param end_time End timestamp
         * @return Vector of historical anomalies
         */
        std::vector<AnomalyResult> get_historical_anomalies(const std::string &device_id = "",
                                                            uint64_t start_time = 0,
                                                            uint64_t end_time = 0) const;

        // Predictive Analytics
        /**
         * @brief Generate forecast for metric
         * @param device_id Device identifier
         * @param metric_name Metric to forecast
         * @param forecast_horizon_hours Hours to forecast ahead
         * @return Forecast result
         */
        ForecastResult generate_forecast(const std::string &device_id,
                                         const std::string &metric_name,
                                         uint32_t forecast_horizon_hours = 24);

        /**
         * @brief Train predictive model
         * @param device_id Device identifier
         * @param metric_name Metric name
         * @param model_type Model type ("linear", "arima", "lstm", "prophet")
         * @return true if training successful
         */
        bool train_predictive_model(const std::string &device_id,
                                    const std::string &metric_name,
                                    const std::string &model_type = "auto");

        // Pattern Recognition
        /**
         * @brief Detect patterns in time series data
         * @param device_id Device identifier
         * @param metric_name Metric name
         * @param pattern_type Pattern type to detect
         * @return Detected patterns
         */
        std::vector<std::string> detect_patterns(const std::string &device_id,
                                                 const std::string &metric_name,
                                                 const std::string &pattern_type = "auto");

        /**
         * @brief Cluster similar devices based on behavior
         * @param metric_names Metrics to use for clustering
         * @param num_clusters Number of clusters (0 = auto-detect)
         * @return Device clusters
         */
        std::map<std::string, std::vector<std::string>> cluster_devices(
            const std::vector<std::string> &metric_names,
            uint32_t num_clusters = 0);

        // Business Intelligence
        /**
         * @brief Generate insights from data
         * @param insight_type Type of insight ("performance", "cost", "efficiency", "all")
         * @return Generated insights
         */
        std::vector<AnalyticsInsight> generate_insights(const std::string &insight_type = "all");

        /**
         * @brief Calculate ROI metrics
         * @param baseline_period_days Days for baseline calculation
         * @return ROI metrics map
         */
        std::map<std::string, double> calculate_roi_metrics(uint32_t baseline_period_days = 30);

        /**
         * @brief Generate cost analysis
         * @param cost_model Cost model parameters
         * @return Cost analysis results
         */
        std::map<std::string, double> generate_cost_analysis(
            const std::map<std::string, double> &cost_model);

        // Real-time Monitoring
        /**
         * @brief Get real-time dashboard metrics
         * @return Current dashboard metrics
         */
        DashboardMetrics get_dashboard_metrics() const;

        /**
         * @brief Get device performance metrics
         * @param device_id Device identifier
         * @param time_range_hours Hours of history to analyze
         * @return Performance metrics map
         */
        std::map<std::string, double> get_device_performance(const std::string &device_id,
                                                             uint32_t time_range_hours = 24) const;

        /**
         * @brief Get system health score
         * @return Health score (0.0 - 1.0, higher is better)
         */
        double get_system_health_score() const;

        // Data Aggregation
        /**
         * @brief Aggregate data by time window
         * @param device_id Device identifier
         * @param metric_name Metric name
         * @param granularity Time granularity
         * @param aggregation_func Aggregation function ("mean", "sum", "max", "min", "count")
         * @param start_time Start timestamp
         * @param end_time End timestamp
         * @return Aggregated data points
         */
        std::vector<std::pair<uint64_t, double>> aggregate_data(
            const std::string &device_id,
            const std::string &metric_name,
            TimeGranularity granularity,
            const std::string &aggregation_func = "mean",
            uint64_t start_time = 0,
            uint64_t end_time = 0) const;

        /**
         * @brief Calculate correlation matrix
         * @param metric_names Metrics to correlate
         * @param time_range_hours Hours of data to analyze
         * @return Correlation matrix
         */
        std::map<std::string, std::map<std::string, double>> calculate_correlation_matrix(
            const std::vector<std::string> &metric_names,
            uint32_t time_range_hours = 24) const;

        // Event Handling
        /**
         * @brief Set anomaly detection callback
         * @param callback Function to call when anomaly detected
         */
        void set_anomaly_callback(AnomalyCallback callback);

        /**
         * @brief Set insights callback
         * @param callback Function to call when insights generated
         */
        void set_insights_callback(InsightCallback callback);

        /**
         * @brief Set metrics update callback
         * @param callback Function to call when metrics updated
         */
        void set_metrics_callback(MetricsCallback callback);

        // Configuration
        /**
         * @brief Update analytics configuration
         * @param config New configuration parameters
         * @return true if update successful
         */
        bool update_configuration(const std::map<std::string, std::string> &config);

        /**
         * @brief Get current configuration
         * @return Current configuration parameters
         */
        std::map<std::string, std::string> get_configuration() const;

        // Utility Functions
        /**
         * @brief Export analytics results to file
         * @param file_path Output file path
         * @param format Export format ("json", "csv", "parquet")
         * @param filters Export filters
         * @return true if export successful
         */
        bool export_analytics_data(const std::string &file_path,
                                   const std::string &format = "json",
                                   const std::map<std::string, std::string> &filters = {}) const;

        /**
         * @brief Import historical data for training
         * @param file_path Input file path
         * @param format File format ("json", "csv", "parquet")
         * @return Number of data points imported
         */
        uint32_t import_historical_data(const std::string &file_path,
                                        const std::string &format = "json");

        /**
         * @brief Clear analytics data
         * @param older_than_days Clear data older than specified days (0 = all)
         * @return Number of data points cleared
         */
        uint32_t clear_analytics_data(uint32_t older_than_days = 0);

        /**
         * @brief Get analytics statistics
         * @return Statistics map
         */
        std::map<std::string, uint64_t> get_statistics() const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

    /**
     * @brief Convert anomaly type to string
     * @param type Anomaly type
     * @return String representation
     */
    std::string anomaly_type_to_string(AnomalyType type);

    /**
     * @brief Convert time granularity to string
     * @param granularity Time granularity
     * @return String representation
     */
    std::string time_granularity_to_string(TimeGranularity granularity);

    /**
     * @brief Calculate statistical measures
     * @param values Vector of values
     * @return Map of statistical measures (mean, std, min, max, etc.)
     */
    std::map<std::string, double> calculate_statistics(const std::vector<double> &values);

    /**
     * @brief Detect outliers using IQR method
     * @param values Vector of values
     * @param multiplier IQR multiplier (default 1.5)
     * @return Indices of outliers
     */
    std::vector<size_t> detect_outliers_iqr(const std::vector<double> &values, double multiplier = 1.5);

    /**
     * @brief Normalize time series data
     * @param values Vector of values
     * @param method Normalization method ("minmax", "zscore", "robust")
     * @return Normalized values
     */
    std::vector<double> normalize_time_series(const std::vector<double> &values,
                                              const std::string &method = "minmax");

} // namespace cardano_iot::analytics

#endif // CARDANO_IOT_ANALYTICS_H
