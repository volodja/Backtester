#ifndef HANDLER_TYPES_HPP 
#define HANDLER_TYPES_HPP

#include <functional>
#include <ats/message/level2_message.hpp>
#include <ats/message/trade_message.hpp>
#include <ats/message/order_status_message.hpp>
#include <ats/position/position.hpp>

namespace ats
{
	typedef std::function<void(const ats::order_status_message&)> order_status_handler;
	typedef std::function<void(const ats::position&)> position_change_handler;
	typedef std::function<void(const ats::level2_message_packet&)> order_book_changed_handler;
}

#endif
