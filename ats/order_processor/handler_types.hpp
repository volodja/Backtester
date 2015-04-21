#ifndef HANDLER_TYPES_HPP
#define HANDLER_TYPES_HPP

#include <functional>
#include <ats/message/order_status_message.hpp>

namespace ats
{
	typedef std::function<void(const ats::order_status_message&)> order_status_handler;
	typedef std::function<void(const ats::position&)> position_change_handler;
}

#endif