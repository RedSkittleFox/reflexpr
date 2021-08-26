#include <iostream>
#include <string_view>
#include <algorithm>
#include <string>

#include "ruby_reflexpr.hpp"

struct aggregate_type
{
	int a;
	float b;
	std::string str;
};

REFLECT(
struct aggregate_type_reflected
{
	int a;
	float b;
	std::string str;
}
);

struct functor
{
	template<class T>
	void operator()() const
	{
		std::cout << "Type: " << typeid(T).name() << '\n';
	};
};

struct functor_reflected
{
	template<class T>
	void operator()(const std::string& name) const
	{
		std::cout << "Name: " << name << " Type: " << typeid(T).name() << '\n';
	};
};

int main()
{
	// DEMO: reflexpr::for_each_member_variable
	{
		std::cout << "For each member variable:\n";
		auto func = []<class T>(T & v)
		{
			std::cout << "Type: " << typeid(T).name() << " Value: " << v << '\n';
		};

		aggregate_type at{ 1 , 3.5f, "Foxes are great!" };
		
		reflexpr::for_each_member_variable(at, func);
		std::cout << '\n';
	}

	// DEMO: reflexpr::for_each_member_type
	{
		std::cout << "For each member type:\n";

		reflexpr::for_each_member_type<aggregate_type, functor>(functor{});
		std::cout << '\n';
	}

	// DEMO: reflexpr::for_each_reflected_member_variable
	{
		std::cout << "For each member variable reflected:\n";
		auto func = []<class T>(T & v, const std::string& name)
		{
			std::cout << "Name: " << name << " Type: " << typeid(T).name() << " Value: " << v << '\n';
		};

		aggregate_type_reflected at{ 1 , 3.5f, "Foxes are great!" };

		reflexpr::for_each_reflected_member_variable(at, func);
		std::cout << '\n';
	}

	// DEMO: reflexpr::for_each_reflected_member_type
	{
		std::cout << "For each member type reflected:\n";

		reflexpr::for_each_reflected_member_type<aggregate_type_reflected, functor_reflected>(functor_reflected{});
		std::cout << '\n';
	}
	
	return 1;
}