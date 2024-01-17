#include <iostream>
#include <string_view>
#include <algorithm>
#include <string>
#include <array>

#include <fox/reflexpr.hpp>

struct my_aggregate
{
	int a;
	float b;
	std::string c;
	int& d;
};

int main()
{
	int d = 5;
	my_aggregate obj
	{
		.a = 1 ,
		.b = 3.5f,
		.c = "Foxes are great!",
		.d = d
	};

	auto&& [v0, v1, v2, v3] = obj;

	// Get Nth member - fox::reflexpr::get<N>(aggregate)
	std::cout << fox::reflexpr::get<0>(obj) << '\n'; // prints obj.a 
	
	// Iterate over members - fox::reflexpr::for_each(aggregate, func)
	fox::reflexpr::for_each(obj, [](auto&& v) {std::cout << v << ' '; }), std::cout << '\n';

	// Create a tuple-tie from members - fox::reflexpr::tie(aggregate)
	auto tie = fox::reflexpr::tie(obj);
	std::cout << (std::get<2>(tie) = 2) << '\n';

	// Create a tuple from members - fox::reflexpr::make_tuple(aggregate)
	auto tuple = fox::reflexpr::make_tuple(obj);
	std::cout << (std::get<2>(tuple)) << '\n';

	// Tuple size - fox::reflexpr::tuple_size_v<aggregate_type>
	static_assert(fox::reflexpr::tuple_size_v<my_aggregate> == static_cast<std::size_t>(4));

	// Tuple Nth type
	static_assert(std::is_same_v<fox::reflexpr::tuple_element_t<3, my_aggregate>, int&>);

	return 0;
}