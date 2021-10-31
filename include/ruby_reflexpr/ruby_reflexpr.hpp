/// This header is part of Ruby Ecosystem distributed under MIT license.
///
/// Author:			Marcin Poloczek (aka. RedSkittleFox)
///	Contact:		RedSkittleFox@gmail.com
/// Copyright:		Marcin Poloczek
/// License:		MIT
/// Version:		0.1.0
///
/// TODO:
///		* Improve struct parsing.
///		* Introduce noreflect keyword
///		
#ifndef RUBY_RUBY_REFLEXPR_RUBY_REFLEXPR_H_
#define RUBY_RUBY_REFLEXPR_RUBY_REFLEXPR_H_
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <bit>
#include <concepts>
#include <algorithm>
#include <regex>

namespace ruby_reflexpr
{
	/// <summary>
	/// Satisifes std::is_aggregate
	/// https://en.cppreference.com/w/cpp/types/is_aggregate
	/// </summary>
	template<class T>
	concept aggregate = std::is_aggregate_v<T>;

	/// <summary>
	/// Iterates over every member variable of an aggregate 
	/// </summary>
	/// <typeparam name="T">Aggregate's Type</typeparam>
	/// <typeparam name="Pred">Predicate's Type</typeparam>
	/// <param name="obj">Aggregate to iterate over</param>
	/// <param name="pred">Predicate that iterates over object. Requires to be invocable with T&, where T is a member type of an aggregate.</param>
	/// <example>
	///		void foo()
	///		{
	///			std::cout << "For each member variable:\n";
	///			auto func = []<class T>(T & v)
	///			{
	///				std::cout << "Type: " << typeid(T).name() << " Value: " << v << '\n';
	///			};
	///			
	///			aggregate_type at{ 1 , 3.5f, "Foxes are great!" };
	///			
	///			reflexpr::for_each_member_variable(at, func);
	///			std::cout << '\n';
	///		}
	/// </example>
	template<aggregate T, class Pred>
	void for_each_member_variable(T& obj, Pred&& pred);

	/// <summary>
	/// Iterates over every member variable's type of an aggregate
	/// </summary>
	/// <typeparam name="T">Aggregate's Type</typeparam>
	/// <typeparam name="Pred">Predicate's Type</typeparam>
	/// <param name="pred">Predicate that iterates over object. Invocable as pred.operator()<T>(),
	/// where T is a member type of an aggregate.</param>
	/// <example>
	///		struct functor
	///		{
	///			template<class T>
	///			void operator()() const
	///			{
	///				std::cout << "Type: " << typeid(T).name() << '\n';
	///			};
	///		};
	/// 
	///		void foo()
	///		{
	///			std::cout << "For each member type:\n";
	///			reflexpr::for_each_member_type<aggregate_type, functor>(functor{});
	///			std::cout << '\n';
	///		}
	/// </example>
	template<std::default_initializable T, class Pred>
	void for_each_member_type(Pred&& pred);

	/// <summary>
	/// Iterates over every member variable of an aggregate 
	/// </summary>
	/// <typeparam name="T">Reflected Aggregate's Type</typeparam>
	/// <typeparam name="Pred">Predicate's Type</typeparam>
	/// <param name="obj">Aggregate to iterate over</param>
	/// <param name="pred">Predicate that iterates over object. Requires to be invocable with T& and const std::string&, where T is a member type of an aggregate.</param>
	/// <example>
	///
	///		REFLECT(
	///		struct aggregate_type_reflected
	///		{
	///			int a;
	///			float b;
	///			std::string str;
	///		}
	///		);
	///		void foo()
	///		{
	///			std::cout << "For each member variable reflected:\n";
	///			auto func = []<class T>(T & v, const std::string & name)
	///			{
	///				std::cout << "Name: " << name << " Type: " << typeid(T).name() << " Value: " << v << '\n';
	///			};
	///			
	///			aggregate_type_reflected at{ 1 , 3.5f, "Foxes are great!" };
	///			
	///			reflexpr::for_each_reflected_member_variable(at, func);
	///			std::cout << '\n';
	///		}
	/// </example>
	template<aggregate T, class Pred>
	void for_each_reflected_member_variable(T& obj, Pred&& pred);

	/// <summary>
	/// Iterates over every member variable's type of an aggregate
	/// </summary>
	/// <typeparam name="T">Reflected Aggregate's Type</typeparam>
	/// <typeparam name="Pred">Predicate's Type</typeparam>
	/// <param name="pred">Predicate that iterates over object. Invocable as pred.operator()<T>(const std::string&),
	/// where T is a member type of an aggregate.</param>
	/// <example>
	///		REFLECT(
	///		struct aggregate_type_reflected
	///		{
	///			int a;
	///			float b;
	///			std::string str;
	///		}
	///		);
	///
	///		struct functor_reflected
	///		{
	///			template<class T>
	///			void operator()(const std::string& name) const
	///			{
	///				std::cout << "Name: " << name << " Type: " << typeid(T).name() << '\n';
	///			};
	///		};
	///		
	///		void foo()
	///		{
	///			std::cout << "For each member type reflected:\n";
	///			reflexpr::for_each_reflected_member_type<aggregate_type_reflected, functor_reflected>(functor_reflected{});
	///			std::cout << '\n';
	///		}
	/// </example>
	template<std::default_initializable T, class Pred>
	void for_each_reflected_member_type(Pred&& pred);

	/// <summary>
	/// Calculates compile time number of member variables
	/// </summary>
	/// <typeparam name="T">Aggregate Type</typeparam>
	template<aggregate T, class... Args>
	struct member_count;
}

namespace ruby_reflexpr
{
	class _reflexpr
	{
		struct _member_variable_t
		{
			std::string name;
			std::type_index type_index = std::type_index(typeid(void));
			std::ptrdiff_t offset;
		};

		struct _type_t
		{
			std::string name;
			std::type_index type_index = std::type_index(typeid(void));
			std::vector<_member_variable_t> member_variables;
		};

	private:
		inline static std::unordered_map<std::type_index, _type_t> type_info_;

	public:
		template<aggregate T, class Pred>
		static void for_each_member_variable(T& obj, Pred&& pred)
		{
			auto r = type_info_.find(std::type_index(typeid(T)));
			if (r == std::end(type_info_))
				throw std::logic_error(std::string("reflexpr: Trying to use not reflected type: ") + typeid(T).name());

			const auto& type = r->second;

			auto func = [index = static_cast<size_t>(0llu), &type, &pred]<class T>(T & v) mutable
			{
				std::invoke(pred, v, type.member_variables[index].name);
				++index;
			};

			::ruby_reflexpr::for_each_member_variable(obj, func);
		};

	private:
		inline static size_t counter_ = 0;

		template<class T>
		static void type_register_member_variable_names_(std::string contents) try
		{
			// get to the inside of the struct
			contents = std::string(std::find(std::begin(contents), std::end(contents), '{') + 1, std::end(contents) - 1);

			static const std::string member_var_pattern =
				R"(([a-zA-Z_]+\w*(?=\s*;)))";

			static const std::regex member_var_exp(member_var_pattern,
				std::regex::ECMAScript | std::regex::optimize);

			static const std::string comment_pattern =
				R"(((?:\/\*)[\w\s]*(?:\*\/))|((?:\/\/)[\w\s]*\n))";

			static const std::regex comment_exp(comment_pattern,
				std::regex::ECMAScript | std::regex::optimize);

			static const std::string remove_initializations_pattern
				= R"((\=\s*\{[\w\s,.=\{\}]*\})|(\s*\{[\w\s,.=\{\}]*\})|(=\s*[\w\s,.=\{\}]*))";

			static const std::regex remove_initializations_exp(remove_initializations_pattern,
				std::regex::ECMAScript | std::regex::optimize);

			// Remove comments
			contents = std::regex_replace(contents, comment_exp, "\n");
			contents = std::regex_replace(contents, remove_initializations_exp, "\n");

			// Find member variables
			auto match = std::sregex_iterator(
				std::begin(contents), std::end(contents), member_var_exp);

			auto& type = type_info_[std::type_index(typeid(T))];

			for (auto rng = std::ranges::subrange(match, std::sregex_iterator());
				auto & e : rng)
			{
				type.member_variables.push_back(_member_variable_t{ e.str() });
			}

			// type.member_variables.shrink_to_fit();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << '\n';
		};

		template<class T>
		static void type_register_member_variable_type_infos_()
		{
			T temp{};
			auto address = std::bit_cast<ptrdiff_t>(std::addressof(temp));
			auto& type = type_info_[std::type_index(typeid(T))];

			auto func = [index = static_cast<size_t>(0llu), &type, address]<class U>(U & v) mutable
			{
				type.member_variables[index].type_index = std::type_index(typeid(U));
				type.member_variables[index].offset = std::bit_cast<ptrdiff_t>(std::addressof(v)) - address;
				++index;
			};

			::ruby_reflexpr::for_each_member_variable<T>(temp, func);
		}

	public:
		template<aggregate  T>
		static size_t type_register_proxy(std::string contents)
		{
			if (type_info_.contains(typeid(T)))
				return ++counter_;

			type_register_member_variable_names_<T>(contents);
			type_register_member_variable_type_infos_<T>();

			return ++counter_;
		}
	};

	struct _any_type
	{
		template<class T>
		operator T() {};
	};

	template<aggregate T, class... Args>
	struct member_count
	{
		constexpr static size_t f(int32_t*)
		{
			return sizeof...(Args) - 1;
		}

		template<class U = T, class enabld = decltype(U{ Args{}... }) >
		constexpr static size_t f(std::nullptr_t)
		{
			return member_count<T, Args..., _any_type>::value;
		}

		constexpr static auto value = f(nullptr);
	};

	template<aggregate T, class Pred>
	void for_each_member_variable(T& obj, Pred&& pred)
	{
		static constexpr size_t size = member_count<T>::value;

		if constexpr (size == 1)
		{
			auto& [v0] = obj;
			std::invoke(pred, v0);
		}
		else if constexpr (size == 2)
		{
			auto& [v0, v1] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
		}
		else if constexpr (size == 3)
		{
			auto& [v0, v1, v2] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
		}
		else if constexpr (size == 4)
		{
			auto& [v0, v1, v2, v3] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
		}
		else if constexpr (size == 5)
		{
			auto& [v0, v1, v2, v3, v4] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
		}
		else if constexpr (size == 6)
		{
			auto& [v0, v1, v2, v3, v4, v5] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
		}
		else if constexpr (size == 7)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
		}
		else if constexpr (size == 8)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
		}
		else if constexpr (size == 9)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
		}
		else if constexpr (size == 10)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
		}
		else if constexpr (size == 11)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
		}
		else if constexpr (size == 12)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
		}
		else if constexpr (size == 13)
			{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
		}
		else if constexpr (size == 14)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
		}
		else if constexpr (size == 15)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
		}
		else if constexpr (size == 16)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
			std::invoke(pred, v15);
		}
		else if constexpr (size == 16)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
			std::invoke(pred, v15);
		}
		else if constexpr (size == 17)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
			std::invoke(pred, v15);
			std::invoke(pred, v16);
		}
		else if constexpr (size == 18)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
			std::invoke(pred, v15);
			std::invoke(pred, v16);
			std::invoke(pred, v17);
		}
		else if constexpr (size == 19)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
			std::invoke(pred, v15);
			std::invoke(pred, v16);
			std::invoke(pred, v17);
			std::invoke(pred, v18);
		}
		else if constexpr (size == 20)
		{
			auto& [v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19] = obj;
			std::invoke(pred, v0);
			std::invoke(pred, v1);
			std::invoke(pred, v2);
			std::invoke(pred, v3);
			std::invoke(pred, v4);
			std::invoke(pred, v5);
			std::invoke(pred, v6);
			std::invoke(pred, v7);
			std::invoke(pred, v8);
			std::invoke(pred, v9);
			std::invoke(pred, v10);
			std::invoke(pred, v11);
			std::invoke(pred, v12);
			std::invoke(pred, v13);
			std::invoke(pred, v14);
			std::invoke(pred, v15);
			std::invoke(pred, v16);
			std::invoke(pred, v17);
			std::invoke(pred, v18);
			std::invoke(pred, v19);
		}
		else
		{
			static_assert(false, "Unsupported number of struct members");
		}
	};

	template<std::default_initializable T, class Pred>
	void for_each_member_type(Pred&& pred)
	{
		auto proxy = [&pred]<class U>(U & v) -> void
		{
			pred.template operator() < U > ();
		};

		auto v = T{};
		for_each_member_variable(v, proxy);
	};

	template<aggregate T, class Pred>
	void for_each_reflected_member_variable(T& obj, Pred&& pred)
	{
		_reflexpr::for_each_member_variable(obj, pred);
	};

	template<std::default_initializable T, class Pred>
	void for_each_reflected_member_type(Pred&& pred)
	{
		auto proxy = [&pred]<class U>(U & v, const std::string & name) -> void
		{
			pred.template operator() < U > (name);
		};

		auto v = T{};
		_reflexpr::for_each_member_variable(v, proxy);
	};

}


#define _REFLECT_IDENTIFIER_PROXY(X) reflexpr_##X
#define _REFLECT_IDENTIFIER(X) _REFLECT_IDENTIFIER_PROXY(X)

#define REFLECT(...) typedef __VA_ARGS__ _REFLECT_IDENTIFIER(__LINE__)##_t;\
	static size_t _REFLECT_IDENTIFIER(__LINE__)##_proxy = \
		::ruby_reflexpr::_reflexpr::type_register_proxy<_REFLECT_IDENTIFIER(__LINE__)##_t>(#__VA_ARGS__);\

#endif