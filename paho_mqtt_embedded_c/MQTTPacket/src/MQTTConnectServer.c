/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "StackTrace.h"
#include "MQTTPacket.h"
#include <string.h>

#define min(a, b) ((a < b) ? a : b)


/**
  * Validates MQTT protocol name and version combinations
  * @param protocol the MQTT protocol name as an MQTTString
  * @param version the MQTT protocol version number, as in the connect packet
  * @return correct MQTT combination?  1 is true, 0 is false
  */
int MQTTPacket_checkVersion(MQTTString* protocol, int version) {
    int rc;

    if ((version == 3) && memcmp(protocol->lenstring.data, "MQIsdp", min(6, protocol->lenstring.len)) == 0) {
        rc = 1;
    }
    else if ((version == 4) && memcmp(protocol->lenstring.data, "MQTT", min(4, protocol->lenstring.len)) == 0) {
        rc = 1;
    }
    else {
        rc = 0;
    }

    return (rc);
}


/**
  * Deserializes the supplied (wire) buffer into connect data structure
  * @param data the connect data structure to be filled out
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
 */
int MQTTDeserialize_connect(MQTTPacket_connectData* data, unsigned char* buf, int len) {
    MQTTHeader header;
    MQTTConnectFlags flags;
    unsigned char* curdata = buf;
    unsigned char* enddata = &buf[len];
    int rc;
    MQTTString Protocol;
    int version;
    int mylen;

    FUNC_ENTRY;
    rc = 0;
    header.byte = readChar(&curdata);
    if (header.bits.type != CONNECT) {
        goto exit;
    }

    curdata += MQTTPacket_decodeBuf(curdata, &mylen); /* read remaining length */

    if (!readMQTTLenString(&Protocol, &curdata, enddata) ||
        ((enddata - curdata) < 0)) {        /* do we have enough data to read the protocol version byte? */
        goto exit;
    }

    version = (int)readChar(&curdata); /* Protocol version */
    /* If we don't recognize the protocol version, we don't parse the connect packet on the
     * basis that we don't know what the format will be.
     */
    if (MQTTPacket_checkVersion(&Protocol, version) == 1) {
        flags.all = readChar(&curdata);
        data->cleansession = flags.bits.cleansession;
        data->keepAliveInterval = readInt(&curdata);
        if (!readMQTTLenString(&data->clientID, &curdata, enddata)) {
            goto exit;
        }

        data->willFlag = flags.bits.will;
        if (flags.bits.will) {
            data->will.qos = flags.bits.willQoS;
            data->will.retained = flags.bits.willRetain;
            if (!readMQTTLenString(&data->will.topicName, &curdata, enddata) ||
                !readMQTTLenString(&data->will.message, &curdata, enddata)) {
                goto exit;
            }
        }

        if (flags.bits.username) {
            if (((enddata - curdata) < 3) || !readMQTTLenString(&data->username, &curdata, enddata)) {
                goto exit;                  /* username flag set, but no username supplied - invalid */
            }

            if (flags.bits.password &&
                (((enddata - curdata) < 3) || !readMQTTLenString(&data->password, &curdata, enddata))) {
                goto exit;                  /* password flag set, but no password supplied - invalid */
            }
        }
        else if (flags.bits.password) {
            goto exit;                      /* password flag set without username - invalid */
        }

        rc = 1;
    }

exit :
    FUNC_EXIT_RC(rc);

    return (rc);
}


/**
  * Serializes the connack packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param connack_rc the integer connack return code to be used 
  * @param sessionPresent the MQTT 3.1.1 sessionPresent flag
  * @return serialized length, or error if 0
 */
int MQTTSerialize_connack(unsigned char* buf, int buflen, unsigned char connack_rc, unsigned char sessionPresent) {
    int rc;

    FUNC_ENTRY;
    if (buflen < 4) {
        rc = MQTTPACKET_BUFFER_TOO_SHORT;
        goto exit;
    }

    buf[0] = utl_mqtt_pkg_type(CONNACK);    /* write header */
    buf[1] = 2;                             /* write remaining length */
    buf[2] = (sessionPresent & 0x01);
    buf[3] = connack_rc;

    rc = 4;

exit :
    FUNC_EXIT_RC(rc);

    return (rc);
}

