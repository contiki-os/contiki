/*
 * This file is part of HiKoB Openlab.
 *
 * HiKoB Openlab is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, version 3.
 *
 * HiKoB Openlab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HiKoB Openlab. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2011,2012 HiKoB.
 */

/**
 * \file
 *         Configuration for IoT-LAB A8-M3
 *
 * \author
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 *         Gaëtan Harter <gaetan.harter.at.inria.fr>
 *
 */

#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

/* specific defines */

#define LEDS_GREEN       1
#define LEDS_YELLOW      4
#define LEDS_RED         2
#define LEDS_BLUE        0
#define LEDS_CONF_ALL   (LEDS_GREEN | LEDS_YELLOW | LEDS_RED | LEDS_BLUE)


#ifndef SLIP_ARCH_CONF_BAUDRATE
#define SLIP_ARCH_CONF_BAUDRATE 500000
#endif

/* Sensors / actuators */
#define PLATFORM_HAS_LEDS    1
#define PLATFORM_HAS_GYROSCOPE 1
#define PLATFORM_HAS_ACCELEROMETER 1
#define PLATFORM_HAS_MAGNETOMETER 1

/* Openlab common definitions */

#include "../openlab/contiki-openlab-conf.h"

#endif /* __CONTIKI_CONF_H__ */
