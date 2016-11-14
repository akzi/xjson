#include "../../xtest/include/xtest.hpp"
#include "../include/xjson.hpp"
xtest_run;

using namespace xjson;
XTEST_SUITE(xjson)
{
	XUNIT_TEST(get)
	{
		obj_t o(1);
		xassert(o.get<int>() == 1);
	}

	struct user
	{
		int id;
		std::string name;
		void xpack(obj_t &o)const
		{
			o["id"] = id;
			o["name"] = name;
		}
		void xunpack(obj_t &o)
		{
			id = o["id"].get<decltype(id)>();
			name = o["name"].get<decltype(name)>();
		}
	};


	XUNIT_TEST(get_struct)
	{
		user u{ 1,"u1" };
		
		obj_t o;
		o["user"] = u;

		xassert(o["user"]["id"].get<int>() == 1);
		xassert(o["user"]["name"].get<std::string>() == "u1");

		auto u2 = o["user"].get<user>();

		xassert(u2.id == u.id);
		xassert(u2.name == u.name);
	}
}