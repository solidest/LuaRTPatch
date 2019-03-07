/*
** rtgc.h for RTLua
**
** Made by header_template
** Login   <solidest>
**
** Started on  undefined Mar 3 7:30:42 PM 2019 header_template
** Last update Wed Mar 5 1:51:28 AM 2019 header_template
*/

#ifndef RTGC_H_
# define RTGC_H_

#define RTGC_DEBUG

#ifndef RTGC_DEBUG
  #define GC_BEGIN() ((void)0)
  #define GC_END() ((void)0)
  #define RTGC_END(begint, endt) ((void)0)
  #define RTGC_ATIMEOUT()  ((void)0)
  #define RTGC_TIMEOUT(end0t, end1t) ((void)0)
#else
  #define GC_BEGIN() __u64 t0 = rdtsc()
  #define GC_END() __u64 t1 = rdtsc(); gc_finish(t0, t1)
  #define RTGC_TIMEOUT(end0t, end1t) rtgc_timeout(end0t, end1t)
  #define RTGC_ATIMEOUT()  rtgc_atimeout()
#endif

#include "lua.h"

typedef unsigned long long __u64;


//取cpu时间
__u64 rdtsc();

//设置rtgc的参数
int set_rtgc(lua_State* L);
int get_gctick();
int is_rtgc();

void rtgc_timeout(__u64 begint, __u64 endt);
void rtgc_atimeout();
void gc_finish(__u64 begint, __u64 endt);


//int write_log (const char *format, ...);

//执行一个时间片的rtgc
//__u64 rtgc_step(lua_State* L, int opCode);


#endif /* !RTGC_H_ */
