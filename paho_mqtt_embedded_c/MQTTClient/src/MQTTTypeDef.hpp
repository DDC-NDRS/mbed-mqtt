/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
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
 *    Ian Craggs - fix for bug 458512 - QoS 2 messages
 *    Ian Craggs - fix for bug 460389 - send loop uses wrong length
 *    Ian Craggs - fix for bug 464169 - clearing subscriptions
 *    Ian Craggs - fix for bug 464551 - enums and ints can be different size
 *    Mark Sonnentag - fix for bug 475204 - inefficient instantiation of Timer
 *    Ian Craggs - fix for bug 475749 - packetid modified twice
 *    Ian Craggs - add ability to set message handler separately #6
 *******************************************************************************/

#if !defined(MQTTTYPEDEF_H)
#define MQTTTYPEDEF_H

#include "MQTTPacket.h"
#include <string.h>

namespace MQTT {

enum QoS {
    QOS0,
    QOS1,
    QOS2
};

// all failure return codes must be negative
enum returnCode {
    BUFFER_OVERFLOW = -2,
    FAILURE = -1,
    SUCCESS = 0
};


struct Message {
    enum QoS qos;
    bool retained;
    bool dup;
    unsigned short id;
    void* payload;
    size_t payloadlen;
};

struct MessageAck {
    unsigned char  type;
    unsigned char  dup;
    unsigned short id;
};

struct MessageData {
    MessageData(MQTTString& aTopicName, struct Message& aMessage) :
        message(aMessage), topicName(aTopicName) {
    }

    struct Message& message;
    MQTTString& topicName;
};


struct connackData {
    int rc;
    bool sessionPresent;
};


struct subackData {
    int grantedQoS;
};


class PacketId {
public :
    PacketId() : next(0) {
        // pass
    }

    int getNext() {
        if (next == MAX_PACKET_ID) {
            next = 1;
        }
        else {
            next = next + 1;
        }

        return (next);
    }

private :
    static const int MAX_PACKET_ID = 65535;
    int next;
};

}

#endif // MQTTTYPEDEF_H
