#ifndef MULTIEVENT_HANDLER_HPP
#define MULTIEVENT_HANDLER_HPP

#include <unordered_map>
#include <list>
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
		typedef std::unordered_map<std::type_index, std::list<func_ptr>> handlers_container;
	public:
		// Register an event handler based on its signature (normally, non-member functions)
		template<typename FuncT>
		void add_event_handler(FuncT& handler)
		{
			// define a basic function of signature FuncT from the passed handler
			func_ptr f_ptr(new ats::detail::basic_function<FuncT>(std::function<FuncT>(handler)));

			// insert the function above into the hash map
			std::type_index index(typeid(FuncT));
			auto it = handlers_.find(index);
			if (it == handlers_.end())
				handlers_[index] = std::list<func_ptr>{ std::move(f_ptr) };
			else
				it->second.push_back(std::move(f_ptr));
		}

		// Register an event handler given a class where it belongs and member function pointer
		template<class Object, typename... Args>
		void add_event_handler(void(Object::*MemFuncPtr)(Args...), Object* instance)
		{
			// define a basic function from the passed member function handler
			typedef void FuncT(Args...);
			func_ptr f_ptr(new ats::detail::basic_function<FuncT>(
					[MemFuncPtr, instance](Args... args) { (instance->*MemFuncPtr)(args...); }
			));

			// insert the function above into the hash map
			std::type_index index(typeid(FuncT));
			auto it = handlers_.find(index);
			if (it == handlers_.end())
				handlers_[index] = std::list<func_ptr>{ std::move(f_ptr) };
			else
				it->second.push_back(std::move(f_ptr));
		}

		// Pass the argument/arguments to all event handlers that take it as a parameter/parameters
		template<typename... Args>
		void invoke(Args&&... args)
		{
			typedef void FuncT(Args...);
			auto it = handlers_.find(typeid(FuncT));
			if (it != handlers_.end())
			{
				for (const auto& f_ptr : it->second)
				{
					std::function<FuncT> func = static_cast<const ats::detail::basic_function<FuncT>&>(*f_ptr).function;
					func(std::forward<Args>(args)...);
				}
			}
		}

		// Same as invoke
		template<typename... Args>
		void operator()(Args&&... args)
		{
			typedef void FuncT(Args...);
			auto it = handlers_.find(typeid(FuncT));
			if (it != handlers_.end())
			{
				for (const auto& f_ptr : it->second)
				{
					std::function<FuncT> func = static_cast<const ats::detail::basic_function<FuncT>&>(*f_ptr).function;
					func(std::forward<Args>(args)...);
				}
			}
		}

	//	size_t size() const { return handlers_.size(); }
	//	bool empty() const { return handlers_.empty(); }
	private:
		handlers_container handlers_;
	};
}

#endif