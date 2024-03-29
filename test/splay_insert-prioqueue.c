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

int main(void)
{
	struct splay_prioqueue queue;
	size_t i, j;
	struct splayitem *item;

	for (i = 0; i < 256; i++) {
		random_shuffle_array(values, (uint16_t)ARRAY_SIZE(values));

		splay_prioqueue_init(&queue);
		for (j = 0; j < ARRAY_SIZE(values); j++) {
			item = (struct splayitem *)malloc(sizeof(*item));
			assert(item);

			item->i = values[j];
			splay_prioqueue_insert_balanced(&queue, item);
		}

		for (j = 0; j < ARRAY_SIZE(values); j++) {
			item = splay_prioqueue_pop_balanced(&queue);
			assert(item);
			assert(item->i == j);

			free(item);
		}
		assert(splay_empty(&queue.root));
	}

	return 0;
}
