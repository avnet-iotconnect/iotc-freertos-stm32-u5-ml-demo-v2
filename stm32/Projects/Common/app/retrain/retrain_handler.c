/**
 * @file retrain_handler.c
 * @brief Retrain Handler Implementation
 *
 * This module provides functions to initialize, enqueue messages, and process
 * retrain data messages.
 */

#include "logging_levels.h"

/* Define LOG_LEVEL here if you want to modify the logging level from the default */
#define LOG_LEVEL LOG_DEBUG
#include "logging.h"

#include "retrain_handler.h"
#include "app/s3_client/s3_https_client.h"

/* ============================ Constants and Macros ============================ */

/* Maximum size of the message queue */
#define MAX_QUEUE_SIZE 10

/* Timeout for sending messages to the queue (in milliseconds) */
#define QUEUE_SEND_TIMEOUT_MS 500

/* ============================ Static Variables ============================ */

/* Internal context structure */
struct RetrainHandlerContext {
    QueueHandle_t retrain_queue;        /**< FreeRTOS queue handle */
    TaskHandle_t processing_task;       /**< Task handling message processing */
    uint32_t total_messages_processed;  /**< Performance tracking */
    bool is_initialized;                /**< Initialization state */
};

/* Static allocation for low-overhead scenarios */
static struct RetrainHandlerContext s_default_context = {0};

/* ============================ Static Function Declarations ============================ */

/**
 * @brief Validate the retrain data message.
 *
 * This function validates the retrain data message to ensure it meets the
 * required criteria.
 *
 * @param[in] message Pointer to the retrain data message to validate.
 *
 * @return RetrainHandlerStatus_t
 * - RETRAIN_HANDLER_OK if the message is valid.
 * - RETRAIN_HANDLER_ERR_INVALID_BUFFER if the message buffer is invalid.
 * - RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW if the message buffer size exceeds the maximum allowed size.
 */
static RetrainHandlerStatus_t prvValidateMessageData(const RetrainData_t* message);

/* ============================ Function Implementations ============================ */

RetrainHandlerStatus_t RetrainHandler_init(void) {
    RetrainHandlerHandle_t handler = &s_default_context;

    /* Create message queue */
    handler->retrain_queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(RetrainData_t));

    if (handler->retrain_queue == NULL) {
        LogError("Failed to create retrain_queue");
        return RETRAIN_HANDLER_ERR_QUEUE_CREATION;
    }

    handler->is_initialized = true;

    return RETRAIN_HANDLER_OK;
}

RetrainHandlerStatus_t RetrainData_enqueue(const RetrainData_t* message) {
    RetrainHandlerHandle_t handler = &s_default_context;

    /* Validate message */
    if (message == NULL) {
        return RETRAIN_HANDLER_ERR_INVALID_MESSAGE;
    }

    /* Validate message data */
    RetrainHandlerStatus_t validation_status = prvValidateMessageData(message);
    if (validation_status != RETRAIN_HANDLER_OK) {
        return validation_status;
    }

    /* Attempt to send message to queue */
    BaseType_t queue_status = xQueueSend(handler->retrain_queue, message, pdMS_TO_TICKS(QUEUE_SEND_TIMEOUT_MS));

    if (queue_status == pdPASS) {
        LogDebug("Message enqueued successfully");
        return RETRAIN_HANDLER_OK;
    } else {
        LogError("Failed to enqueue message, queue might be full");
        return RETRAIN_HANDLER_ERR_QUEUE_FULL;
    }
}

void vRetrainProcessingTask(void* pvParameters) {
    (void)pvParameters; /* Cast to void to avoid unused parameter warning */

    RetrainHandlerHandle_t handler = &s_default_context;
    RetrainData_t received_message;

    if (handler->is_initialized == false) {
        LogError("Context is not initialized");
        vTaskDelete(NULL);
    }

    for (;;) {
        /* Wait for message with timeout */
        BaseType_t receive_status = xQueueReceive(handler->retrain_queue, &received_message, portMAX_DELAY);

        if (receive_status == pdTRUE) {
            LogDebug("Retrain data received.\n\rData size: %d", received_message.buffer_size);

            int init_result = S3Client_Init();
            if (init_result != S3_CLIENT_SUCCESS) {
                LogError("Failed to initialize S3 client, error code: %d", init_result);
                continue;
            }

            int connect_result = S3Client_Connect();
            if (connect_result != S3_CLIENT_SUCCESS) {
                LogError("Failed to connect to S3 client, error code: %d", connect_result);
                continue;
            }

            HTTPCustomHeader_t headers[] = {
                {"Content-Type", "audio/wav"},
                {"x-api-key", "DkIxv0zK8T7qHHajtc5y58182rBycj6V7OTMzsEe"},
                {"sound-classes", "Alarm"}
            };

            int result = S3Client_Post(
                received_message.buffer,
                received_message.buffer_size,
                headers,
                sizeof(headers) / sizeof(headers[0])
            );
            printf("Large file upload result: %d\n", result);

            int disconnect_result = S3Client_Disconnect();
            if (disconnect_result != S3_CLIENT_SUCCESS) {
                LogError("Failed to disconnect from AWS S3. Error code: %d", disconnect_result);
            }
        }
    }
}

static RetrainHandlerStatus_t prvValidateMessageData(const RetrainData_t* message) {
    if (message->buffer == NULL) {
        LogError("Invalid message buffer: NULL");
        return RETRAIN_HANDLER_ERR_INVALID_BUFFER;
    }

    if (message->buffer_size > RETRAIN_MAX_BUFFER_SIZE) {
        LogError("Message buffer size exceeds maximum: %d", message->buffer_size);
        return RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW;
    }

    return RETRAIN_HANDLER_OK;
}
