/**
 * librouteros - src/routeros_api.h
 * Copyright (C) 2009  Florian octo Forster
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the License is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Authors:
 *   Florian octo Forster <octo at verplant.org>
 **/

#ifndef ROUTEROS_API_H
#define ROUTEROS_API_H 1

#include <stdint.h>
#include <inttypes.h>

#include <routeros_version.h>

#define ROUTEROS_API_PORT "8728"

/*
 * C++ doesn't have _Bool. We can't simply "#define _Bool bool", because we
 * don't know if "bool" and "_Bool" are of the same size. If they are not, this
 * would result in ABI incompatible code.
 *
 * So we're doing a best-effort solution here: If we're compiled with the GNU
 * C++ compiler, g++, we include <stdbool.h>. The GCC will, as a GNU extension,
 * define _Bool for C++. Since it's the compiler doing the definition, it's
 * kind of save to assume that it will be done in an ABI compatible manner.
 *
 * If this results in any problems for you, define "ROS_HAVE_CPP_BOOL" to true
 * to have this magic disabled. You will then have to define _Bool yourself.
 *
 * TODO: Write a test program for the configure sript to figure out the size of
 *   _Bool. Make this size available via <routeros_versioin.h> and define _Bool
 *   to short, long, ... here.
 */
#ifdef __cplusplus
# if !defined (ROS_HAVE_CPP_BOOL) || !ROS_HAVE_CPP_BOOL
#  ifdef __GNUC__
#   include <stdbool.h>
#  endif /* __GNUC__ */
# endif /* !defined (ROS_HAVE_CPP_BOOL) || !ROS_HAVE_CPP_BOOL */

extern "C" {
#endif

struct ros_connection_s;
typedef struct ros_connection_s ros_connection_t;

struct ros_reply_s;
typedef struct ros_reply_s ros_reply_t;

typedef int (*ros_reply_handler_t) (ros_connection_t *c, const ros_reply_t *r,
		void *user_data);

/*
 * Connection handling
 */
ros_connection_t *ros_connect (const char *node, const char *service,
		const char *username, const char *password);
int ros_disconnect (ros_connection_t *con);

/* 
 * Command execution
 */
int ros_query (ros_connection_t *c,
		const char *command,
		size_t args_num, const char * const *args,
		ros_reply_handler_t handler, void *user_data);

/* 
 * Reply handling
 */
const ros_reply_t *ros_reply_next (const ros_reply_t *r);
int ros_reply_num (const ros_reply_t *r);

const char *ros_reply_status (const ros_reply_t *r);

/* Receiving reply parameters */
const char *ros_reply_param_key_by_index (const ros_reply_t *r,
		unsigned int index);
const char *ros_reply_param_val_by_index (const ros_reply_t *r,
		unsigned int index);
const char *ros_reply_param_val_by_key (const ros_reply_t *r, const char *key);

/* High-level function for accessing /interface {{{ */
struct ros_interface_s;
typedef struct ros_interface_s ros_interface_t;
struct ros_interface_s
{
	/* Name of the interface */
	const char *name;
	const char *type;
	const char *comment;

	/* Packet, octet and error counters. */
	uint64_t rx_packets;
	uint64_t tx_packets;
	uint64_t rx_bytes;
	uint64_t tx_bytes;
	uint64_t rx_errors;
	uint64_t tx_errors;
	uint64_t rx_drops;
	uint64_t tx_drops;

	/* Maximum transfer unit */
	unsigned int mtu;
	unsigned int l2mtu;

	/* Interface flags */
	_Bool dynamic;
	_Bool running;
	_Bool enabled;

	/* Next interface */
	const ros_interface_t *next;
};

/* Callback function */
typedef int (*ros_interface_handler_t) (ros_connection_t *c,
		const ros_interface_t *i, void *user_data);

int ros_interface (ros_connection_t *c,
		ros_interface_handler_t handler, void *user_data);
/* }}} /interface */

/* High-level function for accessing /interface/wireless/registration-table {{{ */
struct ros_registration_table_s;
typedef struct ros_registration_table_s ros_registration_table_t;
struct ros_registration_table_s
{
	/* Name of the interface */
	const char *interface;
	/* Name of the remote radio */
	const char *radio_name;

	/* ap is set to true, if the REMOTE radio is an access point. */
	_Bool ap;
	_Bool wds;

	/* Receive and transmit rate in MBit/s */
	double rx_rate;
	double tx_rate;

	/* Packet, octet and frame counters. */
	uint64_t rx_packets;
	uint64_t tx_packets;
	uint64_t rx_bytes;
	uint64_t tx_bytes;
	uint64_t rx_frames;
	uint64_t tx_frames;
	uint64_t rx_frame_bytes;
	uint64_t tx_frame_bytes;
	uint64_t rx_hw_frames;
	uint64_t tx_hw_frames;
	uint64_t rx_hw_frame_bytes;
	uint64_t tx_hw_frame_bytes;

	/* Signal quality information (in dBm) */
	double rx_signal_strength;
	double tx_signal_strength;
	double signal_to_noise;

	/* Overall connection quality (in percent) */
	double rx_ccq;
	double tx_ccq;

	/* Next interface */
	const ros_registration_table_t *next;
};

/* Callback function */
typedef int (*ros_registration_table_handler_t) (ros_connection_t *c,
		const ros_registration_table_t *r, void *user_data);

int ros_registration_table (ros_connection_t *c,
		ros_registration_table_handler_t handler, void *user_data);
/* }}} /interface/wireless/registration-table */

#ifdef __cplusplus
}
#endif

#endif /* ROUTEROS_API_H */

/* vim: set ts=2 sw=2 noet fdm=marker : */
