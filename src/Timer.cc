#include "Timer.hh"

#include <iostream>
#include <sys/times.h>
#include <unistd.h>

// ---------------------------------------------------------------------
//                         class Time
// ---------------------------------------------------------------------
const long Time::sc_clk_tck = sysconf(_SC_CLK_TCK);

Time Time::now()
{
	struct tms tms;
	clock_t clock = times(&tms);

	Time p;
	p.clock = clock;
	p.utime = tms.tms_utime;
	p.stime = tms.tms_stime;
	return p;
}

Time operator- (const Time &t1, const Time &t2)
{
	Time t;
	t.utime = t1.utime - t2.utime;
	t.stime = t1.stime - t2.stime;
	t.clock = t1.clock - t2.clock;
	return t;
}

std::ostream& operator<< (std::ostream &out, const Time &p)
{
	out << double(p.utime)/Time::sc_clk_tck << ' '
	    << double(p.stime)/Time::sc_clk_tck << ' '
	    << double(p.clock)/Time::sc_clk_tck;
	return out;
}

// ---------------------------------------------------------------------
//                            class Timer
// ---------------------------------------------------------------------
Timer::Timer() : start(Time::now()) {}

Time Timer::elapsed()
{
	return Time::now() - start;
}
