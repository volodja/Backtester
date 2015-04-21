#ifndef EXECUTION_ENGINE_HPP
#define EXECUTION_ENGINE_HPP

#include <string>
#include <ats/event_handler/multievent_handler.hpp>
#include <ats/order/market_order.hpp>
#include <ats/order/limit_order.hpp>
#include <ats/order/stop_order.hpp>
#include <ats/handler_types.hpp>
#include <ats/types.hpp>

namespace ats
{
	class execution_engine : public ats::multievent_handler
	{
	public:
		execution_engine(const std::string& name, const ats::subscription& subscription)
			: name_(name), subscription_(subscription)
		{
			add_event_handler(&execution_engine::on_order_status_changed, this);
		}

		virtual ~execution_engine() { }

		virtual void subscribe(const ats::symbol_key&) = 0;

		// Basic order types that the engine must be able to handle
		virtual void send_order(const ats::market_order&) = 0;
		virtual void send_order(const ats::limit_order&) = 0;
		virtual void send_order(const ats::stop_order&) = 0;
		virtual void cancel_order(const ats::orderid_t& order_id) = 0;

		const std::string& name() const { return name_; }

		const ats::subscription& subscription() const { return subscription_; }

		void add_order_status_listener(const ats::order_status_handler& handler)
		{
			order_status_handler_ = handler;
		}

		void on_order_status_changed(const ats::order_status_message& msg)
		{
			if (order_status_handler_ != nullptr)
				order_status_handler_(msg);
		}

	private:
		ats::order_status_handler order_status_handler_;
		std::string name_;
		ats::subscription subscription_;
	};
}

#endif