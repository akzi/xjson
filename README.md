# xjson
c++11 json. small and simple

```cpp

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
```
