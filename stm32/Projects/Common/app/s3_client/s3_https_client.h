#ifndef S3_HTTPS_CLIENT_H
#define S3_HTTPS_CLIENT_H

#include "FreeRTOS.h"

/**
 * @brief Initializes the S3 client module.
 */
void S3Client_Init(void);

/**
 * @brief Establishes an HTTPS connection to the S3 endpoint.
 *
 * @return pdTRUE if the connection was successful, otherwise pdFALSE.
 */
BaseType_t S3Client_Connect(void);

/**
 * @brief Sends a PUT request to upload an object to S3.
 *
 * @param[in] payload Pointer to the data to be uploaded.
 * @param[in] payloadLength Length of the data in bytes.
 *
 * @return pdTRUE if the PUT request was successful, otherwise pdFALSE.
 */
BaseType_t S3Client_PutObject(const char *payload, uint32_t payloadLength);

/**
 * @brief Sends a GET request to download an object from S3.
 *
 * @return Pointer to the downloaded data on success, or NULL on failure.
 */
const char *S3Client_GetObject(void);

/**
 * @brief Closes the connection to the S3 endpoint.
 */
void S3Client_Disconnect(void);

/**
 * @brief Task function to test S3 functionality
 */
void vS3ConnectTask( void * pvParameters );

#endif /* S3_CLIENT_H */