/*
 * Copyright (c) 2019, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MQTTClientMbedOs.h"
#include "MQTTNetworkUtil.h"

int MQTTNetworkMbedOs::read(unsigned char* buffer, int len, int timeout) {
    int rc;

    rc = accumulate_mqtt_read(socket, buffer, len, timeout);

    return (rc);
}

int MQTTNetworkMbedOs::write(unsigned char* buffer, int len, int timeout) {
    int rc;

    rc = mqtt_write(socket, buffer, len, timeout);

    return (rc);
}

int MQTTNetworkMbedOs::connect(const char* hostname, int port) {
    nsapi_error_t ret;

    SocketAddress sockAddr(hostname, port);
    ret = socket->connect(sockAddr);

    return (static_cast<int>(ret));
}

int MQTTNetworkMbedOs::disconnect() {
    nsapi_error_t ret;

    ret = socket->close();

    return (static_cast<int>(ret));
}

MQTTClient::MQTTClient(TCPSocket* _socket) {
    init(_socket);

    mqttNet = new MQTTNetworkMbedOs(socket);
    if (mqttNet != nullptr) {
        client  = new MQTT::Client<MQTTNetworkMbedOs, Countdown, MBED_CONF_MBED_MQTT_MAX_PACKET_SIZE,
                                   MBED_CONF_MBED_MQTT_MAX_CONNECTIONS>(*mqttNet);
    }
}

#if defined(MBEDTLS_SSL_CLI_C) || defined(DOXYGEN_ONLY)
MQTTClient::MQTTClient(TLSSocket* _socket) {
    init(_socket);

    mqttNet = new MQTTNetworkMbedOs(socket);
    if (mqttNet != nullptr) {
        client = new MQTT::Client<MQTTNetworkMbedOs, Countdown, MBED_CONF_MBED_MQTT_MAX_PACKET_SIZE,
                                  MBED_CONF_MBED_MQTT_MAX_CONNECTIONS>(*mqttNet);
    }
}
#endif

MQTTClient::MQTTClient(UDPSocket* _socket) {
    init(_socket);

    mqttNet = new MQTTNetworkMbedOs(socket);
    if (mqttNet != nullptr) {
        clientSN = new MQTTSN::Client<MQTTNetworkMbedOs, Countdown, MBED_CONF_MBED_MQTT_MAX_PACKET_SIZE,
                                      MBED_CONF_MBED_MQTT_MAX_CONNECTIONS>(*mqttNet);
    }
}

#if defined(MBEDTLS_SSL_CLI_C) || defined(DOXYGEN_ONLY)
MQTTClient::MQTTClient(DTLSSocket* _socket) {
    init(_socket);

    mqttNet = new MQTTNetworkMbedOs(socket);
    if (mqttNet != nullptr) {
        clientSN = new MQTTSN::Client<MQTTNetworkMbedOs, Countdown, MBED_CONF_MBED_MQTT_MAX_PACKET_SIZE,
                                      MBED_CONF_MBED_MQTT_MAX_CONNECTIONS>(*mqttNet);
    }
}
#endif

MQTTClient::~MQTTClient() {
    delete mqttNet;

    if (client != nullptr) {
        delete client;
    }

    if (clientSN != nullptr) {
        delete clientSN;
    }
}

nsapi_error_t MQTTClient::connect(MQTTPacket_connectData& options) {
    nsapi_error_t ret;

    if (client == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = client->connect(options);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::connect(MQTTSNPacket_connectData& options) {
    nsapi_error_t ret;

    if (clientSN == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = clientSN->connect(options);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::publish(const char* topicName, MQTT::Message& message) {
    nsapi_error_t ret;

    if (client == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = client->publish(topicName, message);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::publish(MQTTSN_topicid& topicName, MQTTSN::Message& message) {
    nsapi_error_t ret;

    if (clientSN == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = clientSN->publish(topicName, message);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::subscribe(const char* topicFilter, enum MQTT::QoS qos, messageHandler mh) {
    nsapi_error_t ret;

    if (client == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = client->subscribe(topicFilter, qos, mh);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::subscribe(MQTTSN_topicid& topicFilter, enum MQTTSN::QoS qos, messageHandlerSN mh) {
    nsapi_error_t ret;

    if (clientSN == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = clientSN->subscribe(topicFilter, qos, mh);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::unsubscribe(const char* topicFilter) {
    nsapi_error_t ret;

    if (client == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = client->unsubscribe(topicFilter);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::unsubscribe(MQTTSN_topicid& topicFilter) {
    nsapi_error_t ret;

    if (clientSN == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = clientSN->unsubscribe(topicFilter);
        if (ret < 0) {
            ret = NSAPI_ERROR_NO_CONNECTION;
        }
    }

    return (ret);
}

nsapi_error_t MQTTClient::yield(unsigned long timeout_ms) {
    nsapi_error_t ret;

    if (client != nullptr) {
        ret = client->yield(timeout_ms);
    }
    else if (clientSN != nullptr) {
        ret = clientSN->yield(timeout_ms);
    }
    else {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }

    return (ret);
}

nsapi_error_t MQTTClient::disconnect(void) {
    nsapi_error_t ret;

    if (client != nullptr) {
        ret = client->disconnect();
    }
    else if (clientSN != nullptr) {
        ret = clientSN->disconnect(0);
    }
    else {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }

    return (ret);
}

bool MQTTClient::isConnected(void) {
    bool rc;

    if (((client == nullptr) && (clientSN == nullptr)) ||
        ((client != nullptr) && (clientSN != nullptr))) {
        rc = false;
    }
    else if (client != nullptr) {
        rc = client->isConnected();
    }
    else {
        rc = clientSN->isConnected();
    }

    return (rc);
}

void MQTTClient::setDefaultMessageHandler(messageHandler mh) {
    if (client != nullptr) {
        client->setDefaultMessageHandler(mh);
    }
    else if (clientSN != nullptr) {
        client->setDefaultMessageHandler(mh);
    }
    else {
        // pass
    }
}

nsapi_error_t MQTTClient::setMessageHandler(const char* topicFilter, messageHandler mh) {
    nsapi_error_t ret;

    if (clientSN != nullptr) {
        ret = NSAPI_ERROR_UNSUPPORTED;
    }
    else if (client == nullptr) {
        ret = NSAPI_ERROR_NO_CONNECTION;
    }
    else {
        ret = client->setMessageHandler(topicFilter, mh);
    }

    return (ret);
}

void MQTTClient::init(Socket* sock) {
    socket   = sock;
    client   = nullptr;
    clientSN = nullptr;
}
