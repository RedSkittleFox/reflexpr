import os
import sys

num_supported_members : int = 40
out = ""

for i in range(1, num_supported_members):
	out_class = """
struct test_aggregate_%d
{
	static constexpr std::size_t member_count = %d;
	
	template<std::size_t I>
	auto get() -> decltype(auto) requires (I < member_count)
	{
""" % (i, i)
    
	for j in range(1, i + 1):
		out_class = out_class + """
		if constexpr (I == %d)
			return v%d;
""" % (j - 1, j)
		
	out_class = out_class + """
	}
	"""
	
	for j in range(1, i + 1):
		if j % 2 == 0:
			out_class = out_class + """
		int v%d = %d;""" % (j, j)
		else:
			out_class = out_class + """
		static inline int s_v%d = %d;
		int& v%d = s_v%d;""" % (j, j, j, j)

	out_class = out_class + """
};
"""
	out = out + out_class
	
out = out + f"""
using types = testing::Types<
	"""
	
for i in range(1, num_supported_members):
	out = out + f"""test_aggregate_{i}"""
	if i + 1 != num_supported_members:
		out += ", "

out = out + """>;"""


f = open(f"{sys.argv[1]}/reflexpr_test_types.inl", "w")
f.write(out)
f.close()