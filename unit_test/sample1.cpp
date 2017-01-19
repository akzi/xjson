#include "../include/xjson.hpp"
#include "../../xtest/include/xtest.hpp"
#include <assert.h>

XTEST_SUITE(sample)
{

	XUNIT_TEST(build)
	{
		xjson::obj_t obj;
		obj["int"] = 1;

		obj["bool"] = false;
		obj["float"] = 0.1;
		obj["str"] = "xjson";
		obj["vec"].add(true);
		obj["vec"].add(true);
		obj["vec"].add("xjson vec");
		obj["obj"]["child"]["int"] = 1;

		auto json2 = xjson::build(obj.str());

		assert(json2.str() == obj.str());


		try 
		{
			json2["obj"]["child"]["int"].get<bool>();
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}

		auto ret = json2["obj"]["child"]["int"].get<xjson::optional<bool>>();

		//no exist
		xassert(!ret);

		std::cout << obj.str().c_str() << std::endl;

		
	}
}

