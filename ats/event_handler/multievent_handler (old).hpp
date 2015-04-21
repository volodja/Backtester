#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>

namespace ats
{
	namespace detail
	{
		struct function_base { };

		template<typename T>
		struct basic_function : public function_base
		{
			typedef std::function<T> function_type;
			basic_function(const function_type& func) : function(func) { }
			basic_function(function_type&& func) : function(std::move(func)) { }
			function_type function;
		};
	}

	// Handler for events of any type. There may exist any number of event handlers of the same signature.
	class multievent_handler
	{
		typedef std::shared_ptr<ats::detail::function_base> func_ptr;
		typedef std::unordered_multimap<std::type_index, func_ptr> handlers_container;
	public:
		template<typename FuncT>
		void add_event_handler(FuncT& handler)
		{
			using func_type = std::function<FuncT>;
			func_ptr f(new ats::detail::basic_function<FuncT>(func_type(handler)));
			handlers_.insert(handlers_container::value_type(typeid(FuncT), std::move(f)));
		}

		template<class Object, typename... Args>
		void add_event_handler(void(Object::*MemFuncPtr)(Args...), Object* instance)
		{
			typedef void FuncT(Args...);
			func_ptr f(new ats::detail::basic_function<FuncT>(
					[MemFuncPtr, instance](Args... args) { (instance->*MemFuncPtr)(args...); } // std::forward?
			));
			handlers_.insert(handlers_container::value_type(typeid(FuncT), std::move(f)));
		}

		template<typename... Args>
		void invoke(Args&&... args)
		{
			typedef void FuncT(Args...);
			auto handler_range = handlers_.equal_range(typeid(FuncT));
			for (auto it = handler_range.first; it != handler_range.second; ++it)
			{
				const ats::detail::function_base& f = *it->second;
				std::function<FuncT> func = static_cast<const ats::detail::basic_function<FuncT>&>(f).function;
				func(std::forward<Args>(args)...);
			}
		}
	private:
		handlers_container handlers_;
	};
}
