
#include "rtgc.h"

#include "lauxlib.h"
#include "lgc.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <time.h>

static double tickGc = 0;          //gc时间片大小 毫秒
//static float tickCode = 0.8;      //代码时间片大小 毫秒
static int testTimeOutCount = 0;
static int bLog = 0;
static int msCpuTick = 2000000;   //每毫秒内cpu的计数
static int gcCpuTick = 0;

int runRtGc = 0;
FILE* pLogFile = NULL;


__u64 rdtsc()
{
    // __u64 ret;
    // __asm__ __volatile__("rdtsc" : "=A" (ret));
    // return ret;

    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );

    return (__u64)hi<<32|lo;
}


//取cpu速度
int get_cpu_speed() {
  int speed = 0;
  static const char speed_marker[] = "cpu MHz\t\t: ";
  char buf[1024];

  FILE* fp = fopen("/proc/cpuinfo", "r");
  if (fp == NULL)
    return 0;
  while (fgets(buf, sizeof(buf), fp)) {
    if (strncmp(buf, speed_marker, sizeof(speed_marker) - 1) == 0)
        speed = atoi(buf + sizeof(speed_marker) - 1);
  }

  fclose(fp);

  return speed;
}

int write_log (const char *format, ...) {
    if(pLogFile == NULL) return 0;
    va_list arg;
    int done;

    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    fprintf(pLogFile, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    done = vfprintf (pLogFile, format, arg);
    va_end (arg);

    fflush(pLogFile);
    return done;
}


//设置rtgc的参数
int set_rtgc(lua_State* L)
{
    luaL_checknumber(L, 1);
    luaL_checkinteger(L, 2);

    msCpuTick = get_cpu_speed() * 1000;
    if(msCpuTick<=0)
    {
        luaL_error(L, "Failed read cpu speed\n");
    }

    double oldGc = tickGc;
    int oldLog = bLog;
    int oldToutCount = testTimeOutCount;

    tickGc = lua_tonumber(L, 1);
    int option = lua_tointeger(L, 2);

    if(option == 1) //记录log日志
    {
        if(pLogFile==NULL)  pLogFile = fopen("rtlog.txt", "a");
        bLog = 1;
    }
    else
    {
        bLog = 0;
    }

    //结束
    if(tickGc == 0)
    {
        if(oldLog) write_log("cpu(%.1fG) gc(%.4fms) timeout count:%d\n", (float)(msCpuTick/1000000), oldGc, oldToutCount);
        testTimeOutCount = 0;
        lua_gc(L, LUA_GCCOLLECT);
        lua_gc(L, LUA_GCRESTART);
    }
    else
    {
        if(tickGc<0) tickGc = 0.1;
    }
    lua_gc(L, LUA_GCINC);
    lua_gc(L, LUA_GCSETPAUSE, 120);
    lua_gc(L, LUA_GCSETSTEPMUL, 400);

    //清理 并返回
    if(oldLog)
    {
        lua_pushinteger(L, oldToutCount);
    }
    else
    {
        lua_pushnumber(L, oldGc);
    }

    if(pLogFile!=NULL && !bLog)
    {
        fclose(pLogFile);
        pLogFile = NULL;
    }

    gcCpuTick = msCpuTick * tickGc;
    return 1;
}



// //执行一个时间片的rtgc
// __u64 rtgc_step(lua_State* L, int opCode)
// {
//     //printf("code:%d ", opCode);
//     runRtGc = 2;
//     //int total = lua_gc(L, LUA_GCCOUNT);
//     //printf("start memory: %dkb\n", total);
//     __u64 now = rdtsc();
//     __u64 tlimit = now + tickGc*msCpuTick;

//     global_State *g = G(L);

//     g->gcrunning = 1;  /* allow GC to run */
//     do
//     {
//         luaE_setdebt(g, 0);  /* do a basic step */
//         luaC_step(L);
//         now = rdtsc(); 
//     } while (now<tlimit && g->gcstate != GCSpause);
//     g->gcrunning = 0;

//     if(g->gcstate == GCSpause)
//     {
//         //printf("gc ok\n");
//         runRtGc = 0;
//     }

//     //超时多过一个gc时间片报错
//     if(now>tlimit && ((now-tlimit)>tickGc*msCpuTick))
//     {
//         testTimeOutCount += 1;
//         //luaL_error(L, "Real time gc failed, timeout=%dvs\n", (int)((now-tlimit)/(msCpuTick/1000)));
//         //printf("Real time gc failed, timeout=%dvs\n", (int)((now-tlimit)/(msCpuTick/1000)));
//         //if(pFile != NULL) write_log(pFile, "Real time gc failed, timeout=%dvs\n", (int)((now-tlimit)/(msCpuTick/1000)));
//     }

//     //total = total - lua_gc(L, LUA_GCCOUNT);
//     //if(total>1) printf("gc memory: %dkb\n", total);

//     //printf("end step count:%d\n", itemp);
//     if(runRtGc == 2) runRtGc = 1;
//     return now + tickCode*msCpuTick;
// }

int get_gctick()
{
    return gcCpuTick;
}

int is_rtgc()
{
    return (gcCpuTick>0);
}

void rtgc_atimeout()
{
    //if(bLog) write_log("skip rtgc atomic\n");
}


void rtgc_timeout(__u64 begint, __u64 endt)
{
    if(endt-begint > gcCpuTick/2)
        testTimeOutCount += 1;
    //if(bLog) write_log("rtgc timeout:%.4fms\n", (double)((double)(endt-begint)/msCpuTick));
}


//gc总过程
void gc_finish(__u64 begint, __u64 endt)
{
    double gct =  (double)((double)(endt-begint)/msCpuTick);
    if(gct<1) return;
    //if(bLog) write_log("gc finish:%.4fms\n",gct);
}