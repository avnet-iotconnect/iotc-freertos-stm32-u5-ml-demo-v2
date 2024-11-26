/**
 * @file s3_https_client.c
 * @brief AWS S3 HTTPS Client Implementation
 *
 * This module provides functions to initialize, connect, interact with, and disconnect
 * from AWS S3 using HTTPS.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "logging_levels.h"

/* Define LOG_LEVEL here if you want to modify the logging level from the default */
#define LOG_LEVEL LOG_DEBUG

#include "s3_https_client.h"
#include "logging.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "core_http_client.h"
#include "mbedtls_transport.h"
#include "sys_evt.h"



/* ============================ Constants and Macros ============================ */

/* Buffer sizes for HTTP request and response */
#define REQUEST_BODY_BUFFER_SIZE    1024
#define RESPONSE_BODY_BUFFER_SIZE   1024

/* Buffer for HTTP headers */
#define HEADERS_BUFFER_SIZE 500

/* Maximum number of custom headers */
#define MAX_CUSTOM_HEADERS 10       

/* AWS S3 Configuration */
#define S3_HOSTNAME "u1cmgbltr6.execute-api.us-east-2.amazonaws.com"   /**< AWS S3 Hostname */
#define S3_HTTPS_PORT 443                /**< AWS S3 HTTPS Port */
#define S3_OBJECT_KEY "receive_file"   /**< AWS S3 Object Key */

/* Root CA Certificate for AWS S3 (STARFIELD_ROOT_CA_G2) */
#define STARFIELD_ROOT_CA_G2 \
"-----BEGIN CERTIFICATE-----\n"\
"MIID7zCCAtegAwIBAgIBADANBgkqhkiG9w0BAQsFADCBmDELMAkGA1UEBhMCVVMx\n"\
"EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxJTAjBgNVBAoT\n"\
"HFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xOzA5BgNVBAMTMlN0YXJmaWVs\n"\
"ZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5\n"\
"MDkwMTAwMDAwMFoXDTM3MTIzMTIzNTk1OVowgZgxCzAJBgNVBAYTAlVTMRAwDgYD\n"\
"VQQIEwdBcml6b25hMRMwEQYDVQQHEwpTY290dHNkYWxlMSUwIwYDVQQKExxTdGFy\n"\
"ZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTswOQYDVQQDEzJTdGFyZmllbGQgU2Vy\n"\
"dmljZXMgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIwDQYJKoZI\n"\
"hvcNAQEBBQADggEPADCCAQoCggEBANUMOsQq+U7i9b4Zl1+OiFOxHz/Lz58gE20p\n"\
"OsgPfTz3a3Y4Y9k2YKibXlwAgLIvWX/2h/klQ4bnaRtSmpDhcePYLQ1Ob/bISdm2\n"\
"8xpWriu2dBTrz/sm4xq6HZYuajtYlIlHVv8loJNwU4PahHQUw2eeBGg6345AWh1K\n"\
"Ts9DkTvnVtYAcMtS7nt9rjrnvDH5RfbCYM8TWQIrgMw0R9+53pBlbQLPLJGmpufe\n"\
"hRhJfGZOozptqbXuNC66DQO4M99H67FrjSXZm86B0UVGMpZwh94CDklDhbZsc7tk\n"\
"6mFBrMnUVN+HL8cisibMn1lUaJ/8viovxFUcdUBgF4UCVTmLfwUCAwEAAaNCMEAw\n"\
"DwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFJxfAN+q\n"\
"AdcwKziIorhtSpzyEZGDMA0GCSqGSIb3DQEBCwUAA4IBAQBLNqaEd2ndOxmfZyMI\n"\
"bw5hyf2E3F/YNoHN2BtBLZ9g3ccaaNnRbobhiCPPE95Dz+I0swSdHynVv/heyNXB\n"\
"ve6SbzJ08pGCL72CQnqtKrcgfU28elUSwhXqvfdqlS5sdJ/PHLTyxQGjhdByPq1z\n"\
"qwubdQxtRbeOlKyWN7Wg0I8VRw7j6IPdj/3vQQF3zCepYoUz8jcI73HPdwbeyBkd\n"\
"iEDPfUYd/x7H4c7/I9vG+o1VTqkC50cRRj70/b17KSa7qWFiNyi2LSr2EIZkyXCn\n"\
"0q23KXB56jzaYyWf/Wi3MOxw+3WKt21gZ7IeyLnp2KhvAotnDU0mV3HaIPzBSlCN\n"\
"sSi6\n"\
"-----END CERTIFICATE-----"

/* Uncomment the following line to enable dumping of user headers */
#define ENABLE_USER_HEADERS_DUMP

/* Uncomment the following line to enable dumping of payload data */
#define ENABLE_PAYLOAD_HEXDUMP
/* Number of bytes to display in each line of the payload hexdump */
#define PAYLOAD_HEXDUMP_BYTES 16

/* Uncomment the following line to enable dumping of HTTP response */
#define ENABLE_RESPONSE_DUMP

/* ============================ Static Variables ============================ */

/* HTTP Client and Network contexts */
static NetworkContext_t *ptrNetworkContext = NULL; /**< Pointer to the network context */
static TransportInterface_t transport_if = {0};      /**< Transport interface structure */

/* Buffers for HTTP operations */
static char requestBodyBuffer[REQUEST_BODY_BUFFER_SIZE];       /**< Buffer for HTTP request body */
static uint8_t responseBodyBuffer[RESPONSE_BODY_BUFFER_SIZE]; /**< Buffer for HTTP response body */

/* Buffer for HTTP headers */
static uint8_t headersBuffer[HEADERS_BUFFER_SIZE];

/* ============================ Static Function Declarations ============================ */

/* ============================ Function Implementations ============================ */

int S3Client_Init(void)
{
    LogDebug("Initializing S3 Client buffers.");

    /* Clear memory for request and response buffers */
    memset(requestBodyBuffer, 0, REQUEST_BODY_BUFFER_SIZE);
    memset(responseBodyBuffer, 0, RESPONSE_BODY_BUFFER_SIZE);

    LogDebug("S3 Client buffers initialized successfully.");

    return S3_CLIENT_SUCCESS;
}

int S3Client_Connect(void)

{
    LogInfo("Attempting to establish connection to AWS S3.");

    TlsTransportStatus_t tlsTransportStatus;
    const char *alpnProtocols[] = {NULL}; /* No ALPN required */

    /* Allocate network context for TLS transport */
    LogDebug("Allocating network context for TLS.");
    NetworkContext_t *networkContextPtr = mbedtls_transport_allocate();
    if (networkContextPtr == NULL)
    {
        LogError("Failed to allocate network context!");
        return S3_CLIENT_ERROR; // Return generic error code
    }
    LogDebug("Network context allocated successfully.");

    /* Configure TLS settings */
    LogDebug("Configuring TLS transport parameters.");
    PkiObject_t caCertificates[] = {
        PKI_OBJ_PEM((const unsigned char *)STARFIELD_ROOT_CA_G2, sizeof(STARFIELD_ROOT_CA_G2))
    };

    tlsTransportStatus = mbedtls_transport_configure(
        networkContextPtr,
        alpnProtocols,
        NULL,
        NULL,
        caCertificates,
        sizeof(caCertificates) / sizeof(caCertificates[0])
    );

    if (tlsTransportStatus != TLS_TRANSPORT_SUCCESS)
    {
        LogError("Failed to configure TLS transport! Error Code: %d", tlsTransportStatus);
        mbedtls_transport_free(networkContextPtr);
        return S3_CLIENT_TLS_ERROR; // Return TLS specific error code
    }
    LogDebug("TLS transport configured successfully.");

    /* Establish the TLS connection to AWS S3 */
    LogInfo("Connecting to AWS S3 at %s:%d.", S3_HOSTNAME, S3_HTTPS_PORT);
    tlsTransportStatus = mbedtls_transport_connect(networkContextPtr, S3_HOSTNAME, S3_HTTPS_PORT, 10000, 10000);
    if (tlsTransportStatus != TLS_TRANSPORT_SUCCESS)
    {
        LogError("Failed to connect to AWS S3! Error Code: %d", tlsTransportStatus);
        mbedtls_transport_free(networkContextPtr);
        return S3_CLIENT_NETWORK_ERROR; // Return network specific error code
    }
    LogInfo("Successfully connected to AWS S3.");

    /* Store the network context for later use */
    ptrNetworkContext = networkContextPtr;

    /* Initialize the transport interface with the network context and transport functions */
    transport_if.pNetworkContext = ptrNetworkContext;
    transport_if.send = mbedtls_transport_send;
    transport_if.recv = mbedtls_transport_recv;

    LogDebug("Transport interface initialized.");

    return S3_CLIENT_SUCCESS; // Return success code
}

int S3Client_Post(const char *payload, 
                        uint32_t payloadLength, 
                        HTTPCustomHeader_t* userHeaders, 
                        uint8_t headerCount)
{
    /* Initial logging and parameter validation */
    LogInfo("Initiating S3 Object Upload");
    LogDebug("Upload Parameters:");
    LogDebug("  Payload Address: %p", (void*)payload);
    LogDebug("  Payload Length: %lu bytes", payloadLength);
    LogDebug("  Custom Header Count: %u", headerCount);

    /* Validate input parameters */
    if (payload == NULL || payloadLength == 0)
    {
        LogError("Invalid parameters: payload or payloadLength is null or zero.");
        return S3_CLIENT_INVALID_PARAM;
    }

    /* Validate custom headers */
    if (headerCount > MAX_CUSTOM_HEADERS) {
        LogError("Too many custom headers. Maximum allowed: %d", MAX_CUSTOM_HEADERS);
        return S3_CLIENT_INVALID_PARAM;
    }

    /* Initialize HTTP request and response structures */
    HTTPRequestHeaders_t headers = {0};
    headers.pBuffer = headersBuffer;
    headers.bufferLen = sizeof(headersBuffer);

    HTTPStatus_t https_status = HTTPSuccess;
    LogDebug("Initializing HTTP request headers");
    
    /* Configure initial request headers */
    HTTPRequestInfo_t requestInfo = {0};
    requestInfo.pHost = S3_HOSTNAME;
    requestInfo.hostLen = strlen(S3_HOSTNAME);
    requestInfo.pPath = "/" S3_OBJECT_KEY;
    requestInfo.pathLen = strlen(requestInfo.pPath);
    requestInfo.pMethod = HTTP_METHOD_POST;
    requestInfo.methodLen = strlen(HTTP_METHOD_POST);
    requestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

    https_status = HTTPClient_InitializeRequestHeaders(&headers, &requestInfo);
    if (https_status != HTTPSuccess)
    {
        LogError("Failed to initialize HTTP headers! HTTP Status: %s", HTTPClient_strerror(https_status));
        return S3_CLIENT_HTTP_ERROR;
    }

    /* Add user-provided custom headers */
    for (uint8_t i = 0; i < headerCount; i++) {
        if (userHeaders[i].key && userHeaders[i].value) {
            LogDebug("Adding Custom Header: Key='%s', Value='%s'", 
                     userHeaders[i].key, userHeaders[i].value);
            
            HTTPClient_AddHeader(&headers, 
                                 userHeaders[i].key, 
                                 strlen(userHeaders[i].key), 
                                 userHeaders[i].value, 
                                 strlen(userHeaders[i].value));
        }
    }

    /* Prepare HTTP response structure */
    HTTPResponse_t response = {0};
    response.pBuffer = responseBodyBuffer;
    response.bufferLen = RESPONSE_BODY_BUFFER_SIZE;

    /* Send entire payload in single transmission */
    LogInfo("Sending entire payload in single transmission");
    LogDebug("Payload Size: %lu bytes", payloadLength);

    /* Optional: Hexdump of first few bytes (for debugging) */
    #ifdef ENABLE_PAYLOAD_HEXDUMP
    {
        LogDebug("First %d bytes of payload:", PAYLOAD_HEXDUMP_BYTES);
        for (uint32_t i = 0; i < (payloadLength < PAYLOAD_HEXDUMP_BYTES ? payloadLength : PAYLOAD_HEXDUMP_BYTES); i++) {
            LogDebug("%02X ", (unsigned char)payload[i]);
        }
    }
    #endif

    /* Transmit payload */
    https_status = HTTPClient_Send(&transport_if, 
                                   &headers, 
                                   (const uint8_t*)payload, 
                                   payloadLength, 
                                   &response, 
                                   0);

    /* Check transmission result */
    if (https_status != HTTPSuccess) {
        LogError("Payload upload failed! HTTP Status: %s", HTTPClient_strerror(https_status));
        return S3_CLIENT_HTTP_ERROR;
    }

    /* Optional: Response dump for debugging */
    #ifdef ENABLE_RESPONSE_DUMP
    {
        LogDebug("Response Dump:");
        LogDebug("%.*s", response.bufferLen, response.pBuffer);
    }
    #endif

    LogInfo("Upload complete. Total bytes sent: %lu", payloadLength);
    
    return S3_CLIENT_SUCCESS;
}

int S3Client_Disconnect(void)
{
    if (ptrNetworkContext != NULL)
    {
        LogInfo("Disconnecting from AWS S3.");
        mbedtls_transport_disconnect(ptrNetworkContext);
        mbedtls_transport_free(ptrNetworkContext);
        ptrNetworkContext = NULL;
        LogInfo("Connection to AWS S3 closed and resources freed.");
        return S3_CLIENT_SUCCESS;  // Return success code
    }
    else
    {
        LogWarn("Disconnect called, but network context is already NULL.");
        return S3_CLIENT_ERROR;    // Return error code if no network context
    }
}

void vS3ConnectTask( void * pvParameters )
{
    ( void ) pvParameters;

    /* Block until the network interface is connected */
	( void ) xEventGroupWaitBits( xSystemEvents,
								  EVT_MASK_NET_CONNECTED,
								  pdFALSE,
								  pdTRUE,
								  portMAX_DELAY );

    S3Client_Init();
    S3Client_Connect();

    /* Test code */
    const char *small_payload = "Hello, World!";

    HTTPCustomHeader_t headers[] = {
        {"Content-Type", "audio/wav"},
        {"x-api-key", "DkIxv0zK8T7qHHajtc5y58182rBycj6V7OTMzsEe"}
    };
    
    int result = S3Client_Post(
        small_payload, 
        strlen(small_payload), 
        headers, 
        sizeof(headers) / sizeof(headers[0])
    );
    printf("Large file upload result: %d\n", result);

    while(1)
    {

    }
}