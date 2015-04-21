#ifndef PORTFOLIO_BASE_HPP
#define PORTFOLIO_BASE_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <ats/order/order.hpp>
#include <ats/container/security_container.hpp>
#include <ats/container/order_container.hpp>
#include <ats/security/security_base.hpp>
#include <ats/position/position.hpp>

#include <ats/execution_engine/level2/level2_execution_engine.hpp>

#include <ats/report/report_engine.hpp>
//#include <ats/container/double_key_lookup.hpp>

namespace ats
{
	enum class execution_mode
	{
		BackTesting,
		WarmUp,
		RealTime
	};

	class portfolio_base : public ats::multievent_handler
	{
		using security_base_ptr = std::shared_ptr<ats::security_base>;
		using order_ptr = std::shared_ptr<ats::order>;
	public:
		portfolio_base() : log_("LOG.txt")
		{
			// register "global" event handlers (i.e., not having info about the symbol and exchange)
			this->add_event_handler(&portfolio_base::process_order_status_message, this);
			this->add_event_handler(&portfolio_base::process_message, this);
//			this->add_event_handler(&portfolio_base::on_order_book_changed, this);

			on_init();
		}

		virtual ~portfolio_base() {	}

		virtual void on_init() { }
		virtual void on_exit() { }

		// Time of the latest message
		const ats::timestamp_t& current_time() const { return time_; }

		void set_bar_parameters(const boost::posix_time::time_duration& bar_periodicity,
				size_t bars_to_store)
		{
			bar_periodicity_ = bar_periodicity;
			bars_to_store_ = bars_to_store;

			for (auto& sec : securities_)
				sec->set_bar_parameters(bar_periodicity, bars_to_store);
		}

	// Basic event handlers
//	private:
		// Update time whenever a new message is received
		void on_time_update(const ats::timestamp_t& time)
		{
			time_ = time;

/*			for (auto& sec: securities_)
				sec->process_time_update(time);*/
		}

		// Called once an order status change event is received from an exchange
		void process_order_status_message(const ats::order_status_message& msg);

		void process_message(const ats::level2_message_packet& msg)
		{
			on_time_update(msg.time);

			const ats::symbol_key* symbol = get_symbol_key(msg.symbol);
			if (symbol != nullptr)
				securities_[symbol->index]->process_message(msg);
		}

/*		void process_message(const ats::trade_message& msg)
		{
			on_time_update(msg.time);

			//
		}*/
	public:
		void send_order(const ats::order& order);

		void cancel_order(const ats::orderid_t& order_id)
		{
/*			auto it = orders_.get(order_id);
			if (it != orders_.cend())
			{
				ats::execution_engine* engine = execution_engines_[it->second->exchange];
				engine->cancel_order(order_id);
			}*/
			auto it = orders_.find(order_id);
			if (it != orders_.cend())
			{
				ats::execution_engine* engine = execution_engines_[it->second->exchange];
				engine->cancel_order(order_id);
			}
		}

		void cancel_pending_orders()
		{
/*			for (auto it = orders_.begin(); it != orders_.end(); ++it)
			{
				auto& order = it->second;
				if (order->is_pending())
				{
					std::cout << "Trying to cancel order id=" << it->second->id() << '\n';
					cancel_order(order->id());
				}
			}*/
			for (auto it = orders_.begin(); it != orders_.end();)
			{
				if (it->second->is_pending())
					cancel_order((it++)->second->id());
				else
					++it;
			}
		}

	public:
		void add_connection(ats::execution_engine* engine)
		{
			// subscribe securities that trade on the venue
			for (const auto& sec : securities_)
			{
				const auto& book = sec->order_book();
				if (book.get(engine->name()) != nullptr)
					engine->subscribe(sec->symbol());
			}

			engine->add_order_status_listener([=](const ats::order_status_message& msg) { process_order_status_message(msg); });

			if (engine->subscription() == ats::subscription::Level2)
			{
				ats::level2_execution_engine* l2_engine = static_cast<ats::level2_execution_engine*>(engine);
				l2_engine->add_order_book_changed_listener([=](const ats::level2_message_packet& msg) { process_message(msg); });
			}
			else if (engine->subscription() == ats::subscription::TimeAndSales)
			{
			}

			execution_engines_.insert(std::make_pair(engine->name(), engine));
		}

		std::ofstream& LOG() { return log_;	}

		void add_security(ats::security_base* sec)
		{
			const ats::symbol_key& symbol = sec->symbol();
			symbols_.push_back(symbol);
			symbol_keys_.insert(std::make_pair(symbol.name, symbol));
			positions_.push_back(ats::position(symbol));

			securities_.push_back(security_base_ptr(dynamic_cast<decltype(sec)>(sec)));
		}

		void add_security(const security_base_ptr& sec)
		{
			securities_.push_back(sec);

			const ats::symbol_key& symbol = sec->symbol();
			symbols_.push_back(symbol);
			symbol_keys_.insert(std::make_pair(symbol.name, symbol));
			positions_.push_back(ats::position(symbol));
		}

		void create_order_book(const std::string& symbol, const std::string& exchange, size_t book_depth = 10U)
		{
			const ats::symbol_key* key = get_symbol_key(symbol);
			if (key != nullptr)
				securities_[key->index]->create_order_book(exchange, book_depth);
		}

	public:
		ats::execution_engine* get_execution_engine(const std::string& engine_name)
		{
			auto find = execution_engines_.find(engine_name);
			return find != execution_engines_.end() ? find->second : nullptr;
		}

		const std::vector<ats::symbol_key>& get_symbols() const { return symbols_; }

		security_base_ptr& get_security(const ats::symbol_key& symbol)
		{
			return securities_[symbol.index];
		}

		template<typename SecurityT>
		std::shared_ptr<SecurityT> get_casted_security(const ats::symbol_key& symbol)
		{
			security_base_ptr& sec = securities_[symbol.index];
			return std::static_pointer_cast<SecurityT>(sec);
		}

		security_base_ptr* get_security(const std::string& symbol)
		{
			const ats::symbol_key* key = get_symbol_key(symbol);
			return key != nullptr ? &securities_[key->index] : nullptr;
		}

		std::shared_ptr<ats::order>* get_order(const ats::orderid_t& order_id)
		{
//			auto it = orders_.get(order_id);
			auto it = orders_.find(order_id);
			return it != orders_.end() ? &it->second : nullptr;
		}

		const ats::symbol_key* get_symbol_key(const std::string& symbol) const
		{
			auto it = symbol_keys_.find(symbol);
			return it != symbol_keys_.cend() ? &it->second : nullptr;
		}

		const ats::order_book& get_order_book(const ats::symbol_key& symbol) const
		{
//			return order_books_[symbol.index];

			return securities_[symbol.index]->order_book();
		}

		const ats::position& get_position(const ats::symbol_key& symbol) const
		{
			return positions_[symbol.index];
		}

		const ats::report_engine& get_report() const { return report_; }

		size_t get_next_order_id() const
		{
			static size_t id = 1;
			return id++;
		}


//	private:
		size_t get_next_symbol_key() const
		{
			static size_t index = 0;
			return index++;
		}

		void process_execution(const ats::symbol_key& symbol, ats::order_side side, long quantity,
				ats::price_t price, const ats::timestamp_t& time)
		{
			ats::position& pos = positions_[symbol.index];
			double previous_pnl = pos.realized_pnl();
			bool is_long = side == ats::order_side::Buy || side == ats::order_side::BuyCover;
			bool is_opposite_dir = pos.quantity() != 0 && is_long != pos.is_long();

			// Add execution to position
			pos.add_execution(side, quantity, price, time);

			if (is_opposite_dir)
			{
				ats::pnl_item item;
				item.time = time;
				item.profit = pos.realized_pnl() - previous_pnl;
				report_.add_pnl_item(item);
			}
		}

	private:
//		ats::security_container securities_;                              // securities to be traded
		std::vector<ats::symbol_key> symbols_;
		std::vector<security_base_ptr> securities_;
		std::unordered_map<std::string, ats::execution_engine*> execution_engines_;

		// To work with orders
//		ats::order_container orders_; // orders that have been submitted
		std::unordered_map<ats::orderid_t, order_ptr> orders_;
		ats::timestamp_t time_;       // time of the last message

		std::ofstream log_;

		std::vector<ats::position> positions_;
//		std::vector<ats::order_book> order_books_;
		ats::report_engine report_;

		boost::posix_time::time_duration bar_periodicity_;
		size_t bars_to_store_ = 0;

	protected:
		std::unordered_map<std::string, ats::symbol_key> symbol_keys_;
	};
}

#endif