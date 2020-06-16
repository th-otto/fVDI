/*
 * VDI profiler - counter/timer reset
 *
 * Copyright 1997 & 2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
#endif

#define ABS(x) (((x) >= 0) ? x : -x)

#ifndef SuperToUser
# define SuperToUser(ptr) Super(ptr)
#endif

#define VERSION "v0.60"
#define SUPPORTED_TABLE 0x10
#define MAX_COUNT 192

struct Profile {
	long count;
	long time_interrupt;
	long time_counter;
	long dummy;
} ;

struct Info {
	unsigned char version;
	unsigned char table_version;
	int flags;
	long start_time;
	struct Profile *prof;
	int table_size;
	char fives;
	char elevenths;
	void (*reset)(void);
	long (*unlink)(void);
	long (*relink)(void);
} ;
	
long get_cookie(const char *cname)
{
   long oldstack, *ptr, value, name;

   name = 0;
   while(*cname)
      name = (name << 8) | (unsigned char)*cname++;

   oldstack = (long)Super(0L);
   ptr = (long *)*(long *)0x5a0;

   if (ptr != NULL) {
      while ((*ptr != 0) && (*ptr != name))
         ptr += 2;
      if (*ptr == name)      /* The if statement is new */
         value = ptr[1];
      else
         value = -1;
   } else
      value = -1;             /*  This used to be 0 */

   SuperToUser((void *)oldstack);
   return value;
}

int main(void)
{
	long tmp;
	struct Info *info;
	
	if ((tmp = get_cookie("VDIp")) == -1)
		return 1;
	
	info = (struct Info *)tmp;

	info->reset();

	return 0;
}
