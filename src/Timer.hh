#ifndef Timer_h
#define Timer_h

#include <ostream>

struct Time {
	static const long sc_clk_tck;
	long utime, stime, clock;

	static Time now();
};
Time operator- (const Time &t1, const Time &t2);
std::ostream& operator<< (std::ostream &out, const Time &p);

class Timer {
	public:
		const Time start;
		Timer();
		Time elapsed();
};

#endif
