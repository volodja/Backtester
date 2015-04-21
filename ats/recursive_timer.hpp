#ifndef RECURSIVE_TIMER_HPP
#define RECURSIVE_TIMER_HPP

#include <functional>
#include <list>

namespace ats
{
	// Timer that is invoked every specified units of time (say, every second)
	class recursive_timer
	{
	public:
		typedef std::function<void(const ats::timestamp_t&)> time_listener;

		recursive_timer(const boost::posix_time::time_duration& period = boost::posix_time::seconds(1))
			: period_(period) { }

		void init(const boost::posix_time::time_duration& period) { period_ = period; }

		void update(const ats::timestamp_t& time)
		{
			if (end_time_.is_not_a_date_time() || time.date() != end_time_.date())
			{
				end_time_ = time.date();
				while (end_time_ + period_ < time)
					end_time_ += period_;

				invoke_listeners(end_time_);
			}
			else if (time - end_time_ >= period_)
			{
				end_time_ += period_;
				for (; end_time_ + period_ < time; end_time_ += period_)
					invoke_listeners(end_time_);

				invoke_listeners(end_time_);
			}
		}

		void add_time_listener(const time_listener& listener) { time_listeners_.push_back(listener); }

	private:
		void invoke_listeners(const ats::timestamp_t& time)
		{
			for (const auto& l: time_listeners_)
				l(time);
		}

	private:
		boost::posix_time::time_duration period_;
		ats::timestamp_t end_time_;
		std::list<time_listener> time_listeners_;
	};
}

#endif
