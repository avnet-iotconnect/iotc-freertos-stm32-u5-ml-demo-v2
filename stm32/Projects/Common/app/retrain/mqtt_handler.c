#include "mqtt_handler.h"

/* MQTT library includes. */
#include "core_mqtt.h"
#include "core_mqtt_agent.h"
#include "subscription_manager.h"
#include "mqtt_agent_task.h"
#include "sys_evt.h"

#include <string.h>

#define MQTT_PUBLISH_MAX_LEN (512)
#define MQTT_PUBLISH_TIME_BETWEEN_MS (5000)
#define MQTT_PUBLISH_TOPIC "mic_sensor_data"
#define MQTT_PUBLICH_TOPIC_STR_LEN (256)
#define IOTC_CD_MAX_LEN (10)
#define MQTT_PUBLISH_BLOCK_TIME_MS (1000)
#define MQTT_PUBLISH_NOTIFICATION_WAIT_MS (1000)

#define MQTT_NOTIFY_IDX (1)
#define MQTT_PUBLISH_QOS (MQTTQoS0)

/**
 * @brief Defines the structure to use as the command callback context in this
 * demo.
 */
struct MQTTAgentCommandContext
{
	MQTTStatus_t xReturnStatus;
	TaskHandle_t xTaskToNotify;
};


static void prvPublishCommandCallback(MQTTAgentCommandContext_t *pxCommandContext,
									  MQTTAgentReturnInfo_t *pxReturnInfo)
{
	configASSERT(pxCommandContext != NULL);
	configASSERT(pxReturnInfo != NULL);

	pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

	if (pxCommandContext->xTaskToNotify != NULL)
	{
		/* Send the context's ulNotificationValue as the notification value so
		 * the receiving task can check the value it set in the context matches
		 * the value it receives in the notification. */
		(void)xTaskNotifyGiveIndexed(pxCommandContext->xTaskToNotify,
									 MQTT_NOTIFY_IDX);
	}
}


static BaseType_t prvPublishAndWaitForAck(MQTTAgentHandle_t xAgentHandle,
										  const char *pcTopic,
										  const void *pvPublishData,
										  size_t xPublishDataLen)
{
	BaseType_t xResult = pdFALSE;
	MQTTStatus_t xStatus;

	configASSERT(pcTopic != NULL);
	configASSERT(pvPublishData != NULL);
	configASSERT(xPublishDataLen > 0);

	MQTTPublishInfo_t xPublishInfo =
		{
			.qos = MQTT_PUBLISH_QOS,
			.retain = 0,
			.dup = 0,
			.pTopicName = pcTopic,
			.topicNameLength = (uint16_t)strlen(pcTopic),
			.pPayload = pvPublishData,
			.payloadLength = xPublishDataLen};

	MQTTAgentCommandContext_t xCommandContext =
		{
			.xTaskToNotify = xTaskGetCurrentTaskHandle(),
			.xReturnStatus = MQTTIllegalState,
		};

	MQTTAgentCommandInfo_t xCommandParams =
		{
			.blockTimeMs = MQTT_PUBLISH_BLOCK_TIME_MS,
			.cmdCompleteCallback = prvPublishCommandCallback,
			.pCmdCompleteCallbackContext = &xCommandContext,
		};

	/* Clear the notification index */
	xTaskNotifyStateClearIndexed(NULL, MQTT_NOTIFY_IDX);

	xStatus = MQTTAgent_Publish(xAgentHandle,
								&xPublishInfo,
								&xCommandParams);

	if (xStatus == MQTTSuccess)
	{
		xResult = ulTaskNotifyTakeIndexed(MQTT_NOTIFY_IDX,
										  pdTRUE,
										  pdMS_TO_TICKS(MQTT_PUBLISH_NOTIFICATION_WAIT_MS));

		if (xResult == 0)
		{
			LogError("Timed out while waiting for publish ACK or Sent event. xTimeout = %d",
					 pdMS_TO_TICKS(MQTT_PUBLISH_NOTIFICATION_WAIT_MS));
			xResult = pdFALSE;
		}
		else if (xCommandContext.xReturnStatus != MQTTSuccess)
		{
			LogError("MQTT Agent returned error code: %d during publish operation.",
					 xCommandContext.xReturnStatus);
			xResult = pdFALSE;
		}
	}
	else
	{
		LogError("MQTTAgent_Publish returned error code: %d.",
				 xStatus);
	}

	return xResult;
}


static BaseType_t xIsMqttConnected(void)
{
	/* Wait for MQTT to be connected */
	EventBits_t uxEvents = xEventGroupWaitBits(xSystemEvents,
											   EVT_MASK_MQTT_CONNECTED,
											   pdFALSE,
											   pdTRUE,
											   0);

	return ((uxEvents & EVT_MASK_MQTT_CONNECTED) == EVT_MASK_MQTT_CONNECTED);
}

bool PublishPayloadToTopic(const char *pcTopic, const char *pcPayload, uint32_t xPayloadLen)
{
    // Check if MQTT is connected
    if (xIsMqttConnected() != pdTRUE)
    {
        LogError("MQTT is not connected.");
        return false;
    }

    // Get the MQTT agent handle
    MQTTAgentHandle_t xAgentHandle = xGetMqttAgentHandle();
    if (xAgentHandle == NULL)
    {
        LogError("Failed to get MQTT agent handle.");
        return false;
    }

    // Publish the payload to the specified topic
    BaseType_t xResult = prvPublishAndWaitForAck(xAgentHandle, pcTopic, pcPayload, xPayloadLen);
    if (xResult != pdTRUE)
    {
        LogError("Failed to publish payload to topic: %s", pcTopic);
        return false;
    }
    else
    {
        LogDebug("Successfully published payload to topic: %s", pcTopic);
        return true;
    }
}
