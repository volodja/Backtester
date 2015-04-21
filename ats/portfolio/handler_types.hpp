#ifndef HANDLER_TYPES_HPP
#define HANDLER_TYPES_HPP

#include <functional>
#include <ats/message/level2_message.hpp>
#include <ats/message/trade_message.hpp>
#include <ats/message/order_status_message.hpp>

namespace ats
{
	typedef std::function<void(const ats::order_status_message&)> on_order_status_changed_handler;
	typedef std::function<void(const ats::level2_message_packet&)> on_order_book_changed_handler;
	typedef std::function<void(const ats::trade_message&)> on_trade_handler;
}

#endif