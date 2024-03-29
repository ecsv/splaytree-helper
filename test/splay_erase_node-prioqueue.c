// SPDX-License-Identifier: MIT
/* Minimal Splay-tree helper functions test
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../splaytree.h"
#include "common.h"
#include "common-prioqueue.h"

static uint16_t values[256];
static uint16_t inserted;

static uint16_t valuequeue[ARRAY_SIZE(values)];
static uint16_t queuelen;

static uint16_t valuequeue_getmin(void)
{
	size_t i;
	uint16_t t;
	uint16_t *min_pos = NULL;

	for (i = 0; i < queuelen; i++) {
		if (!min_pos || valuequeue[i] < *min_pos)
			min_pos = &valuequeue[i];
	}

	if (!min_pos)
		return 0;

	t = valuequeue[queuelen - 1];
	valuequeue[queuelen - 1] = *min_pos;
	*min_pos = t;

	return valuequeue[queuelen - 1];
}

int main(void)
{
	struct splay_prioqueue queue;
	size_t i;
	struct splayitem *item;
	uint16_t operation;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));
		inserted = 0;
		queuelen = 0;

		splay_prioqueue_init(&queue);
		while (inserted < ARRAY_SIZE(values) ||
		       queuelen != 0) {

			if (inserted == ARRAY_SIZE(values))
				operation = 0;
			else
				operation = get_unsigned16() % 2;

			if (operation == 1) {
				item = (struct splayitem *)malloc(sizeof(*item));
				assert(item);

				item->i = values[inserted];
				splay_prioqueue_insert_unbalanced(&queue, item);

				valuequeue[queuelen] = values[inserted];
				inserted++;
				queuelen++;
			} else {
				item = splay_prioqueue_pop_unbalanced(&queue);

				if (queuelen) {
					assert(item);
					assert(item->i == valuequeue_getmin());

					queuelen--;
					free(item);
				} else {
					assert(!item);
				}

			}
		}
		assert(splay_empty(&queue.root));
	}

	return 0;
}
