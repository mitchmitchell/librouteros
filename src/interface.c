/**
 * librouteros - src/interface.c
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

#ifndef _ISOC99_SOURCE
# define _ISOC99_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
#endif

#include "config.h"

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include "routeros_api.h"

/*
 * Private data types
 */
struct rt_internal_data_s
{
	ros_interface_handler handler;
	void *user_data;
};
typedef struct rt_internal_data_s rt_internal_data_t;

/*
 * Private functions
 */
static unsigned int sstrtoui (const char *str) /* {{{ */
{
	unsigned int ret;
	char *endptr;

	if (str == NULL)
		return (0);

	errno = 0;
	endptr = NULL;
	ret = (unsigned int) strtoul (str, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
		return (0);

	return (ret);
} /* }}} unsigned int sstrtoui */

static _Bool sstrtob (const char *str) /* {{{ */
{
	if (str == NULL)
		return (false);

	if (strcasecmp ("true", str) == 0)
		return (true);
	return (false);
} /* }}} _Bool sstrtob */

static int string_to_rx_tx_counters (const char *str, /* {{{ */
		uint64_t *rx, uint64_t *tx)
{
	const char *ptr;
	char *endptr;

	if ((rx == NULL) || (tx == NULL))
		return (EINVAL);

	*rx = 0;
	*tx = 0;

	if (str == NULL)
		return (EINVAL);

	ptr = str;
	errno = 0;
	endptr = NULL;
	*rx = (uint64_t) strtoull (ptr, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
		return (EIO);

	assert (endptr != NULL);
	if (*endptr != '/')
		return (EIO);

	ptr = endptr + 1;
	errno = 0;
	endptr = NULL;
	*tx = (uint64_t) strtoull (ptr, &endptr, /* base = */ 10);
	if ((endptr == str) || (errno != 0))
		return (EIO);

	return (0);
} /* }}} int string_to_rx_tx_counters */

static ros_interface_t *rt_reply_to_interface (const ros_reply_t *r) /* {{{ */
{
	ros_interface_t *ret;

	if (r == NULL)
		return (NULL);

	if (strcmp ("re", ros_reply_status (r)) != 0)
		return (rt_reply_to_interface (ros_reply_next (r)));

	ret = malloc (sizeof (*ret));
	if (ret == NULL)
		return (NULL);
	memset (ret, 0, sizeof (*ret));

	ret->name = ros_reply_param_val_by_key (r, "name");
	ret->type = ros_reply_param_val_by_key (r, "type");
	ret->comment = ros_reply_param_val_by_key (r, "comment");

	string_to_rx_tx_counters (ros_reply_param_val_by_key (r, "packets"),
			&ret->rx_packets, &ret->tx_packets);
	string_to_rx_tx_counters (ros_reply_param_val_by_key (r, "bytes"),
			&ret->rx_bytes, &ret->tx_bytes);
	string_to_rx_tx_counters (ros_reply_param_val_by_key (r, "errors"),
			&ret->rx_errors, &ret->tx_errors);
	string_to_rx_tx_counters (ros_reply_param_val_by_key (r, "drops"),
			&ret->rx_drops, &ret->tx_drops);

	ret->mtu = sstrtoui (ros_reply_param_val_by_key (r, "mtu"));
	ret->l2mtu = sstrtoui (ros_reply_param_val_by_key (r, "l2mtu"));

	ret->dynamic = sstrtob (ros_reply_param_val_by_key (r, "dynamic"));
	ret->running = sstrtob (ros_reply_param_val_by_key (r, "running"));
	ret->enabled = !sstrtob (ros_reply_param_val_by_key (r, "disabled"));

	ret->next = rt_reply_to_interface (ros_reply_next (r));

	return (ret);
} /* }}} ros_interface_t *rt_reply_to_interface */

static void if_interface_free (ros_interface_t *r) /* {{{ */
{
	ros_interface_t *next;

	while (r != NULL)
	{
		next = r->next;
		free (r);
		r = next;
	}
} /* }}} void if_interface_free */

static int if_internal_handler (ros_connection_t *c, /* {{{ */
		const ros_reply_t *r, void *user_data)
{
	ros_interface_t *if_data;
	rt_internal_data_t *internal_data;
	int status;

	if_data = rt_reply_to_interface (r);
	if (if_data == NULL)
		return (errno);

	internal_data = user_data;

	status = internal_data->handler (c, if_data, internal_data->user_data);

	if_interface_free (if_data);

	return (status);
} /* }}} int if_internal_handler */

/*
 * Public functions
 */
int ros_interface (ros_connection_t *c, /* {{{ */
		ros_interface_handler handler, void *user_data)
{
	rt_internal_data_t data;

	if ((c == NULL) || (handler == NULL))
		return (EINVAL);

	data.handler = handler;
	data.user_data = user_data;

	return (ros_query (c, "/interface/print",
				/* args_num = */ 0, /* args = */ NULL,
				if_internal_handler, &data));
} /* }}} int ros_interface */

/* vim: set ts=2 sw=2 noet fdm=marker : */
