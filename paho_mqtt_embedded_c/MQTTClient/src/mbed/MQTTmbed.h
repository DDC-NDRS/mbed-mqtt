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
 *    Ian Craggs - change Timer member initialization to avoid copy constructor
 *******************************************************************************/

#if !defined(MQTT_MBED_H)
#define MQTT_MBED_H

#include "mbed.h"
#include <chrono>

class Countdown {
public :
    explicit Countdown() : t() {
        // pass
    }

    explicit Countdown(int ms) : t() {
        countdown_ms(ms);
    }


    bool expired(void) {
        unsigned long t_ms;

        t_ms = std::chrono::duration_cast<std::chrono::duration<unsigned long, milli>>(t.elapsed_time()).count();

        return (t_ms >= interval_end_ms);
    }

    void countdown_ms(unsigned long ms) {
        t.stop();
        interval_end_ms = ms;
        t.reset();
        t.start();
    }

    void countdown(int seconds) {
        countdown_ms((unsigned long)seconds * 1000L);
    }

    int left_ms(void) {
        unsigned long t_ms;

        t_ms = std::chrono::duration_cast<std::chrono::duration<unsigned long, milli>>(t.elapsed_time()).count();

        return (interval_end_ms - t_ms);
    }

private :
    Timer t;
    unsigned long interval_end_ms;
};

#endif
