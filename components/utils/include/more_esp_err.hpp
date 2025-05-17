#ifndef MORE_ESP_ERR_HPP
#define MORE_ESP_ERR_HPP

extern "C" {
#include "esp_err.h"
#include "esp_log.h"
}

/**
 * Macro which can be used to check the error code. If the code is not ESP_OK, it prints the message and returns.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_RETURN_ON_ERROR(x, log_tag, format, ...) \
        do {                                             \
            esp_err_t err_rc_ = (x);                     \
            if (unlikely(err_rc_ != ESP_OK)) {           \
                return err_rc_;                          \
            }                                            \
        } while (0)
#else
    #define ESP_RETURN_ON_ERROR(x, log_tag, format, ...)                                     \
        do {                                                                                 \
            esp_err_t err_rc_ = (x);                                                         \
            if (unlikely(err_rc_ != ESP_OK)) {                                               \
                ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                return err_rc_;                                                              \
            }                                                                                \
        } while (0)
#endif

/**
 * A version of ESP_RETURN_ON_ERROR() macro that can be called from ISR.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_RETURN_ON_ERROR_ISR(x, log_tag, format, ...) \
        do {                                                 \
            esp_err_t err_rc_ = (x);                         \
            if (unlikely(err_rc_ != ESP_OK)) {               \
                return err_rc_;                              \
            }                                                \
        } while (0)
#else
    #define ESP_RETURN_ON_ERROR_ISR(x, log_tag, format, ...)                                       \
        do {                                                                                       \
            esp_err_t err_rc_ = (x);                                                               \
            if (unlikely(err_rc_ != ESP_OK)) {                                                     \
                ESP_EARLY_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                return err_rc_;                                                                    \
            }                                                                                      \
        } while (0)
#endif

/**
 * Macro which can be used to check the error code. If the code is not ESP_OK, it prints the message,
 * sets the local variable 'ret' to the code, and then exits by jumping to 'goto_tag'.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_GOTO_ON_ERROR(x, goto_tag, log_tag, format, ...) \
        do {                                                     \
            esp_err_t err_rc_ = (x);                             \
            if (unlikely(err_rc_ != ESP_OK)) {                   \
                ret = err_rc_;                                   \
                goto goto_tag;                                   \
            }                                                    \
        } while (0)
#else
    #define ESP_GOTO_ON_ERROR(x, goto_tag, log_tag, format, ...)                             \
        do {                                                                                 \
            esp_err_t err_rc_ = (x);                                                         \
            if (unlikely(err_rc_ != ESP_OK)) {                                               \
                ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                ret = err_rc_;                                                               \
                goto goto_tag;                                                               \
            }                                                                                \
        } while (0)
#endif

/**
 * A version of ESP_GOTO_ON_ERROR() macro that can be called from ISR.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_GOTO_ON_ERROR_ISR(x, goto_tag, log_tag, format, ...) \
        do {                                                         \
            esp_err_t err_rc_ = (x);                                 \
            if (unlikely(err_rc_ != ESP_OK)) {                       \
                ret = err_rc_;                                       \
                goto goto_tag;                                       \
            }                                                        \
        } while (0)
#else
    #define ESP_GOTO_ON_ERROR_ISR(x, goto_tag, log_tag, format, ...)                               \
        do {                                                                                       \
            esp_err_t err_rc_ = (x);                                                               \
            if (unlikely(err_rc_ != ESP_OK)) {                                                     \
                ESP_EARLY_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                ret = err_rc_;                                                                     \
                goto goto_tag;                                                                     \
            }                                                                                      \
        } while (0)
#endif

/**
 * Macro which can be used to check the condition. If the condition is not 'true', it prints the message
 * and returns with the supplied 'err_code'.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_RETURN_ON_FALSE(a, err_code, log_tag, format, ...) \
        do {                                                       \
            if (unlikely(!(a))) {                                  \
                return err_code;                                   \
            }                                                      \
        } while (0)
#else
    #define ESP_RETURN_ON_FALSE(a, err_code, log_tag, format, ...)                           \
        do {                                                                                 \
            if (unlikely(!(a))) {                                                            \
                ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                return err_code;                                                             \
            }                                                                                \
        } while (0)
#endif

/**
 * A version of ESP_RETURN_ON_FALSE() macro that can be called from ISR.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_RETURN_ON_FALSE_ISR(a, err_code, log_tag, format, ...) \
        do {                                                           \
            if (unlikely(!(a))) {                                      \
                return err_code;                                       \
            }                                                          \
        } while (0)
#else
    #define ESP_RETURN_ON_FALSE_ISR(a, err_code, log_tag, format, ...)                             \
        do {                                                                                       \
            if (unlikely(!(a))) {                                                                  \
                ESP_EARLY_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                return err_code;                                                                   \
            }                                                                                      \
        } while (0)
#endif

/**
 * Macro which can be used to check the condition. If the condition is not 'true', it prints the message,
 * sets the local variable 'ret' to the supplied 'err_code', and then exits by jumping to 'goto_tag'.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_GOTO_ON_FALSE(a, err_code, goto_tag, log_tag, format, ...) \
        do {                                                               \
            if (unlikely(!(a))) {                                          \
                ret = err_code;                                            \
                goto goto_tag;                                             \
            }                                                              \
        } while (0)
#else
    #define ESP_GOTO_ON_FALSE(a, err_code, goto_tag, log_tag, format, ...)                   \
        do {                                                                                 \
            if (unlikely(!(a))) {                                                            \
                ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                ret = err_code;                                                              \
                goto goto_tag;                                                               \
            }                                                                                \
        } while (0)
#endif

/**
 * A version of ESP_GOTO_ON_FALSE() macro that can be called from ISR.
 */
#if defined(CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT)
    #define ESP_GOTO_ON_FALSE_ISR(a, err_code, goto_tag, log_tag, format, ...) \
        do {                                                                   \
            if (unlikely(!(a))) {                                              \
                ret = err_code;                                                \
                goto goto_tag;                                                 \
            }                                                                  \
        } while (0)
#else
    #define ESP_GOTO_ON_FALSE_ISR(a, err_code, goto_tag, log_tag, format, ...)                     \
        do {                                                                                       \
            if (unlikely(!(a))) {                                                                  \
                ESP_EARLY_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                ret = err_code;                                                                    \
                goto goto_tag;                                                                     \
            }                                                                                      \
        } while (0)
#endif

#endif  // MORE_ESP_ERR_HPP
