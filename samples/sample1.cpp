#include "../include/xjson.hpp"
#include <assert.h>


void vec()
{
	xjson::obj_t vec;
	vec.add("str");
	vec.add(1234);
	vec.add(false);
	auto str = vec.str();

	xjson::json_t *json = xjson::build_json(str);
	auto vec2 = json->vec();
	assert(vec.str() == vec2.str());
	xjson::destory_json(json);
}
void json()
{
	xjson::json_t json;
	json["int"] = 1;
	json["bool"] = false;
	json["float"] = 0.1;
	json["str"] = "xjson";
	json["vec"].add(true);
	json["vec"].add(true);
	json["vec"].add("xjson vec");
	json["obj"]["child"]["int"] = 1;

	std::cout << json.to_string().c_str() << std::endl;

	xjson::json_t *json2 = xjson::build_json(json.to_string());

	assert(json2->to_string() == json.to_string());
	xjson::destory_json(json2);
}
int main()
{
	vec();
	json();
	return 0;
}