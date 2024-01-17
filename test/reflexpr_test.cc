#include <gtest/gtest.h>
#include <fox/reflexpr.hpp>

#include <random>
#include <ranges>
#include <concepts>
#include <array>
#include <utility>

namespace fox::reflexpr
{
#include "reflexpr_test_types.inl"
}

namespace fox::reflexpr
{
	template<class Type>
	class reflexpr_test : public testing::Test {};

	TYPED_TEST_SUITE_P(reflexpr_test);

	TYPED_TEST_P(reflexpr_test, aggregate_concept)
	{
		using value_type = TypeParam;
		EXPECT_EQ(std::is_aggregate_v<value_type>, static_cast<bool>(::fox::reflexpr::aggregate<value_type>));
	}

	TYPED_TEST_P(reflexpr_test, for_each)
	{
		using value_type = TypeParam;

		value_type v;

		auto arr = [&]<std::size_t... Is>(std::index_sequence<Is...>)
		{
			return std::array<int, value_type::member_count>{
				v.template get<Is>()...
			};
		}(std::make_index_sequence<value_type::member_count>{});
		
		::fox::reflexpr::for_each(v, [&, i = static_cast<std::size_t>(0)]<class T>(T&& e) mutable
			{
				EXPECT_TRUE(std::is_lvalue_reference_v<decltype(e)> && std::negation_v<std::is_const<std::remove_reference_t<decltype(e)>>>);
				EXPECT_EQ(arr[i++], e);
			});

		::fox::reflexpr::for_each(std::as_const(v), [&, i = static_cast<std::size_t>(0)]<class T>(T&& e) mutable
		{
			if constexpr(std::is_lvalue_reference_v<decltype(e)>)
			{
				EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(e)>>)
					<< typeid(T).name();
			}
			
			EXPECT_EQ(arr[i++], e);
		});
	}

	TYPED_TEST_P(reflexpr_test, tuple_size)
	{
		using value_type = TypeParam;
		EXPECT_EQ(value_type::member_count, ::fox::reflexpr::tuple_size<value_type>::value);
		EXPECT_EQ(value_type::member_count, ::fox::reflexpr::tuple_size_v<value_type>);
	}

	TYPED_TEST_P(reflexpr_test, tuple_element)
	{
		using value_type = TypeParam;
		[]<std::size_t... Is>(std::index_sequence<Is...>)
		{
			EXPECT_TRUE(
				(std::is_same_v<decltype(std::declval<value_type>().template get<Is>()), fox::reflexpr::tuple_element_t<Is, value_type>> && ...)
			);
		}(std::make_index_sequence<value_type::member_count>{});
	}

	TYPED_TEST_P(reflexpr_test, get)
	{
		using value_type = TypeParam;
		{
			value_type value;

			[&] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				([&]<std::size_t I>(std::in_place_index_t<I>)
				{
					auto& v0 = fox::reflexpr::get<I>(value);
					auto v1 = value.template get<I>();
					EXPECT_TRUE(std::negation_v<std::is_const<std::remove_reference_t<decltype(v0)>>>);

					EXPECT_EQ(v0, v1);
					v0 *= 2;
					auto v3 = value.template get<I>();
					EXPECT_NE(v0, v1);
					EXPECT_NE(v1, v3);
					EXPECT_EQ(v0, v3);

				}(std::in_place_index<Is>), ...);
			}(std::make_index_sequence<value_type::member_count>{});
		}

		{
			value_type value;

			[&] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				([&]<std::size_t I>(std::in_place_index_t<I>)
				{
					auto& v0 = fox::reflexpr::get<Is>(std::as_const(value));
					auto v1 = value.template get<I>();
					EXPECT_EQ(v0, v1);
					EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(v0)>>);
				}(std::in_place_index<Is>), ...);
			}(std::make_index_sequence<value_type::member_count>{});
		}
	}

	TYPED_TEST_P(reflexpr_test, make_tuple)
	{
		using value_type = TypeParam;

		{
			value_type value;
			auto tuple = fox::reflexpr::tie(value);

			[&] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				([&]<std::size_t I>(std::in_place_index_t<I>)
				{
					auto& v0 = fox::reflexpr::get<Is>(value);
					auto v1 = value.template get<I>();
					EXPECT_TRUE(std::negation_v<std::is_const<std::remove_reference_t<decltype(v0)>>>);

					EXPECT_EQ(v0, v1);
					v0 *= 2;
					auto v3 = value.template get<I>();
					EXPECT_NE(v0, v1);
					EXPECT_NE(v1, v3);
					EXPECT_EQ(v0, v3);
				}(std::in_place_index<Is>), ...);
			}(std::make_index_sequence<value_type::member_count>{});
		}

		{
			value_type value;
			auto tuple = fox::reflexpr::tie(std::as_const(value));

			[&] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				([&]<std::size_t I>(std::in_place_index_t<I>)
				{
					auto& v0 = std::get<I>(tuple);
					auto v1 = value.template get<I>();
					EXPECT_EQ(v0, v1);
					EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(v0)>>)
						<< typeid(tuple).name();
				}(std::in_place_index<Is>), ...);
			}(std::make_index_sequence<value_type::member_count>{});
		}
	}

	TYPED_TEST_P(reflexpr_test, tie)
	{
		using value_type = TypeParam;

		using value_type = TypeParam;

		{
			value_type value;
			auto tuple = fox::reflexpr::make_tuple(value);
			using tuple_type = std::remove_cvref_t<decltype(tuple)>;
			[&] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				([&]<std::size_t I>(std::in_place_index_t<I>)
				{
					auto& v0 = std::get<Is>(tuple);
					auto v1 = value.template get<I>();
					EXPECT_TRUE(std::negation_v<std::is_const<std::remove_reference_t<decltype(v0)>>>);

					if
						constexpr (
							std::is_same_v<
							std::tuple_element_t<I, tuple_type>,
							decltype(std::declval<value_type>().template get<I>())
							> &&
							std::is_lvalue_reference_v<std::tuple_element_t<I, tuple_type>>
							)
					{
						EXPECT_EQ(v0, v1);
						v0 *= 2;
						auto v3 = value.template get<I>();
						EXPECT_NE(v0, v1);
						EXPECT_NE(v1, v3);
						EXPECT_EQ(v0, v3);
					}
				}(std::in_place_index<Is>), ...);
			}(std::make_index_sequence<value_type::member_count>{});
		}

		{
			value_type value;
			auto tuple = fox::reflexpr::make_tuple(std::as_const(value));
			using tuple_type = std::remove_reference_t<decltype(tuple)>;
			[&] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				([&]<std::size_t I>(std::in_place_index_t<I>)
				{
					auto& v0 = std::get<I>(tuple);
					auto v1 = value.template get<I>();
					EXPECT_EQ(v0, v1);
					if constexpr (std::is_lvalue_reference_v<std::tuple_element_t<I, tuple_type>>)
					{
						EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(v0)>>);
					}
				}(std::in_place_index<Is>), ...);
			}(std::make_index_sequence<value_type::member_count>{});
		}
	}

	struct test_aggregate_test
	{
		static constexpr std::size_t member_count = 2;

		template<std::size_t I>
		auto get() -> decltype(auto) requires (I < member_count)
		{
			if constexpr (I == 0)
				return a;
			if constexpr (I == 1)
				return b;
		}

		static inline int s_a = 1;
		int& a = s_a;
		int b = 2;
	};

	REGISTER_TYPED_TEST_SUITE_P(reflexpr_test, aggregate_concept, for_each, tuple_size, tuple_element, get, make_tuple, tie);
	INSTANTIATE_TYPED_TEST_SUITE_P(fundamental, reflexpr_test, types);
}