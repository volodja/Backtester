#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <chrono>

namespace ats {
namespace date_time
{
	class stop_watch
	{
		std::chrono::system_clock::time_point start_t;
		std::chrono::system_clock::time_point end_t;
	public:
		stop_watch()
		{
			this->start();
			this->stop();
		}
		void start() { start_t = std::chrono::system_clock::now(); }
		void stop() { end_t = std::chrono::system_clock::now(); }
	
		std::chrono::duration<std::chrono::system_clock::rep, std::chrono::system_clock::period> elapsed()
		{
			return end_t - start_t;
		}

		long long milliseconds()
		{
			auto dif = std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t);
			return dif.count();
		}

		long long microseconds()
		{
			auto dif = std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t);
			return dif.count();
		}

		long long seconds()
		{
			auto dif = std::chrono::duration_cast<std::chrono::seconds>(end_t - start_t);
			return dif.count();
		}
	};
}
}

#endif