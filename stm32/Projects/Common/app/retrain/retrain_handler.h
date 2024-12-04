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
 * @brief Task function for retraining processing.
 *
 * This function is responsible for handling the retraining process in the application.
 *
 * @param pvParameters Pointer to the parameters passed to the task. 
 */
void vRetrainProcessingTask(void* pvParameters);

#endif // RETRAIN_HANDLER_H
