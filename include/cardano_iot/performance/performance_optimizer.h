/**
 * @file performance_optimizer.h
 * @brief Performance Optimization Module for Cardano IoT SDK
 *
 * This module provides advanced performance optimization features including
 * automatic tuning, resource management, load balancing, and system optimization.
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_PERFORMANCE_OPTIMIZER_H
#define CARDANO_IOT_PERFORMANCE_OPTIMIZER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>
#include <atomic>
#include <thread>

namespace cardano_iot::performance
{

    /**
     * @brief Optimization strategies
     */
    enum class OptimizationStrategy
    {
        THROUGHPUT_FOCUSED,  // Maximize data throughput
        LATENCY_FOCUSED,     // Minimize response latency
        ENERGY_FOCUSED,      // Minimize energy consumption
        RELIABILITY_FOCUSED, // Maximize system reliability
        COST_FOCUSED,        // Minimize operational costs
        BALANCED,            // Balanced optimization
        ADAPTIVE             // Adaptive strategy based on conditions
    };

    /**
     * @brief Performance metrics
     */
    struct PerformanceMetrics
    {
        // Throughput metrics
        double transactions_per_second;
        double data_points_per_second;
        double blockchain_ops_per_second;

        // Latency metrics
        double avg_response_time_ms;
        double p95_response_time_ms;
        double p99_response_time_ms;
        double max_response_time_ms;

        // Resource utilization
        double cpu_usage_percent;
        double memory_usage_mb;
        double memory_usage_percent;
        double network_bandwidth_mbps;
        double disk_io_mbps;

        // Energy metrics
        double power_consumption_mw;
        double energy_efficiency_score;
        double battery_drain_rate;

        // System health
        double error_rate_percent;
        double uptime_percent;
        uint32_t active_connections;
        uint32_t queue_depth;

        // Blockchain specific
        double blockchain_sync_percent;
        uint32_t pending_transactions;
        double transaction_success_rate;

        uint64_t measurement_time;
    };

    /**
     * @brief Optimization recommendation
     */
    struct OptimizationRecommendation
    {
        std::string category;                          // "cpu", "memory", "network", "energy", etc.
        std::string description;                       // Human-readable description
        std::string action;                            // Specific action to take
        double expected_improvement;                   // Expected improvement (0.0 - 1.0)
        double confidence_score;                       // Confidence in recommendation (0.0 - 1.0)
        uint32_t priority;                             // Priority level (1-10, higher = more important)
        std::map<std::string, std::string> parameters; // Action parameters
        bool auto_applicable;                          // Can be applied automatically
    };

    /**
     * @brief Resource allocation settings
     */
    struct ResourceAllocation
    {
        // Thread allocation
        uint32_t worker_threads;
        uint32_t io_threads;
        uint32_t network_threads;

        // Memory allocation
        uint64_t buffer_size_mb;
        uint64_t cache_size_mb;
        uint64_t max_memory_mb;

        // Network settings
        uint32_t max_connections;
        uint32_t connection_timeout_ms;
        uint32_t read_timeout_ms;
        uint32_t write_timeout_ms;

        // Queue settings
        uint32_t max_queue_size;
        uint32_t batch_size;
        uint32_t flush_interval_ms;

        // Retry settings
        uint32_t max_retries;
        uint32_t retry_delay_ms;
        double backoff_multiplier;
    };

    /**
     * @brief Load balancing configuration
     */
    struct LoadBalancingConfig
    {
        std::string strategy; // "round_robin", "least_connections", "weighted", "adaptive"
        std::vector<std::string> endpoints;
        std::map<std::string, double> weights; // For weighted strategy
        uint32_t health_check_interval_ms;
        uint32_t failure_threshold;
        uint32_t recovery_threshold;
    };

    /**
     * @brief Performance Optimizer
     *
     * Provides comprehensive performance optimization including:
     * - Automatic performance tuning
     * - Resource allocation optimization
     * - Load balancing and distribution
     * - Energy efficiency optimization
     * - Real-time performance monitoring
     */
    class PerformanceOptimizer
    {
    public:
        using MetricsCallback = std::function<void(const PerformanceMetrics &)>;
        using RecommendationCallback = std::function<void(const OptimizationRecommendation &)>;

        /**
         * @brief Constructor
         */
        PerformanceOptimizer();

        /**
         * @brief Destructor
         */
        ~PerformanceOptimizer();

        /**
         * @brief Initialize performance optimizer
         * @param config Configuration parameters
         * @return true if initialization successful
         */
        bool initialize(const std::map<std::string, std::string> &config = {});

        /**
         * @brief Shutdown performance optimizer
         */
        void shutdown();

        // Performance Monitoring
        /**
         * @brief Start performance monitoring
         * @param monitoring_interval_ms Monitoring interval in milliseconds
         * @return true if monitoring started successfully
         */
        bool start_monitoring(uint32_t monitoring_interval_ms = 1000);

        /**
         * @brief Stop performance monitoring
         */
        void stop_monitoring();

        /**
         * @brief Get current performance metrics
         * @return Current performance metrics
         */
        PerformanceMetrics get_current_metrics() const;

        /**
         * @brief Get historical performance metrics
         * @param start_time Start timestamp
         * @param end_time End timestamp
         * @param granularity_ms Granularity in milliseconds
         * @return Vector of historical metrics
         */
        std::vector<PerformanceMetrics> get_historical_metrics(uint64_t start_time,
                                                               uint64_t end_time,
                                                               uint32_t granularity_ms = 60000) const;

        // Optimization Engine
        /**
         * @brief Set optimization strategy
         * @param strategy Optimization strategy to use
         * @return true if strategy set successfully
         */
        bool set_optimization_strategy(OptimizationStrategy strategy);

        /**
         * @brief Enable automatic optimization
         * @param enable Enable or disable automatic optimization
         * @param optimization_interval_ms Interval between optimizations
         * @return true if successful
         */
        bool enable_auto_optimization(bool enable, uint32_t optimization_interval_ms = 60000);

        /**
         * @brief Run optimization analysis
         * @return Vector of optimization recommendations
         */
        std::vector<OptimizationRecommendation> analyze_performance();

        /**
         * @brief Apply optimization recommendation
         * @param recommendation Recommendation to apply
         * @return true if applied successfully
         */
        bool apply_optimization(const OptimizationRecommendation &recommendation);

        /**
         * @brief Apply all applicable optimizations
         * @param auto_only Only apply recommendations marked as auto-applicable
         * @return Number of optimizations applied
         */
        uint32_t apply_all_optimizations(bool auto_only = true);

        // Resource Management
        /**
         * @brief Optimize resource allocation
         * @param target_metrics Target performance metrics
         * @return Optimized resource allocation
         */
        ResourceAllocation optimize_resource_allocation(const PerformanceMetrics &target_metrics);

        /**
         * @brief Apply resource allocation
         * @param allocation Resource allocation to apply
         * @return true if applied successfully
         */
        bool apply_resource_allocation(const ResourceAllocation &allocation);

        /**
         * @brief Get current resource allocation
         * @return Current resource allocation
         */
        ResourceAllocation get_current_resource_allocation() const;

        /**
         * @brief Scale resources dynamically
         * @param scale_factor Scale factor (1.0 = no change, >1.0 = scale up, <1.0 = scale down)
         * @return true if scaling successful
         */
        bool scale_resources(double scale_factor);

        // Load Balancing
        /**
         * @brief Configure load balancing
         * @param config Load balancing configuration
         * @return true if configuration successful
         */
        bool configure_load_balancing(const LoadBalancingConfig &config);

        /**
         * @brief Add endpoint to load balancer
         * @param endpoint Endpoint URL
         * @param weight Weight for weighted strategies (default 1.0)
         * @return true if endpoint added successfully
         */
        bool add_endpoint(const std::string &endpoint, double weight = 1.0);

        /**
         * @brief Remove endpoint from load balancer
         * @param endpoint Endpoint URL to remove
         * @return true if endpoint removed successfully
         */
        bool remove_endpoint(const std::string &endpoint);

        /**
         * @brief Get next endpoint for request
         * @return Selected endpoint URL
         */
        std::string get_next_endpoint();

        /**
         * @brief Report endpoint health
         * @param endpoint Endpoint URL
         * @param is_healthy Health status
         * @param response_time_ms Response time in milliseconds
         */
        void report_endpoint_health(const std::string &endpoint,
                                    bool is_healthy,
                                    uint32_t response_time_ms = 0);

        // Energy Optimization
        /**
         * @brief Optimize for energy efficiency
         * @param target_efficiency Target efficiency score (0.0 - 1.0)
         * @return Energy optimization recommendations
         */
        std::vector<OptimizationRecommendation> optimize_energy_efficiency(double target_efficiency = 0.8);

        /**
         * @brief Enable dynamic power management
         * @param enable Enable or disable dynamic power management
         * @return true if successful
         */
        bool enable_dynamic_power_management(bool enable);

        /**
         * @brief Set power budget
         * @param power_budget_mw Power budget in milliwatts
         * @return true if budget set successfully
         */
        bool set_power_budget(double power_budget_mw);

        /**
         * @brief Get current energy efficiency score
         * @return Energy efficiency score (0.0 - 1.0)
         */
        double get_energy_efficiency_score() const;

        // Caching and Memory Optimization
        /**
         * @brief Configure intelligent caching
         * @param cache_size_mb Cache size in megabytes
         * @param eviction_policy Eviction policy ("lru", "lfu", "random", "ttl")
         * @param ttl_seconds TTL for TTL policy
         * @return true if configuration successful
         */
        bool configure_caching(uint64_t cache_size_mb,
                               const std::string &eviction_policy = "lru",
                               uint32_t ttl_seconds = 3600);

        /**
         * @brief Optimize memory usage
         * @return Memory optimization recommendations
         */
        std::vector<OptimizationRecommendation> optimize_memory_usage();

        /**
         * @brief Perform garbage collection
         * @param force_full_gc Force full garbage collection
         * @return Amount of memory freed (MB)
         */
        uint64_t perform_garbage_collection(bool force_full_gc = false);

        // Network Optimization
        /**
         * @brief Optimize network performance
         * @return Network optimization recommendations
         */
        std::vector<OptimizationRecommendation> optimize_network_performance();

        /**
         * @brief Enable connection pooling
         * @param enable Enable or disable connection pooling
         * @param pool_size Connection pool size
         * @param max_idle_time_ms Maximum idle time for connections
         * @return true if successful
         */
        bool enable_connection_pooling(bool enable,
                                       uint32_t pool_size = 10,
                                       uint32_t max_idle_time_ms = 30000);

        /**
         * @brief Configure compression
         * @param algorithm Compression algorithm ("gzip", "lz4", "zstd", "none")
         * @param compression_level Compression level (1-9)
         * @return true if configuration successful
         */
        bool configure_compression(const std::string &algorithm = "lz4",
                                   uint32_t compression_level = 3);

        // Benchmarking and Testing
        /**
         * @brief Run performance benchmark
         * @param duration_seconds Benchmark duration in seconds
         * @param workload_type Workload type ("light", "medium", "heavy", "stress")
         * @return Benchmark results
         */
        PerformanceMetrics run_benchmark(uint32_t duration_seconds = 60,
                                         const std::string &workload_type = "medium");

        /**
         * @brief Run stress test
         * @param duration_seconds Test duration in seconds
         * @param load_multiplier Load multiplier (1.0 = normal load)
         * @return Stress test results
         */
        std::map<std::string, double> run_stress_test(uint32_t duration_seconds = 300,
                                                      double load_multiplier = 2.0);

        /**
         * @brief Measure operation latency
         * @param operation_name Operation name for tracking
         * @param operation Function to measure
         * @return Latency in milliseconds
         */
        template <typename Operation>
        double measure_latency(const std::string &operation_name, Operation operation);

        // Event Handling
        /**
         * @brief Set metrics callback
         * @param callback Function to call when metrics updated
         */
        void set_metrics_callback(MetricsCallback callback);

        /**
         * @brief Set recommendation callback
         * @param callback Function to call when recommendations generated
         */
        void set_recommendation_callback(RecommendationCallback callback);

        // Configuration and Status
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

        /**
         * @brief Get optimization statistics
         * @return Statistics about optimizations performed
         */
        std::map<std::string, uint64_t> get_optimization_statistics() const;

        /**
         * @brief Reset performance counters
         */
        void reset_counters();

        /**
         * @brief Export performance data
         * @param file_path Output file path
         * @param format Export format ("json", "csv", "prometheus")
         * @return true if export successful
         */
        bool export_performance_data(const std::string &file_path,
                                     const std::string &format = "json") const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

    /**
     * @brief Performance profiler utility
     */
    class PerformanceProfiler
    {
    public:
        /**
         * @brief Start profiling session
         * @param session_name Name of profiling session
         */
        static void start_session(const std::string &session_name);

        /**
         * @brief End profiling session
         * @param session_name Name of profiling session
         * @return Profiling results
         */
        static std::map<std::string, double> end_session(const std::string &session_name);

        /**
         * @brief Profile a function
         * @param function_name Name for identification
         * @param function Function to profile
         * @return Execution time in milliseconds
         */
        template <typename Function>
        static double profile_function(const std::string &function_name, Function function);

        /**
         * @brief Get profiling report
         * @return Comprehensive profiling report
         */
        static std::string get_profiling_report();
    };

    /**
     * @brief RAII performance timer
     */
    class ScopedTimer
    {
    public:
        explicit ScopedTimer(const std::string &operation_name);
        ~ScopedTimer();

        double get_elapsed_ms() const;

    private:
        std::string operation_name_;
        std::chrono::high_resolution_clock::time_point start_time_;
    };

    // Utility Functions
    /**
     * @brief Convert optimization strategy to string
     * @param strategy Optimization strategy
     * @return String representation
     */
    std::string optimization_strategy_to_string(OptimizationStrategy strategy);

    /**
     * @brief Calculate performance score
     * @param metrics Performance metrics
     * @param weights Weights for different metrics
     * @return Overall performance score (0.0 - 1.0)
     */
    double calculate_performance_score(const PerformanceMetrics &metrics,
                                       const std::map<std::string, double> &weights = {});

    /**
     * @brief Estimate optimal thread count
     * @param workload_type Type of workload ("cpu_bound", "io_bound", "mixed")
     * @return Recommended thread count
     */
    uint32_t estimate_optimal_thread_count(const std::string &workload_type = "mixed");

    /**
     * @brief Estimate optimal buffer size
     * @param data_rate_mbps Expected data rate in Mbps
     * @param latency_target_ms Target latency in milliseconds
     * @return Recommended buffer size in bytes
     */
    uint64_t estimate_optimal_buffer_size(double data_rate_mbps, uint32_t latency_target_ms);

// Macros for easy profiling
#define PROFILE_SCOPE(name) cardano_iot::performance::ScopedTimer timer(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

} // namespace cardano_iot::performance

#endif // CARDANO_IOT_PERFORMANCE_OPTIMIZER_H
