#include "../include/xjson.hpp"
#include <assert.h>

void json()
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
	try {
		obj["obj"]["child"]["int"].get<bool>();
	}
	catch (xjson::type_error &e)
	{
		std::cout << e.str() << std::endl;
	}
	std::cout << obj.str().c_str() << std::endl;

	xjson::obj_t *json2 = xjson::build(obj.str());

	assert(json2->str() == obj.str());
	xjson::destory(json2);
}
int main()
{
	json();
	return 0;
}