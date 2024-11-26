#ifndef S3_HTTPS_CLIENT_H
#define S3_HTTPS_CLIENT_H

/**
 * @file S3HttpsClient.h
 * @brief Provides functions for interacting with the S3 HTTPS client.
 */

#include "FreeRTOS.h"   /**< FreeRTOS includes for task management and types */


/* ============================ Data Structures ============================ */

/**
 * @brief Structure representing a custom HTTP header.
 */
typedef struct {
    const char* key;   /**< The key/name of the HTTP header. */
    const char* value; /**< The value of the HTTP header. */
} HTTPCustomHeader_t;

/**
 * @brief Structure representing the context for an S3 upload.
 */
typedef struct {
    const char* payload;      /**< Pointer to the payload data to be uploaded. */
    uint32_t payloadLength;   /**< Length of the payload data in bytes. */
    uint32_t sentBytes;       /**< Number of bytes already sent. */
} S3UploadContext;

/* ============================ Return Codes ============================ */

/** @brief Success code for operations */
#define S3_CLIENT_SUCCESS        0   /**< Operation was successful */

/** @brief General error code */
#define S3_CLIENT_ERROR          -1  /**< General error */

/** @brief TLS-related errors */
#define S3_CLIENT_TLS_ERROR      -2  /**< Error in TLS setup or connection */

/** @brief HTTP-related errors */
#define S3_CLIENT_HTTP_ERROR     -3  /**< Error in HTTP request or response */

/** @brief Network-related errors */
#define S3_CLIENT_NETWORK_ERROR  -4  /**< Error in establishing or maintaining network connection */

/** @brief Input parameters related errors */
#define S3_CLIENT_INVALID_PARAM  -5  /**< Error processing input parameters*/


/**
 * @brief Initializes the S3 client module.
 * 
 * This function initializes any resources required by the S3 client and prepares
 * the module for establishing connections with the S3.
 * 
 * @return S3_CLIENT_SUCCESS if initialization was successful, 
 *         S3_CLIENT_ERROR if there was an error.
 */
int S3Client_Init(void);

/**
 * @brief Establishes an HTTPS connection to the S3 endpoint.
 * 
 * This function attempts to connect to the S3 endpoint over HTTPS. 
 * It returns S3_CLIENT_SUCCESS if the connection was successfully established, 
 * or an error code if the connection failed.
 * 
 * @return S3_CLIENT_SUCCESS if the connection was successful, 
 *         S3_CLIENT_CONN_FAILED if the connection failed.
 */
int S3Client_Connect(void);

/**
 * @brief Sends a POST request to upload an object to AWS S3.
 * 
 * This function sends a POST request to the S3 endpoint to upload an object.
 *
 * @param[in] payload Pointer to the data to be uploaded.
 * @param[in] payloadLength Length of the data to be uploaded.
 * @param[in] userHeaders Pointer to an array of custom HTTP headers.
 * @param[in] headerCount Number of custom HTTP headers.
 *
 * @return S3_CLIENT_SUCCESS on success, or an appropriate error code on failure.
 */
int S3Client_Post(const char *payload, 
                        uint32_t payloadLength, 
                        HTTPCustomHeader_t* userHeaders, 
                        uint8_t headerCount);

/**
 * @brief Closes the connection to the S3 endpoint.
 * 
 * This function closes the established HTTPS connection to the S3 endpoint.
 * 
 * @return S3_CLIENT_SUCCESS if the disconnection was successful, 
 *         S3_CLIENT_DISCONNECT_FAILED if the disconnection failed.
 */
int S3Client_Disconnect(void);

/**
 * @brief Task function to test S3 functionality.
 * 
 * This FreeRTOS task function is used to test the S3 client operations such as 
 * establishing a connection, uploading and downloading objects, and disconnecting.
 * 
 * @param pvParameters Parameters passed to the task.
 */
void vS3ConnectTask(void *pvParameters);

#endif /* S3_HTTPS_CLIENT_H */
