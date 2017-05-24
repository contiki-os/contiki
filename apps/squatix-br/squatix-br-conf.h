/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
 * All rights reserved.
 */

/**
 * \file
 *         SQUATIX configuration
 */

#ifndef __SQUATIX_CONF_H__
#define __SQUATIX_CONF_H__

#ifdef SQUATIX_CONF_RULES
#define SQUATIX_RULES SQUATIX_CONF_RULES
#else /* SQUATIX_CONF_RULES */
/* A default configuration with:
 * - a sender-based slotframe for EB transmission
 * - a sender-based or receiver-based slotframe for unicast to RPL parents and children
 * - a common shared slotframe for any other traffic (mostly broadcast)
 *  */
#define SQUATIX_RULES {&one_slotframe, &data_slotframe}
#endif /* SQUATIX_CONF_RULES */


///////// Using only one Slotframe, no need to declare several slotframes//////

/* Length of the various slotframes. Tune to balance network capacity,
 * contention, energy, latency. */

/* Only use one slotframe, comment other slotframes structure*/
#ifdef SQUATIX_CONF_ONE_SLOTFRAME_PERIOD
#define SQUATIX_ONE_SLOTFRAME_PERIOD                  SQUATIX_CONF_ONE_SLOTFRAME_PERIOD
#else /* SQUATIX_CONF_ONE_SLOTFRAME_PERIOD */
#define SQUATIX_ONE_SLOTFRAME_PERIOD                  5
#endif /* SQUATIX_ONE_SLOTFRAME_PERIOD */

#ifdef SQUATIX_CONF_DATA_SLOTFRAME_PERIOD
#define SQUATIX_DATA_SLOTFRAME_PERIOD                  SQUATIX_CONF_DATA_SLOTFRAME_PERIOD
#else /* SQUATIX_CONF_DATA_SLOTFRAME_PERIOD */
#define SQUATIX_DATA_SLOTFRAME_PERIOD                  5
#endif /* SQUATIX_DATA_SLOTFRAME_PERIOD */

/* Is the per-neighbor unicast slotframe sender-based (if not, it is receiver-based).
 * Note: sender-based works only with RPL storing mode as it relies on DAO and
 * routing entries to keep track of children and parents. */
#ifdef SQUATIX_CONF_DATA_SENDER_BASED
#define SQUATIX_DATA_SENDER_BASED            SQUATIX_CONF_DATA_SENDER_BASED
#else /* SQUATIX_CONF_DATA_SENDER_BASED */
#define SQUATIX_DATA_SENDER_BASED            0
#endif /* SQUATIX_CONF_DATA_SENDER_BASED */

///////// End of Using only one Slotframe, no need to declare several slotframes//////


// Comment the hash function use to assign cell to node ID//
/* The hash function used to assign timeslot to a given node (based on its link-layer address) */
#ifdef SQUATIX_CONF_LINKADDR_HASH
#define SQUATIX_LINKADDR_HASH                   SQUATIX_CONF_LINKADDR_HASH
#else /* SQUATIX_CONF_LINKADDR_HASH */
#define SQUATIX_LINKADDR_HASH(addr)             ((addr != NULL) ? (addr)->u8[LINKADDR_SIZE - 1] : -1)
#endif /* SQUATIX_CONF_LINKADDR_HASH */

/* The maximum hash */
#ifdef SQUATIX_CONF_MAX_HASH
#define SQUATIX_MAX_HASH                        SQUATIX_CONF_MAX_HASH
#else /* SQUATIX_CONF_MAX_HASH */
#define SQUATIX_MAX_HASH                        0x7fff
#endif /* SQUATIX_CONF_MAX_HASH */

/* Is the "hash" function collision-free? (e.g. it maps to unique node-ids) */
#ifdef SQUATIX_CONF_COLLISION_FREE_HASH
#define SQUATIX_COLLISION_FREE_HASH             SQUATIX_CONF_COLLISION_FREE_HASH
#else /* SQUATIX_CONF_COLLISION_FREE_HASH */
#define SQUATIX_COLLISION_FREE_HASH             0 /* Set to 1 if SQUATIX_LINKADDR_HASH returns unique hashes */
#endif /* SQUATIX_CONF_COLLISION_FREE_HASH */

// End of Comment the hash function use to assign cell to node ID//

#endif /* __SQUATIX_CONF_H__ */
