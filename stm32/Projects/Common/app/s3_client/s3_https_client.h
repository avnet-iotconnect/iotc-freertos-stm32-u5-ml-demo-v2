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

/** @brief Server returned a bad response */
#define S3_CLIENT_BAD_RESPONSE   -6  /**< Server returned an error response */


/**
 * @brief Initializes the S3 client module.
 * 
 * This function initializes any resources required by the S3 client and prepares
 * the module for establishing connections with the S3.
 * 
 * @return S3_CLIENT_SUCCESS if initialization was successful, 
 *         S3_CLIENT_ERROR if there was an error.
 * 
 * @usage
 * @code
 * int result = S3Client_Init();
 * if (result == S3_CLIENT_SUCCESS) {
 *     // Initialization successful
 * } else {
 *     // Handle error
 * }
 * @endcode
 */
int S3Client_Init(void);

/**
 * @brief Establishes an HTTPS connection to the S3 endpoint.
 * 
 * This function attempts to connect to the S3 endpoint over HTTPS using the provided hostname and path.
 * It returns S3_CLIENT_SUCCESS if the connection was successfully established, 
 * or an error code if the connection failed.
 * 
 * @param[in] hostnameWithPath The full hostname and path of the S3 endpoint.
 * 
 * @return S3_CLIENT_SUCCESS if the connection was successful, 
 *         S3_CLIENT_ERROR if the connection failed.
 * 
 * @usage
 * @code
 * const char *hostnameWithPath = "s3.amazonaws.com/mybucket";
 * int result = S3Client_Connect(hostnameWithPath);
 * if (result == S3_CLIENT_SUCCESS) {
 *     // Connection successful
 * } else {
 *     // Handle error
 * }
 * @endcode
 */
int S3Client_Connect(const char *hostnameWithPath);

/**
 * @brief Sends a POST request to upload an object to AWS S3.
 * 
 * This function sends a POST request to the S3 endpoint to upload an object.
 *
 * @param[in] hostnameWithPath The full hostname and path of the S3 endpoint.
 * @param[in] payload Pointer to the data to be uploaded.
 * @param[in] payloadLength Length of the data to be uploaded.
 * @param[in] userHeaders Pointer to an array of custom HTTP headers.
 * @param[in] headerCount Number of custom HTTP headers.
 *
 * @return S3_CLIENT_SUCCESS on success, or an appropriate error code on failure.
 * 
 * @usage
 * @code
 * const char *hostnameWithPath = "s3.amazonaws.com/mybucket";
 * const char *payload = "data to upload";
 * uint32_t payloadLength = strlen(payload);
 * HTTPCustomHeader_t headers[] = {{"Content-Type", "application/octet-stream"}};
 * int headerCount = sizeof(headers) / sizeof(headers[0]);
 * int result = S3Client_Post(hostnameWithPath, payload, payloadLength, headers, headerCount);
 * if (result == S3_CLIENT_SUCCESS) {
 *     // Upload successful
 * } else {
 *     // Handle error
 * }
 * @endcode
 */
int S3Client_Post(const char *hostnameWithPath, const char *payload, 
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
 * 
 * @usage
 * @code
 * int result = S3Client_Disconnect();
 * if (result == S3_CLIENT_SUCCESS) {
 *     // Disconnection successful
 * } else {
 *     // Handle error
 * }
 * @endcode
 */
int S3Client_Disconnect(void);

#endif /* S3_HTTPS_CLIENT_H */
