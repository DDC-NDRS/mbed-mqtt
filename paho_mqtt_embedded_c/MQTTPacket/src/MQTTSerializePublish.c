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
 *    Ian Craggs - fix for https://bugs.eclipse.org/bugs/show_bug.cgi?id=453144
 *******************************************************************************/

#include "MQTTPacket.h"
#include "StackTrace.h"

#include <string.h>


/**
  * Determines the length of the MQTT publish packet that would be produced using the supplied parameters
  * @param qos the MQTT QoS of the publish (packetid is omitted for QoS 0)
  * @param topicName the topic name to be used in the publish  
  * @param payloadlen the length of the payload to be sent
  * @return the length of buffer needed to contain the serialized version of the packet
  */
int MQTTSerialize_publishLength(int qos, MQTTString topicName, int payloadlen) {
    int len = 0;

    len += 2 + MQTTstrlen(topicName) + payloadlen;
    if (qos > 0) {
        len += 2;                           /* packetid */
    }

    return (len);
}


/**
  * Serializes the supplied publish data into the supplied buffer, ready for sending
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param dup integer - the MQTT dup flag
  * @param qos integer - the MQTT QoS value
  * @param retained integer - the MQTT retained flag
  * @param packetid integer - the MQTT packet identifier
  * @param topicName MQTTString - the MQTT topic in the publish
  * @param payload byte buffer - the MQTT publish payload
  * @param payloadlen integer - the length of the MQTT payload
  * @return the length of the serialized data.  <= 0 indicates error
  */
int MQTTSerialize_publish(unsigned char* buf, int buflen, unsigned char dup, int qos, unsigned char retained,
                          unsigned short packetid, MQTTString topicName, unsigned char* payload, int payloadlen) {
    unsigned char* ptr = buf;
    MQTTHeader header;
    int rem_len;
    int rc;

    FUNC_ENTRY;
    rem_len = MQTTSerialize_publishLength(qos, topicName, payloadlen);
    if (MQTTPacket_len(rem_len) > buflen) {
        rc = MQTTPACKET_BUFFER_TOO_SHORT;
        goto exit;
    }

	header.bits.type = PUBLISH;
	header.bits.dup = dup;
	header.bits.qos = qos;
	header.bits.retain = retained;
	writeChar(&ptr, header.byte); /* write header */

	ptr += MQTTPacket_encode(ptr, rem_len); /* write remaining length */;

	writeMQTTString(&ptr, topicName);

	if (qos > 0) {
		writeInt(&ptr, packetid);
	}

	memcpy(ptr, payload, payloadlen);

	rc = (ptr + payloadlen) - buf;

exit:
	FUNC_EXIT_RC(rc);
	return rc;
}



/**
  * Serializes the ack packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param type the MQTT packet type
  * @param dup the MQTT dup flag
  * @param packetid the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int MQTTSerialize_ack(unsigned char* buf, int buflen, unsigned char packettype, unsigned char dup,
                      unsigned short packetid) {
    MQTTHeader header;
    int rc;

    FUNC_ENTRY;
    if (buflen < 4) {
        rc = MQTTPACKET_BUFFER_TOO_SHORT;
        goto exit;
    }

    header.byte = 0;
    header.bits.type = packettype;
    header.bits.dup  = dup;
    header.bits.qos  = (packettype == PUBREL) ? 1 : 0;
    buf[0] = header.byte;                   /* write header */
    buf[1] = 2;                             /* write remaining length */
    utl_mqtt_write_int(buf[2], buf[3], packetid);
    rc = 4;

exit :
    FUNC_EXIT_RC(rc);

    return (rc);
}


/**
  * Serializes a puback packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param packetid integer - the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int MQTTSerialize_puback(unsigned char* buf, int buflen, unsigned short packetid) {
    return MQTTSerialize_ack(buf, buflen, PUBACK, 0, packetid);
}


/**
  * Serializes a pubrel packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param dup integer - the MQTT dup flag
  * @param packetid integer - the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int MQTTSerialize_pubrel(unsigned char* buf, int buflen, unsigned char dup, unsigned short packetid) {
    return MQTTSerialize_ack(buf, buflen, PUBREL, dup, packetid);
}


/**
  * Serializes a pubrel packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param packetid integer - the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int MQTTSerialize_pubcomp(unsigned char* buf, int buflen, unsigned short packetid) {
    return MQTTSerialize_ack(buf, buflen, PUBCOMP, 0, packetid);
}


