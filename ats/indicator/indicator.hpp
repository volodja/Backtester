#pragma once

#include <ats/message/level2_message.hpp>

namespace ats
{
	template<typename ValueT = double>
	class indicator
	{
	public:
		virtual ~indicator() { }

		const ValueT& value() const { return value_; }
	protected:
		ValueT value_;
	};
}
