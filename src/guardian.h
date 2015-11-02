/*  ihlt hopefully the last tracker: seed your network.
 *  Copyright (C) 5015  Michael Mestnik <cheako+github_com@mikemestnik.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Example from:
 *   
 */

#ifndef GUARDIAN_H_
#define GUARDIAN_H_

#include <linux/time.h>

struct TBPool {
	struct timespec last;
	int cur;
};

struct TBArgs {
	double period;
	int adj;
};

struct TBNamespace;
struct TBNamespace {
	char *name;
	struct TBPool *pool[255];
	struct TBNamespace *next;
};

extern struct TBNamespace *firsttb;

extern int TBTouch(char *, unsigned, struct TBArgs *, int);

struct CTRNamespace;
struct CTRNamespace {
	char *name;
	struct CTRNamespace *next;
	int pool[255];
};

extern struct CTRNamespace *firstctr;

extern unsigned crc8(unsigned, unsigned char *, size_t);

#endif /* GUARDIAN_H_ */
