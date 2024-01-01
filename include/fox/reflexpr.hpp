/// This header is part of Ruby Ecosystem distributed under MIT license.
///
/// Author:			Marcin Poloczek (aka. RedSkittleFox)
///	Contact:		RedSkittleFox@gmail.com
/// Copyright:		Marcin Poloczek
/// License:		MIT
/// Version:		2.0.0
///
/// TODO:
///		* Improve struct parsing.
///		* Introduce noreflect keyword
///		
#ifndef FOX_REFLEXPR_RUBY_REFLEXPR_H_
#define FOX_REFLEXPR_RUBY_REFLEXPR_H_
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <bit>
#include <concepts>
#include <algorithm>
#include <regex>
#include <cassert>
#include <functional>
#include <type_traits>

namespace fox::reflexpr
{
	/// <summary>
	/// Satisifes std::is_aggregate
	/// https://en.cppreference.com/w/cpp/types/is_aggregate
	/// </summary>
	template<class T>
	concept aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

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
	void for_each_member_variable(T&& obj, Pred&& pred);

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
	template<aggregate T, class Pred>
	void for_each_reflected_member_type(Pred&& pred);

	/// <summary>
	/// Calculates compile time number of member variables
	/// </summary>
	/// <typeparam name="T">Aggregate Type</typeparam>
	template<aggregate T>
	struct member_count;
}

namespace fox::reflexpr
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

			auto func = [index = static_cast<size_t>(0llu), &type, &pred]<class U>(U&& v) mutable
			{
				std::invoke(pred, std::forward<U>(v), type.member_variables[index].name);
				++index;
			};

			::fox::reflexpr::for_each_member_variable(obj, func);
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
			assert(false && "ruby_reflexpr: Something went horribly wrong.");
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

			::fox::reflexpr::for_each_member_variable(temp, func);
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
		operator T() {}
	};

	template<aggregate T, class... Args>
	struct _member_count
	{
		constexpr static size_t f(int32_t*)
		{
			return sizeof...(Args) - 1;
		}

		template<class U = T, class Enabled = decltype(U{ Args{}... }) >
		constexpr static size_t f(std::nullptr_t)
		{
			return _member_count<T, Args..., _any_type>::value;
		}

		constexpr static auto value = f(nullptr);
	};

	template<aggregate T>
	struct member_count
	{
		constexpr static std::size_t value = _member_count<std::remove_cvref_t<T>>::value;
	};

#ifdef FOX_REFLEXPR_UNPACK_APPLY
#pragma message "FOX_REFLEXPR_UNPACK_APPLY macro is internally used by redskittlefox/reflexpr library"
#undef FOX_REFLEXPR_UNPACK_APPLY
#endif

#define FOX_REFLEXPR_UNPACK_APPLY(SIZE, ...)			\
	if constexpr ( size == SIZE )		\
	{									\
		auto&& [__VA_ARGS__] = obj;		\
		apply_pack( std::in_place_index< SIZE >,  __VA_ARGS__ ); \
	}

	template<aggregate T, class Pred>
	void for_each_member_variable(T&& obj, Pred&& pred)
	{
		static constexpr size_t size = member_count<T>::value;

		auto apply_pack = [&]<std::size_t I, class... Args>(std::in_place_index_t<I>, Args&&... args)
		{
			static_assert(I == sizeof...(Args) && "Size and Arguments mismatch.");
			auto invoke_proxy = [&]<class U>(U && arg) -> void
			{
				static_assert(std::is_invocable_v<decltype(pred), decltype(arg)>, "Function is not invokable with type [U]");
				pred( std::forward<U>(arg) );
			};

			( invoke_proxy(std::forward<Args>(args) ) , ...);
		};

		FOX_REFLEXPR_UNPACK_APPLY(1, v0)
		FOX_REFLEXPR_UNPACK_APPLY(2, v0, v1)
		FOX_REFLEXPR_UNPACK_APPLY(3, v0, v1, v2)
		FOX_REFLEXPR_UNPACK_APPLY(4, v0, v1, v2, v3)
		FOX_REFLEXPR_UNPACK_APPLY(5, v0, v1, v2, v3, v4)
		FOX_REFLEXPR_UNPACK_APPLY(6, v0, v1, v2, v3, v4, v5)
		FOX_REFLEXPR_UNPACK_APPLY(7, v0, v1, v2, v3, v4, v5, v6)
		FOX_REFLEXPR_UNPACK_APPLY(8, v0, v1, v2, v3, v4, v5, v6, v7)
		FOX_REFLEXPR_UNPACK_APPLY(9, v0, v1, v2, v3, v4, v5, v6, v7, v8)
		FOX_REFLEXPR_UNPACK_APPLY(10, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9)
		FOX_REFLEXPR_UNPACK_APPLY(11, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)
		FOX_REFLEXPR_UNPACK_APPLY(12, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11)
		FOX_REFLEXPR_UNPACK_APPLY(13, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12)
		FOX_REFLEXPR_UNPACK_APPLY(14, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13)
		FOX_REFLEXPR_UNPACK_APPLY(15, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14)
		FOX_REFLEXPR_UNPACK_APPLY(16, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15)
		FOX_REFLEXPR_UNPACK_APPLY(17, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16)
		FOX_REFLEXPR_UNPACK_APPLY(18, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17)
		FOX_REFLEXPR_UNPACK_APPLY(19, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18)
		FOX_REFLEXPR_UNPACK_APPLY(20, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19)
		FOX_REFLEXPR_UNPACK_APPLY(21, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20)
		FOX_REFLEXPR_UNPACK_APPLY(22, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21)
		FOX_REFLEXPR_UNPACK_APPLY(23, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22)
		FOX_REFLEXPR_UNPACK_APPLY(24, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23)
		FOX_REFLEXPR_UNPACK_APPLY(25, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24)
		FOX_REFLEXPR_UNPACK_APPLY(26, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25)
		FOX_REFLEXPR_UNPACK_APPLY(27, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26)
		FOX_REFLEXPR_UNPACK_APPLY(28, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27)
		FOX_REFLEXPR_UNPACK_APPLY(29, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28)
		FOX_REFLEXPR_UNPACK_APPLY(30, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29)
		FOX_REFLEXPR_UNPACK_APPLY(31, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30)
		FOX_REFLEXPR_UNPACK_APPLY(32, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31)
		FOX_REFLEXPR_UNPACK_APPLY(33, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32)
		FOX_REFLEXPR_UNPACK_APPLY(34, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33)
		FOX_REFLEXPR_UNPACK_APPLY(35, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34)
		FOX_REFLEXPR_UNPACK_APPLY(36, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35)
		FOX_REFLEXPR_UNPACK_APPLY(37, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36)
		FOX_REFLEXPR_UNPACK_APPLY(38, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37)
		FOX_REFLEXPR_UNPACK_APPLY(39, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38)
		FOX_REFLEXPR_UNPACK_APPLY(40, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38, v39)

		static_assert(size <= 40, "Unsupported number of struct members");
	};

#undef FOX_REFLEXPR_UNPACK_APPLY

	template<std::default_initializable T, class Pred>
	void for_each_member_type(Pred&& pred)
	{
		auto proxy = [&pred]<class U>(const U & v) -> void
		{
			pred.template operator() < U > ();
		};

		const T* v = nullptr;
		for_each_member_variable(*v, proxy);
	};

	template<aggregate T, class Pred>
	void for_each_reflected_member_variable(T& obj, Pred&& pred)
	{
		_reflexpr::for_each_member_variable(obj, pred);
	};

	template<aggregate T, class Pred>
	void for_each_reflected_member_type(Pred&& pred)
	{
		auto proxy = [&pred]<class U>(const U & v, const std::string & name) -> void
		{
			pred.template operator() < U > (name);
		};

		const T* v = nullptr;
		_reflexpr::for_each_member_variable(*v, proxy);
	};

}


#define _REFLECT_IDENTIFIER_PROXY(X) reflexpr_##X
#define _REFLECT_IDENTIFIER(X) _REFLECT_IDENTIFIER_PROXY(X)

#define REFLECT(...) typedef __VA_ARGS__ _REFLECT_IDENTIFIER(__LINE__)##_t;\
	static size_t _REFLECT_IDENTIFIER(__LINE__)##_proxy = \
		::fox::reflexpr::_reflexpr::type_register_proxy<_REFLECT_IDENTIFIER(__LINE__)##_t>(#__VA_ARGS__);\

#endif