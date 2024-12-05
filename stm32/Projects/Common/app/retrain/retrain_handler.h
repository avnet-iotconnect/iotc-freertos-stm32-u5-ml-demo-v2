#ifndef RETRAIN_HANDLER_H
#define RETRAIN_HANDLER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "retrain_handler_types.h"
#include <string.h>
#include <stdlib.h>

// Opaque handle for message handler
typedef struct RetrainHandlerContext* RetrainHandlerHandle_t;

/**
 * @brief Initialize the retrain handler.
 *
 * This function initializes the retrain handler by creating the message queue
 * and setting up the internal context. It must be called before any other
 * retrain handler functions are used.
 *
 * @return RetrainHandlerStatus_t
 * - RETRAIN_HANDLER_OK if the initialization was successful.
 * - RETRAIN_HANDLER_ERR_QUEUE_CREATION if the message queue could not be created.
 */
RetrainHandlerStatus_t RetrainHandler_init(void);

/**
 * @brief Enqueue a retrain data message for processing.
 *
 * This function enqueues a retrain data message into the retrain handler's
 * message queue. The message will be processed by the retrain processing task.
 *
 * @param[in] message Pointer to the retrain data message to enqueue.
 *
 * @return RetrainHandlerStatus_t
 * - RETRAIN_HANDLER_OK if the message was successfully enqueued.
 * - RETRAIN_HANDLER_ERR_INVALID_MESSAGE if the message pointer is NULL.
 * - RETRAIN_HANDLER_ERR_INVALID_BUFFER if the message buffer is invalid.
 * - RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW if the message buffer size exceeds the maximum allowed size.
 * - RETRAIN_HANDLER_ERR_QUEUE_FULL if the message queue is full and the message could not be enqueued.
 */
RetrainHandlerStatus_t RetrainData_enqueue(const RetrainData_t* message);

/**
 * @brief Set the buffer data.
 *
 * This function copies provided data to the the internal static buffer.
 *
 * @param[in] data Pointer to the data buffer.
 * @param[in] size Size of the data buffer.
 *
 * @return RetrainHandlerStatus_t
 * - RETRAIN_HANDLER_OK if the buffer data was successfully set.
 * - RETRAIN_HANDLER_ERR_INVALID_BUFFER if the input data is NULL or the buffer size is invalid.
 * - RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW if the buffer size exceeds the maximum allowed size.
 */
RetrainHandlerStatus_t RetrainHandler_SetBufferData(const uint8_t* data, size_t size);

/**
 * @brief Set the buffer data with an offset.
 *
 * This function copies provided data to the internal static buffer at the specified offset.
 *
 * @param[in] data Pointer to the data buffer.
 * @param[in] size Size of the data buffer.
 * @param[in] offset Offset in the internal buffer where the data should be copied.
 *
 * @return RetrainHandlerStatus_t
 * - RETRAIN_HANDLER_OK if the buffer data was successfully set.
 * - RETRAIN_HANDLER_ERR_INVALID_BUFFER if the input data is NULL or the buffer size is invalid.
 * - RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW if the buffer size exceeds the maximum allowed size.
 */
RetrainHandlerStatus_t RetrainHandler_SetBufferDataWithOffset(const uint8_t* data, size_t size, size_t offset);

/**
 * @brief Enqueue the internal buffer data with classification for retraining.
 *
 * This function enqueues the buffer data along with the classification
 * string into the retrain handler's message queue.
 *
 * @param[in] classification Pointer to the classification string.
 *
 * @return RetrainHandlerStatus_t
 * - RETRAIN_HANDLER_OK if the message was successfully enqueued.
 * - RETRAIN_HANDLER_ERR_INVALID_MESSAGE if the classification string is NULL or invalid.
 * - RETRAIN_HANDLER_ERR_QUEUE_FULL if the message queue is full and the message could not be enqueued.
 */
RetrainHandlerStatus_t RetrainHandler_EnqueueBufferData(const char* classification);

/**
 * @brief Get the class name corresponding to the provided index.
 *
 * This function returns a pointer to the class name string from the
 * CTRL_X_CUBE_AI_MODE_CLASS_LIST based on the provided index.
 *
 * @param[in] index The class index (1 to 6).
 *
 * @return const char* Pointer to the class name string, or NULL if the index is out of range.
 */
const char* RetrainHandler_GetClassName(uint8_t index);

/**
 * @brief Task function for retraining processing.
 *
 * This function is responsible for handling the retraining process in the application.
 *
 * @param pvParameters Pointer to the parameters passed to the task. 
 */
void vRetrainProcessingTask(void* pvParameters);

#endif // RETRAIN_HANDLER_H
