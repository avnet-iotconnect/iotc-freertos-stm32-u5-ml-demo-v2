#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Publishes a payload to a specified MQTT topic.
 *
 * @param[in] pcTopic The topic to publish to.
 * @param[in] pcPayload The payload to publish.
 * @param[in] xPayloadLen The length of the payload.
 *
 * @return true if the publish was successful, false otherwise.
 */
bool PublishPayloadToTopic(const char *pcTopic, const char *pcPayload, uint32_t xPayloadLen);

#endif /* MQTT_HANDLER_H */
