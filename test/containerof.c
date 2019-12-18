// SPDX-License-Identifier: MIT
/* Minimal Splay-tree helper functions test
 *
 * SPDX-FileCopyrightText: 2012-2019, Sven Eckelmann <sven@narfation.org>
 */

#include <assert.h>

#include "../splaytree.h"

struct teststruct {
	int a;
	int b;
};

int main(void)
{
	struct teststruct item;

	assert(&item == container_of(&item.b, struct teststruct, b));

	return 0;
}
