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
#include "mqtt_handler.h"
#include "app/s3_client/s3_https_client.h"
#include "ai_model_config.h"
#include "kvstore.h"

/* ============================ Constants and Macros ============================ */

/* Maximum size of the message queue */
#define MAX_QUEUE_SIZE 10

/* Timeout for sending messages to the queue (in milliseconds) */
#define QUEUE_SEND_TIMEOUT_MS 500

/* Maximum size of the audio buffer (64 KB) */
#define AUDIO_BUFFER_SIZE (64 * 1024)

/* Content type for audio data */
#define CONTENT_TYPE_AUDIO "audio/wav"

#define S3_API_KEY_LEN 255
#define S3_ENDPOINT_LEN 255
/* ============================ Static Variables ============================ */

/* Internal context structure */
struct RetrainHandlerContext {
    QueueHandle_t retrain_queue;        /**< FreeRTOS queue handle */
    TaskHandle_t processing_task;       /**< Task handling message processing */
    uint32_t total_messages_processed;  /**< Performance tracking */
    bool is_initialized;                /**< Initialization state */
    uint8_t audio_buffer[AUDIO_BUFFER_SIZE]; /**< Static buffer for audio data transfer */
};

/* Static allocation for low-overhead scenarios */
static struct RetrainHandlerContext s_default_context = {0};

/* Static array for class names */
static const char* class_list[] = CTRL_X_CUBE_AI_MODE_CLASS_LIST;

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

RetrainHandlerStatus_t RetrainHandler_SetBufferData(const uint8_t* data, size_t size) {
    RetrainHandlerHandle_t handler = &s_default_context;

    /* Check if handler is initialized */
    if (!handler->is_initialized) {
        LogError("Handler is not initialized");
        return RETRAIN_HANDLER_ERR_INVALID_BUFFER;
    }

    /* Validate input data */
    if (data == NULL) {
        LogError("Invalid input data: NULL");
        return RETRAIN_HANDLER_ERR_INVALID_BUFFER;
    }

    /* Validate buffer size */
    if (size > AUDIO_BUFFER_SIZE) {
        LogError("Input data size exceeds audio buffer size: %d", size);
        return RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW;
    }

    /* Copy data to the audio buffer */
    memcpy(handler->audio_buffer, data, size);

    return RETRAIN_HANDLER_OK;
}

RetrainHandlerStatus_t RetrainHandler_SetBufferDataWithOffset(const uint8_t* data, size_t size, size_t offset) {
    RetrainHandlerHandle_t handler = &s_default_context;

    /* Check if handler is initialized */
    if (!handler->is_initialized) {
        LogError("Handler is not initialized");
        return RETRAIN_HANDLER_ERR_INVALID_BUFFER;
    }

    /* Validate input data */
    if (data == NULL) {
        LogError("Invalid input data: NULL");
        return RETRAIN_HANDLER_ERR_INVALID_BUFFER;
    }

    /* Validate buffer size */
    if ((offset + size) > AUDIO_BUFFER_SIZE) {
        LogError("Input data size exceeds audio buffer size: %d", size);
        return RETRAIN_HANDLER_ERR_BUFFER_OVERFLOW;
    }

    /* Copy data to the audio buffer at the specified offset */
    memcpy(&handler->audio_buffer[offset], data, size);
    LogDebug("Buffer set at offset %lu with size %lu", offset, size);

    return RETRAIN_HANDLER_OK;
}

RetrainHandlerStatus_t RetrainHandler_EnqueueBufferData(const char* classification) {
    RetrainHandlerHandle_t handler = &s_default_context;

    /* Check if handler is initialized */
    if (!handler->is_initialized) {
        LogError("Handler is not initialized");
        return RETRAIN_HANDLER_ERR_INVALID_BUFFER;
    }

    /* Validate classification string */
    if (classification == NULL) {
        LogError("Invalid classification string: NULL");
        return RETRAIN_HANDLER_ERR_INVALID_MESSAGE;
    }

    if (strlen(classification) >= RETRAIN_MAX_CLASSIFICATION_LEN) {
        LogError("Classification string exceeds maximum length: %d", strlen(classification));
        return RETRAIN_HANDLER_ERR_INVALID_MESSAGE;
    }

    LogDebug("Enqueuing buffer data with classification: %s", classification);

    /* Prepare the message */
    RetrainData_t message;
    message.buffer = handler->audio_buffer;
    message.buffer_size = AUDIO_BUFFER_SIZE;
    strncpy(message.classification, classification, RETRAIN_MAX_CLASSIFICATION_LEN);

    LogDebug("Message prepared with buffer size: %d", message.buffer_size);

    /* Enqueue the message */
    RetrainHandlerStatus_t enqueue_status = RetrainData_enqueue(&message);

    return enqueue_status;
}
/*-----------------------------------------------------------*/

void vRetrainProcessingTask(void* pvParameters) {
    (void)pvParameters; /* Cast to void to avoid unused parameter warning */

    RetrainHandlerHandle_t handler = &s_default_context;
    RetrainData_t received_message;
    char pcS3ApiKey[S3_API_KEY_LEN];
    char pcS3Endpoint[S3_ENDPOINT_LEN];

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
            
            char pcTopicString[256] = {0};
            size_t uxS3ApiKeyLen = KVStore_getString(CS_S3_API_KEY, pcS3ApiKey, S3_API_KEY_LEN);
            size_t uxS3EndpointLen = KVStore_getString(CS_S3_ENDPOINT, pcS3Endpoint, S3_ENDPOINT_LEN);
            if(uxS3ApiKeyLen == 0 || uxS3EndpointLen == 0)
            {
                char pcDeviceId[64];
                size_t uxDevNameLen = KVStore_getString(CS_CORE_THING_NAME, pcDeviceId, 64);

                size_t uxTopicLen = 0;
                if (uxDevNameLen > 0)
                {
                    sprintf(pcTopicString, "$aws/rules/msg_d2c_rpt/%s/2.1/0", pcDeviceId);
                    uxTopicLen = strlen(pcTopicString);
                }
               
                if ((uxTopicLen == 0) || (uxTopicLen >= 256))
                {
                    LogError("Failed to construct topic string. Please configure the device");
                    while (true) {
                        vTaskDelay( 10000 ); // wait forever
                    }
                }

                // Prepare the payload
                char payloadBuf[512];
                int bytesWritten = snprintf(payloadBuf, 512, "{\"d\":[{\"d\":{\"requests3\":\"True\"}}]}");

                if (bytesWritten < 512)
                {
                    // Publish the payload to the topic
                    bool result = PublishPayloadToTopic(pcTopicString, payloadBuf, (size_t)bytesWritten);
                    if (result)
                    {
                        LogDebug("Payload: %s", payloadBuf);
                        LogDebug("Topic name: %s", pcTopicString);
                    }
                }
                else if (bytesWritten > 0)
                {
                    LogError("Not enough buffer space.");
                }
                else
                {
                    LogError("Failed to construct payload.");
                }
                
                LogInfo("Waiting for S3 API Key and Endpoint to be obtained...");
                while(0 == KVStore_getString(CS_S3_API_KEY, pcS3ApiKey, S3_API_KEY_LEN) ||
                     0 == KVStore_getString(CS_S3_ENDPOINT, pcS3Endpoint, S3_ENDPOINT_LEN))
                {
                    vTaskDelay(1000);
                }
            }

            // Here we assume that the S3 API Key and Endpoint are already obtained
            // and we can use them to connect to the S3 client
            int connect_result = S3Client_Connect( pcS3Endpoint );
            if (connect_result != S3_CLIENT_SUCCESS) {
                LogError("Failed to connect to S3 client, error code: %d", connect_result);
                continue;
            }

            HTTPCustomHeader_t headers[] = {
                {"Content-Type", CONTENT_TYPE_AUDIO},
                {"x-api-key", pcS3ApiKey},
                {"sound-classes", received_message.classification} // Use received classification
            };

            int result = S3Client_Post(
                pcS3Endpoint,
                received_message.buffer,
                received_message.buffer_size,
                headers,
                sizeof(headers) / sizeof(headers[0])
            );

            LogDebug("S3Client_Post completed with result: %d", result);
            
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

/**
 * @brief Get the class name corresponding to the provided index.
 *
 * @param[in] index The class index (1 to 6).
 *
 * @return const char* Pointer to the class name string, or NULL if the index is out of range.
 */
const char* RetrainHandler_GetClassName(uint8_t index) {
    if (index < 1 || index > CTRL_X_CUBE_AI_MODE_CLASS_NUMBER) {
        LogError("Invalid class index: %d", index);
        return NULL;
    }

    return class_list[index - 1];
}
