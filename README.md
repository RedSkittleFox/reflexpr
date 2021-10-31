# ruby-reflexpr
is a c++20 compile and runtime Struct Reflections header only library. It allows you to iterate over aggregate type's member variables.

# Example Usage
```cpp
// demo.cpp
#include <iostream>
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
	
	return 0;
}
```

# Planned Improvements
*  Increasing member variable limit to 30
*  Introducting `noreflect` specifier.
*  Improving member variable names parsing.

# Limitation
Right now it supports only up to 20 member variables and introduces small runtime overhead when registering member variable names.

# Extra Notice
This library is part of Ruby ecosystem created and managed by RedSkittleFox. Not all headers are avialable. This header is standalone.