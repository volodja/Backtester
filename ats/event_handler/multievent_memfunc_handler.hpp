#ifndef MULTIEVENT_MEMFUNC_HANDLER_HPP
#define MULTIEVENT_MEMFUNC_HANDLER_HPP

#include <typeindex>         // std::type_index (to be used as a hash map key)
#include <unordered_map>
#include <memory>
//#include "event_args.hpp"
#include <ats/message/message.hpp>

using basic_message = ats::instrument_message;

namespace ats
{
	namespace detail
	{
		class member_function_handler_base
		{
		public:
			virtual ~member_function_handler_base() { }
			virtual void operator()(const basic_message& e) = 0;
		};

		template<class Object, typename EventArgsT>
		class member_function_handler : public ats::detail::member_function_handler_base
		{
		public:
			typedef void(Object::*member_function_type)(const EventArgsT&);
			member_function_handler(member_function_type memfunc, Object* instance)
				: instance_(instance), memfunc_(memfunc) { }

			virtual void operator()(const basic_message& e) override
			{
				(instance_->*memfunc_)(static_cast<const EventArgsT&>(e));
			}

		private:
			Object* instance_;
			member_function_type memfunc_;
		};
	}

	// Event handler that is capable of processing any number of events of type ats::event_args
	// There may be more than one handlers of the same function signature
	class multievent_memfunc_handler
	{
	public:
		// Pass an event to all handlers that process events of the same type
		void invoke(const basic_message& e)
		{
			auto handler_range = handlers_.equal_range(typeid(e));
			for (auto it = handler_range.first; it != handler_range.second; ++it)
				it->second->operator()(e);
		}

		// Add an event handler from a class
		template<class Object, typename EventArgsT>
		void add_event_handler(void(Object::*MemFuncPtr)(const EventArgsT&), Object* instance)
		{
			handlers_.insert(handler_container::value_type(typeid(EventArgsT),
					func_ptr(new ats::detail::member_function_handler<Object, EventArgsT>(MemFuncPtr, instance))));
		}
	private:
		using func_ptr = std::shared_ptr<ats::detail::member_function_handler_base>;
		using handler_container = std::unordered_multimap<std::type_index, func_ptr>;
		handler_container handlers_;
	};
}

#endif