[![CMake MSVC Build and Test](https://github.com/RedSkittleFox/reflexpr/actions/workflows/cmake-msvc-build.yml/badge.svg)](https://github.com/RedSkittleFox/reflexpr/actions/workflows/cmake-msvc-build.yml)

# reflexpr
is a c++20 compile and runtime aggregate reflections header only library. It allows you to iterate over aggregate type's member variables.

# Example Usage
```cpp
// demo.cpp
#include <iostream>
#include <string_view>
#include <algorithm>
#include <string>

#include <fox/reflexpr.hpp>

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
	}
};

struct functor_reflected
{
	template<class T>
	void operator()(const std::string& name) const
	{
		std::cout << "Name: " << name << " Type: " << typeid(T).name() << '\n';
	}
};

int main()
{
	// DEMO: fox::reflexpr::for_each_member_variable
	{
		std::cout << "For each member variable:\n";
		auto func = []<class T>(T & v)
		{
			std::cout << "Type: " << typeid(T).name() << " Value: " << v << '\n';
		};

		aggregate_type at{ 1 , 3.5f, "Foxes are great!" };

		static_assert(fox::reflexpr::aggregate<aggregate_type>, "sus");
		fox::reflexpr::for_each_member_variable(at, func);
		std::cout << '\n';
	}

	// DEMO: fox::reflexpr::for_each_member_type
	{
		std::cout << "For each member type:\n";

		fox::reflexpr::for_each_member_type<aggregate_type, functor>(functor{});
		std::cout << '\n';
	}

	// DEMO: fox::reflexpr::for_each_reflected_member_variable
	{
		std::cout << "For each member variable reflected:\n";
		auto func = []<class T>(T & v, const std::string& name)
		{
			std::cout << "Name: " << name << " Type: " << typeid(T).name() << " Value: " << v << '\n';
		};

		aggregate_type_reflected at{ 1 , 3.5f, "Foxes are great!" };

		fox::reflexpr::for_each_reflected_member_variable(at, func);
		std::cout << '\n';
	}

	// DEMO: fox::reflexpr::for_each_reflected_member_type
	{
		std::cout << "For each member type reflected:\n";

		fox::reflexpr::for_each_reflected_member_type<aggregate_type_reflected, functor_reflected>(functor_reflected{});
		std::cout << '\n';
	}
	
	return 1;
}
```

# Planned Improvements
*	Improving member variable names parsing.
*	Write unit tests.
	
# Limitation
Right now it supports only up to 40 member variables and introduces small runtime overhead when registering member variable names.
