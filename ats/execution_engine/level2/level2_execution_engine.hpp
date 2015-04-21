#ifndef LEVEL2_EXECUTION_ENGINE_HPP
#define LEVEL2_EXECUTION_ENGINE_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <utility>
#include <typeindex>
#include <ats/execution_engine/execution_engine.hpp>
#include <ats/order_book/exchange_order_book.hpp>
//#include <ats/order_book/simulation/fifo_exchange_order_book.hpp>
#include <ats/order_book/simulation/fifo_exchange_order_book.hpp>
#include <ats/order_book/simulation/sim_book.hpp>
#include <ats/handler_types.hpp>
#include <ats/types.hpp>

namespace ats
{
	class level2_execution_engine : public ats::execution_engine
	{
	public:
		level2_execution_engine(const std::string& name, size_t book_depth = 10U)
			: ats::execution_engine(name, ats::subscription::Level2), book_depth_(book_depth)
		{
			add_event_handler(&level2_execution_engine::on_order_book_changed, this);
		}

		void add_order_book_changed_listener(const ats::order_book_changed_handler& handler)
		{
			order_book_changed_handler_ = handler;
		}

		void subscribe(const ats::symbol_key& symbol)
		{
			auto it = sim_books_.find(symbol.to_string());
			if (it == sim_books_.cend())
			{
				ats::sim::fifo_exchange_order_book sim_book(symbol, name(), book_depth_);
				sim_book.add_order_status_listener(std::bind(&ats::level2_execution_engine::on_order_status_changed, this, std::placeholders::_1));
				sim_books_.insert(std::make_pair(symbol.to_string(), std::move(sim_book)));
			}
			else
			{
				std::string text = "level2_execution_engine: Symbol '" + symbol.to_string() + "' already exists";
				throw std::invalid_argument(text);
			}
		}

		void on_order_book_changed(const ats::level2_message_packet& msg)
		{
			time_ = msg.time;

			auto book_it = sim_books_.find(msg.symbol);
			if (book_it == sim_books_.cend()) return;

			for (const auto& m : msg.messages)
				book_it->second.update(m);

			// Check if stop orders must be executed
			const ats::exchange_order_book& book = book_it->second.get_order_book();
			if (!stops_buy_.empty() && !book.asks().empty())
			{
				ats::price_t ask = book.best_ask()->price;
				for (auto it = stops_buy_.begin(); it != stops_buy_.end() && it->second.price() <= ask;)
				{
					const ats::stop_order& order = it->second;
					on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), ask, order.quantity()));
					stops_buy_.erase(it++);
				}
			}
			else if (!stops_sell_.empty() && !book.bids().empty())
			{
				ats::price_t bid = book.best_bid()->price;
				for (auto it = stops_sell_.begin(); it != stops_sell_.end() && it->second.price() > bid;)
				{
					const ats::stop_order& order = it->second;
					on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), bid, order.quantity()));
					stops_sell_.erase(it++);
				}
			}

			if (order_book_changed_handler_ != nullptr)
				order_book_changed_handler_(msg);
		}

		const ats::timestamp_t& current_time() const { return time_; }

		virtual void send_order(const ats::market_order& order) override;
		virtual void send_order(const ats::limit_order& order) override;
		virtual void send_order(const ats::stop_order& order) override;

		void cancel_order(const ats::limit_order& order);

		virtual void cancel_order(const ats::orderid_t& order_id) override
		{
			auto it = orders_.find(order_id);
			if (it != orders_.cend())
			{
				auto& order = it->second;
				std::type_index order_type(typeid(*order.get()));
				if (order_type == typeid(ats::limit_order))
				{
					auto book_it = sim_books_.find(order->symbol());
					book_it->second.cancel_order(order_id, current_time());
				}
				else if (order_type == typeid(ats::stop_order))
				{
					ats::price_t price = std::static_pointer_cast<ats::stop_order>(order)->price();
					if (order->side() == ats::order_side::Buy || order->side() == ats::order_side::BuyCover)
					{
						auto range = stops_buy_.equal_range(price);
						stops_buy_.erase(range.first, range.second);
					}
					else
					{
						auto range = stops_sell_.equal_range(price);
						stops_sell_.erase(range.first, range.second);
					}
				}

				orders_.erase(it);
				// For a limit order, on_order_status_changed will be called from inside the fifo_exchange_order_book
				if (order_type == typeid(ats::stop_order))
					on_order_status_changed(ats::order_status_cancelled_message(order_id, current_time()));
			}
			else
				std::cout << "ERROR (level2_execution_engine): Cannot cancel order id=" << order_id << '\n';
		}

	private:
		template<typename OrderT>
		void add_order(const OrderT& order)
		{
			orders_.insert(std::make_pair(order.id(), std::make_shared<OrderT>(order)));
		}

	private:
		size_t book_depth_;
		std::unordered_map<std::string, ats::sim::fifo_exchange_order_book> sim_books_;
		std::unordered_map<ats::orderid_t, std::shared_ptr<ats::order>> orders_;
		ats::timestamp_t time_;
		ats::order_book_changed_handler order_book_changed_handler_ = nullptr;

		std::multimap<ats::price_t, ats::stop_order, std::less<ats::price_t>> stops_buy_;
		std::multimap<ats::price_t, ats::stop_order, std::greater<ats::price_t>> stops_sell_;
	};
}

#endif