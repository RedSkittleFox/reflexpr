/// This header is distributed under MIT license.
///
/// Author:			Marcin Poloczek (aka. RedSkittleFox)
///	Contact:		RedSkittleFox@gmail.com
/// Copyright:		Marcin Poloczek
/// License:		MIT
/// Version:		3.0.0
///

#ifndef FOX_REFLEXPR_REFLEXPR_H_
#define FOX_REFLEXPR_REFLEXPR_H_
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
#include <tuple>
#include <limits>
#include <utility>

namespace fox::reflexpr
{
	template<class T>
	concept aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;
}

#define FOX_REFLEXPR_NUM_SUPPORTED_MEMBERS (static_cast<std::size_t>(40))

namespace fox::reflexpr
{
	namespace details
	{
		// This is a nasty work-around that makes counting members of aggregates of 
		// references possible. We iterate over all configurations from N to 0 and check if they
		// satisfy aggregate initialization. If we reach zero, we check if type is empty, if not, then that means 
		// there are more members than selected N. We could transfer initial size and increase it, but 
		// picking a sane value like 30 should satisfy 90% of cases.
		template<size_t I>
		struct to_any_type_reference
		{
			template<class T>
			constexpr operator T& ();
		};

		template<class T, class, size_t... I>
		struct member_construct_helper_1 : std::false_type {};

		template<class T, size_t... I>
		struct member_construct_helper_1 < T, std::void_t<decltype(
			T{ std::declval<to_any_type_reference<I>>()... }
		) > , I... > : std::true_type {};

		template<class T, class IndexList>
		struct member_counter_helper_2;

		template<class T, size_t... I>
		struct member_counter_helper_2<T, std::index_sequence<I...>>
			: member_construct_helper_1<T, void, I...> {};

		template<class T, size_t Size>
		struct member_counter_helper_3
			: member_counter_helper_2<T, std::make_index_sequence<Size>> {};

		template<class T, size_t I>
		struct member_counter_helper_4
		{
			static constexpr size_t value =
				member_counter_helper_3<T, I>::value ?
				I : member_counter_helper_4<T, I - 1>::value;
		};

		template<class T>
		struct member_counter_helper_4<T, 0>
		{
			static constexpr size_t value =
				std::is_empty_v<T> ?
				0 : std::numeric_limits<size_t>::max();
		};
	}

	/**
	 * \brief		Provides access to the number of elements in a tuple as a compile-time constant expression.
	 * \tparam T	Aggregate type
	 */
	template<aggregate T>
	struct tuple_size :
		::fox::reflexpr::details::member_counter_helper_4<std::remove_cvref_t<T>, FOX_REFLEXPR_NUM_SUPPORTED_MEMBERS> {};

	/**
	 * \brief		Helper variable template. Provides access to the number of elements in a tuple as a compile-time constant expression.
	 * \tparam T	Aggregate type
	 */
	template<class T>
	static constexpr std::size_t tuple_size_v = tuple_size<T>::value;

	namespace details
	{
		template<std::size_t I, std::size_t J>
		struct to_any_type_reference_if_not_j
		{
			template<class T>
			constexpr operator T& ();
		};

		template<std::size_t I>
		struct to_any_type_reference_if_not_j<I, I>
		{
			template<class T>
			constexpr operator T ();
		};

		template<class T, class, std::size_t I, std::size_t... Is>
		struct is_nth_a_reference_helper_1 : std::false_type {};

		template<class T, std::size_t I, std::size_t... Is>
		struct is_nth_a_reference_helper_1 < T, std::void_t<decltype(
			T{ std::declval<to_any_type_reference_if_not_j<I, Is>>()... }
		) > , I, Is... > : std::true_type {};

		template<class T, std::size_t I, class>
		struct is_nth_a_reference_helper_2;

		template<class T, std::size_t I, std::size_t... Is>
		struct is_nth_a_reference_helper_2<T, I, std::index_sequence<Is...>> :
			is_nth_a_reference_helper_1<T, void, I, Is...> {};

		template<class T, std::size_t I>
		struct is_nth_a_reference : is_nth_a_reference_helper_2<T, I, std::make_index_sequence<::fox::reflexpr::tuple_size_v<T>>> {};

		template<class T, class>
		struct ref_detector_helper_1;

		template<class T, std::size_t... Is>
		struct ref_detector_helper_1<T, std::index_sequence<Is...>>
		{
			static constexpr std::array<bool, ::fox::reflexpr::tuple_size_v<T>> nth_reference{ std::negation_v<is_nth_a_reference<T, Is>>... };
		};

		template<class T>
		struct ref_detector : ref_detector_helper_1<std::remove_cvref_t<T>, std::make_index_sequence<::fox::reflexpr::tuple_size_v<T>>> {};
	}

#ifdef FOX_REFLEXPR_UNPACK_APPLY
#pragma message "FOX_REFLEXPR_UNPACK_APPLY macro is internally used by redskittlefox/reflexpr library"
#undef FOX_REFLEXPR_UNPACK_APPLY
#endif

#ifdef FOX_REFLEXPR_UNPACK_ALL
#pragma message "FOX_REFLEXPR_UNPACK_ALL macro is internally used by redskittlefox/reflexpr library"
#undef FOX_REFLEXPR_UNPACK_ALL
#endif

#define FOX_REFLEXPR_UNPACK_APPLY(SIZE, ...)			\
	if constexpr ( size == SIZE )		\
	{									\
		auto&& [__VA_ARGS__] = obj;		\
		return apply_pack( std::in_place_index< SIZE >, __VA_ARGS__ ); \
	}

#define FOX_REFLEXPR_UNPACK_ALL	\
	FOX_REFLEXPR_UNPACK_APPLY( 1, v0) \
	FOX_REFLEXPR_UNPACK_APPLY( 2, v0, v1) \
	FOX_REFLEXPR_UNPACK_APPLY( 3, v0, v1, v2) \
	FOX_REFLEXPR_UNPACK_APPLY( 4, v0, v1, v2, v3) \
	FOX_REFLEXPR_UNPACK_APPLY( 5, v0, v1, v2, v3, v4) \
	FOX_REFLEXPR_UNPACK_APPLY( 6, v0, v1, v2, v3, v4, v5) \
	FOX_REFLEXPR_UNPACK_APPLY( 7, v0, v1, v2, v3, v4, v5, v6) \
	FOX_REFLEXPR_UNPACK_APPLY( 8, v0, v1, v2, v3, v4, v5, v6, v7) \
	FOX_REFLEXPR_UNPACK_APPLY( 9, v0, v1, v2, v3, v4, v5, v6, v7, v8) \
	FOX_REFLEXPR_UNPACK_APPLY(10, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9) \
	FOX_REFLEXPR_UNPACK_APPLY(11, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) \
	FOX_REFLEXPR_UNPACK_APPLY(12, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11) \
	FOX_REFLEXPR_UNPACK_APPLY(13, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12) \
	FOX_REFLEXPR_UNPACK_APPLY(14, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13) \
	FOX_REFLEXPR_UNPACK_APPLY(15, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14) \
	FOX_REFLEXPR_UNPACK_APPLY(16, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15) \
	FOX_REFLEXPR_UNPACK_APPLY(17, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
	FOX_REFLEXPR_UNPACK_APPLY(18, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17) \
	FOX_REFLEXPR_UNPACK_APPLY(19, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18) \
	FOX_REFLEXPR_UNPACK_APPLY(20, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19) \
	FOX_REFLEXPR_UNPACK_APPLY(21, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20) \
	FOX_REFLEXPR_UNPACK_APPLY(22, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21) \
	FOX_REFLEXPR_UNPACK_APPLY(23, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22) \
	FOX_REFLEXPR_UNPACK_APPLY(24, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23) \
	FOX_REFLEXPR_UNPACK_APPLY(25, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24) \
	FOX_REFLEXPR_UNPACK_APPLY(26, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25) \
	FOX_REFLEXPR_UNPACK_APPLY(27, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26) \
	FOX_REFLEXPR_UNPACK_APPLY(28, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27) \
	FOX_REFLEXPR_UNPACK_APPLY(29, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28) \
	FOX_REFLEXPR_UNPACK_APPLY(30, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29) \
	FOX_REFLEXPR_UNPACK_APPLY(31, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30) \
	FOX_REFLEXPR_UNPACK_APPLY(32, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31) \
	FOX_REFLEXPR_UNPACK_APPLY(33, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32) \
	FOX_REFLEXPR_UNPACK_APPLY(34, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33) \
	FOX_REFLEXPR_UNPACK_APPLY(35, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34) \
	FOX_REFLEXPR_UNPACK_APPLY(36, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35) \
	FOX_REFLEXPR_UNPACK_APPLY(37, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36) \
	FOX_REFLEXPR_UNPACK_APPLY(38, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37) \
	FOX_REFLEXPR_UNPACK_APPLY(39, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38) \
	FOX_REFLEXPR_UNPACK_APPLY(40, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38, v39) 

	/**
	 * \brief	Allows iteration over all members of an aggregate type.
	 * \tparam T Aggregate type
	 * \tparam Func Function type
	 * \param obj Object to iterate over members of.
	 * \param func Functor invoked for each member.
	 */
	template<aggregate T, class Func>
	constexpr void for_each(T&& obj, Func&& func)
	{
		constexpr size_t size = tuple_size_v<T>;
		static_assert(size <= FOX_REFLEXPR_NUM_SUPPORTED_MEMBERS, "Unsupported number of struct members");

		auto apply_pack = [&]<std::size_t I, class... Args>(std::in_place_index_t<I>, Args&&... args) -> void
		{
			static_assert(I == sizeof...(Args) && "Size and Arguments mismatch.");
			if constexpr(std::is_lvalue_reference_v<decltype(obj)> && std::is_const_v<std::remove_reference_t<decltype(obj)>>)
			{
				(func(std::as_const(std::forward<Args>(args))), ...);
			}
			else
			{
				(func(std::forward<Args>(args)), ...);
			}
		};

		FOX_REFLEXPR_UNPACK_ALL
	};

	/**
	 * \brief Creates a tuple of lvalue references to members of an aggregate.
	 * \tparam T Aggregate type
	 * \param obj Object to make a tie of.
	 * \return A std::tuple object containing lvalue references.
	 */
	template<aggregate T>
	constexpr auto tie(T& obj)
	{
		constexpr size_t size = tuple_size_v<T>;
		static_assert(size <= FOX_REFLEXPR_NUM_SUPPORTED_MEMBERS, "Unsupported number of struct members");

		auto apply_pack = [&]<std::size_t I, class... Args>(std::in_place_index_t<I>, Args&&... args)
		{
			static_assert(I == sizeof...(Args) && "Size and Arguments mismatch.");
			if constexpr(std::is_const_v<T>)
			{
				return std::tuple<std::add_lvalue_reference_t<std::add_const_t<std::remove_cvref_t<Args>>>...>{ std::forward<Args>(args)...};
			}
			else
			{
				return std::tuple<Args&...>{ std::forward<Args>(args)...};
			}
		};

		FOX_REFLEXPR_UNPACK_ALL
	}

	/**
	 * \brief Creates a tuple object, deducing the target type from the types of members.
	 * \tparam T Aggregate type
	 * \param obj Object to make the tuple from.
	 * \return A std::tuple object containing the given values.
	 */
	template<aggregate T>
	constexpr auto make_tuple(T&& obj)
	{
		constexpr size_t size = tuple_size_v<T>;
		static_assert(size <= FOX_REFLEXPR_NUM_SUPPORTED_MEMBERS, "Unsupported number of struct members");

		auto apply_pack = [&]<std::size_t I, class... Args>(std::in_place_index_t<I>, Args&&... args)
		{
			static_assert(I == sizeof...(Args) && "Size and Arguments mismatch.");
			using ref_tester = details::ref_detector<T>;

			return [&]<std::size_t... Is>(std::index_sequence<Is...>)
			{
				using tuple = std::tuple<Args...>;

				if
				constexpr ( std::is_const_v<std::remove_reference_t<T>> )
				{
					return std::tuple<
						std::conditional_t < ref_tester::nth_reference[Is],
						std::add_lvalue_reference_t<std::add_const_t<std::decay_t<std::tuple_element_t<Is, tuple>>>>,
						std::remove_reference_t<std::decay_t<std::tuple_element_t<Is, tuple>>>
						> ... >(std::forward<Args>(args)...);
				}
				else
				{
					return std::tuple<
						std::conditional_t < ref_tester::nth_reference[Is],
						std::add_lvalue_reference_t<std::decay_t<std::tuple_element_t<Is, tuple>>>,
						std::remove_reference_t<std::decay_t<std::tuple_element_t<Is, tuple>>>
					> ... >(std::forward<Args>(args)...);
				}
			}(std::index_sequence_for<Args...>{});
		};

		FOX_REFLEXPR_UNPACK_ALL
	}

#undef FOX_REFLEXPR_UNPACK_APPLY

	/**
	 * \brief Provides compile-time indexed access to the types of the elements of the aggregate
	 * \tparam I Index of the element
	 * \tparam T Aggregate type 
	 */
	template<std::size_t I, aggregate T>
	struct tuple_element
	{
		using type = std::tuple_element_t<I, decltype(::fox::reflexpr::make_tuple(std::declval<T>()))>;
	};

	/**
	 * \brief Helper variable template. Provides compile-time indexed access to the types of the elements of the aggregate
	 * \tparam I Index of the element
	 * \tparam T Aggregate type
	 */
	template<std::size_t I, aggregate T>
	using tuple_element_t = typename tuple_element<I, T>::type;

	/**
	 * \brief Extracts the Ith element from the aggregate.
	 * \tparam I Index of the element
	 * \tparam T Aggregate type
	 * \param obj Object to extract element from
	 * \return A reference to selected element of obj
	 */
	template<std::size_t I, aggregate T>
	auto get(T& obj) noexcept ->
	std::add_lvalue_reference_t<fox::reflexpr::tuple_element_t<I, T>>
		requires ( tuple_size_v<T> > I )
	{
		return std::get<I>(fox::reflexpr::tie(obj));
	}

	/**
	 * \brief Extracts the Ith element from the aggregate.
	 * \tparam I Index of the element
	 * \tparam T Aggregate type
	 * \param obj Object to extract element from
	 * \return A reference to selected element of obj
	 */
	template<std::size_t I, aggregate T>
	auto get(const T& obj) noexcept ->
		std::add_lvalue_reference_t<std::add_const_t<std::remove_cvref_t<fox::reflexpr::tuple_element_t<I, T>>>>
		requires (tuple_size_v<T> > I)
	{
		return std::get<I>(fox::reflexpr::tie(obj));
	}
}

#endif