#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <functional>
#include <list>
#include <utility>

namespace ats
{
	// Necessary for the specialization below
	template<typename T>
	class event_handler;

	// Event handler class
	// Represents a container of connections with the same function signature
	template<typename ReturnT, typename... Args>
	class event_handler<ReturnT(Args...)>
	{
	public:
		typedef typename std::function<ReturnT(Args...)> connection_type;

		// Add a connection
		event_handler& operator+=(const connection_type& handler)
		{
			connections_.push_back(handler);
			return *this;
		}

		// Add a connection
		event_handler& operator+=(connection_type&& handler)
		{
			connections_.push_back(std::move(handler));
			return *this;
		}

		// Add a connection from a class instance
		// Alternative: use += std::bind(...)
		template<class Object, ReturnT(Object::*MethodPtr)(Args...)>
		void add_connection(Object* object_ptr)
		{
			connections_.push_back([&object_ptr](Args... args) { return (object_ptr->*MethodPtr)(args...); });
		}

		// Invoke all the connections
		void operator()(Args&&... args) const
		{
			for (const auto& h : connections_)
				h(std::forward<Args>(args)...);
		}

	private:
		std::list<connection_type> connections_; // connections to be invoked
	};
}

#endif