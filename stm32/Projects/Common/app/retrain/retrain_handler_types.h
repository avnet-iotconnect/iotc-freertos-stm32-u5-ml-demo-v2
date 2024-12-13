/**
 * @file retrain_handler_types.h
 * @brief Type definitions for retrain handling system
 *
 * Provides type definitions for a retrain data handling.
 */

#ifndef RETRAIN_HANDLER_TYPES_H
#define RETRAIN_HANDLER_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** Maximum length for message classification strings */
#define RETRAIN_MAX_CLASSIFICATION_LEN 40

/** 
 * @brief Error codes 
 */
typedef enum {
    // Success Codes
    RETRAIN_HANDLER_OK = 0,               ///< Operation completed successfully
    
    // Memory Errors
    RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW,  ///< Buffer size exceeded maximum
    RETRAIN_HANDLER_ERR_INVALID_BUFFER,   ///< Invalid buffer pointer or properties
    
    // Queue Errors
    RETRAIN_HANDLER_ERR_QUEUE_FULL,       ///< Message queue is at maximum capacity
    RETRAIN_HANDLER_ERR_QUEUE_CREATION,  ///< Failed to create message queue

    // Processing Errors
    RETRAIN_HANDLER_ERR_INVALID_MESSAGE,  ///< Message fails validation

    // Writing Errors
    RETRAIN_HANDLER_ERR_WRITE_BLOCKED,     ///< Writing operation is blocked

    // Initialization Errors
    RETRAIN_HANDLER_ERR_NOT_INITIALIZED   ///< Handler not initialized
} RetrainHandlerStatus_t;

/** 
 * @brief Retrain data message structure
 */
typedef struct {
    void* buffer;                   ///< Pointer to message payload
    size_t buffer_size;             ///< Actual payload size
    char classification[RETRAIN_MAX_CLASSIFICATION_LEN]; ///< Classification string
} RetrainData_t;

#endif // RETRAIN_HANDLER_TYPES_H
