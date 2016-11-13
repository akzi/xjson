#pragma once
#include <map>
#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#pragma once

namespace xjson
{
	
	class type_error: public std::exception
	{
	public:
		type_error(const char* file,int line, const char* errstr)
		{
			errstr_.append("FILE: ");
			errstr_.append(file);
			errstr_.append("  LINE: ");
			errstr_.append(std::to_string(line));
			errstr_.append(" xjson_assert ( ");
			errstr_.append(errstr);
			errstr_.append(" ) failed");
		}
		const char *str()
		{
			return errstr_.c_str();
		}
	private:
		std::string errstr_;
	};

	#define xjson_assert(x) if(!(x)) throw type_error(__FILE__, __LINE__, #x);

	struct obj_t
	{
		enum type_t
		{
			e_null,
			e_num,
			e_str,
			e_bool,
			e_float,
			e_obj,
			e_vec,
		};

		type_t type_;
		union value_t
		{
			std::string *str_;
			int64_t num_;
			double double_;
			bool bool_;
			std::map<std::string, obj_t *> *obj_;
			std::vector<obj_t*> *vec_;
		} val_;

		struct null 
		{
		};
		obj_t()
			:type_(e_null)
		{
		}
// 		template<typename T>
// 		obj_t(T &&val)
// 		{
// 			operator=(std::forward<T>(val));
// 		}
		obj_t(obj_t && self)
		{
			type_ = self.type_;
			val_ = self.val_;
			self.type_ = e_null;
		}
		obj_t &operator = (obj_t && self)
		{
			if(this != &self)
			{
				reset();
				type_ = self.type_;
				val_ = self.val_;
				self.type_ = e_null;
			}
			return *this;
		}
		template<typename T>
		typename std::enable_if<std::is_class<T>::value, obj_t &>::type
		operator = (const T &o)
		{
			o.pack_xjson(*this);
			return *this;
		}

		virtual ~obj_t()
		{
			reset();
		}
		void reset()
		{
			switch (type_)
			{
			case e_null:
			case e_num:
			case e_bool:
			case e_float:
				break;
			case e_str:
				delete val_.str_;
				break;
			case e_obj:
				for (auto&itr : *val_.obj_)
					delete itr.second;
				delete val_.obj_;
				break;
			case e_vec:
				for (auto&itr : *val_.vec_)
					delete itr;
				delete val_.vec_;
				break;
			default:
				break;
			}
			type_ = e_null;
		}
		
		template<typename T>
		typename std::enable_if<std::is_floating_point<T>::value, obj_t &>::type
			operator=(T val)
		{
			reset();
			type_ = e_float;
			val_.double_ = val;
			return *this;
		}
		obj_t &operator=(nullptr_t)
		{
			reset();
			return *this;
		}
		obj_t &operator=(const char *val)
		{
			reset();
			type_ = e_str;
			val_.str_ = new std::string(val);
			return *this;
		}

		obj_t &operator =(const std::string &val)
		{
			reset();
			type_ = e_str;
			val_.str_ = new std::string(val);
			return *this;
		}
		template<typename T>
		typename std::enable_if<std::is_integral<T>::value, obj_t &>::type
			operator=(T val)
		{
			reset();
			type_ = e_num;
			val_.num_ = val;
			return *this;
		}

		obj_t &operator =(bool val)
		{
			reset();
			type_ = e_bool;
			val_.bool_ = val;
			return *this;
		}
		obj_t &operator[](const std::string & key)
		{
			return operator[](key.c_str());
		}

		obj_t &operator[](const char *key)
		{
			if (type_ != e_obj)
			{
				reset();
				type_ = e_obj;
				val_.obj_ = new std::map<std::string, obj_t *>;
			}
			auto itr = val_.obj_->find(key);
			if (itr != val_.obj_->end())
				return *itr->second;
			obj_t *o = new obj_t;
			val_.obj_->emplace(key, o);
			return *o;
		}
		obj_t &make_vec()
		{
			if(type_ != e_vec)
			{
				reset();
				type_ = e_vec;
				val_.vec_ = new std::vector<obj_t *>;
			}
			return *this;
		}
		template<typename T>
		obj_t& add(const T &val)
		{
			if (type_ != e_vec)
			{
				reset();
				type_ = e_vec;
				val_.vec_ = new std::vector<obj_t *>;
			}
			obj_t *obj = new obj_t;
			*obj = val;
			val_.vec_->push_back(obj);
			return *this;
		}
		obj_t& add(obj_t *obj)
		{
			if (type_ != e_vec)
			{
				reset();
				type_ = e_vec;
				val_.vec_ = new std::vector<obj_t *>;
			}
			val_.vec_->push_back(obj);
			return *this;
		}
		obj_t &add(nullptr_t )
		{
			if (type_ != e_vec)
			{
				reset();
				type_ = e_vec;
				val_.vec_ = new std::vector<obj_t *>;
			}
			
			val_.vec_->push_back(new obj_t);
			return *this;
		}
		template<class T>
		typename std::enable_if<std::is_integral<T>::value &&
			!std::is_same<T, bool>::value, T>::type
			get() const
		{
			xjson_assert(type_ == e_num);
			return static_cast<T>(val_.num_);
		}
		template<class T>
		typename std::enable_if<std::is_same<T, bool>::value, T>::type
			get() const
		{
			xjson_assert(type_ == e_bool);
			return val_.bool_;
		}
		template<class T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
			get() const
		{
			xjson_assert(type_ == e_float);
			return static_cast<T>(val_.double_);
		}
		template<class T>
		typename std::enable_if<std::is_same<T, std::string>::value, std::string &>::type
			get() const
		{
			xjson_assert(type_ == e_str);
			return *val_.str_;
		}
		template<typename T>
		T get(std::size_t idx) const
		{
			xjson_assert(type_ == e_vec);
			xjson_assert(val_.vec_);
			xjson_assert(idx < val_.vec_->size());
			return ((*val_.vec_)[idx])->get<T>();
		}
		obj_t &get(std::size_t idx) const 
		{
			xjson_assert(type_ == e_vec);
			xjson_assert(val_.vec_);
			xjson_assert(idx < val_.vec_->size());
			return *((*val_.vec_)[idx]);
		}
		bool is_null() const
		{
			return type_ == e_null;
		}
		std::size_t size() const 
		{
			xjson_assert(type_ == e_vec);
			return val_.vec_->size();
		}
		type_t type() const
		{
			return type_;
		}
		std::string str()const
		{
			switch (type_)
			{
			case e_bool:
				return val_.bool_ ? "true" : "false";
			case e_num:
				return std::to_string(val_.num_);
			case e_str:
				return "\"" + *val_.str_ + "\"";
			case e_obj:
				do 
				{
					std::string str("{");
					for (auto &itr : *val_.obj_)
					{
						str += "\"" + itr.first + "\"";
						str += " : ";
						str += itr.second->str();
						str += ", ";
					}
					str.pop_back();
					str.pop_back();
					str += "}";
					return str;

				} while (0);
			case e_float:
				return std::to_string(val_.double_);
			case e_vec:
				do 
				{
					std::string str("[");
					xjson_assert(type_ == e_vec);
					xjson_assert(val_.vec_ != NULL);

					for (auto &itr : *val_.vec_)
					{
						str += itr->str();
						str += ", ";
					}
					str.pop_back();
					str.pop_back();
					str += "]";
					return str;
				} while (0);
			}
			return "null";
		}
	};
	
	namespace json_parser
	{
		static inline bool
			skip_space(int &pos, int len, const char * str)
		{
			assert(len > 0);
			assert(pos >= 0);
			assert(pos < len);

			while (pos < len &&
				(str[pos] == ' ' ||
					str[pos] == '\r' ||
					str[pos] == '\n' ||
					str[pos] == '\t'))
				++pos;

			return pos < len;
		}
		static inline std::pair<bool, std::string>
			get_text(int &pos, int len, const char *str)
		{
			assert(len > 0);
			assert(pos >= 0);
			assert(pos < len);
			assert(str[pos] == '"');

			std::string key;
			++pos;
			while (pos < len)
			{
				if (str[pos] != '"')
					key.push_back(str[pos]);
				else if (key.size() && key.back() == '\\')
					key.push_back(str[pos]);
				else
				{
					++pos;
					return{ true, key };
				}
				++pos;
			}
			return{ false,"" };
		}
		static int get_bool(int &pos, int len, const char *str)
		{
			assert(len > 0);
			assert(pos >= 0);
			assert(pos < len);
			assert(str[pos] == 't' || str[pos] == 'f');
			std::string key;

			while (pos < len)
			{
				if (isalpha(str[pos]))
					key.push_back(str[pos]);
				else
					break;
				++pos;
			}
			if (key == "true")
				return 1;
			else if (key == "false")
				return 0;
			return -1;
		}
		static inline bool get_null(int &pos, int len, const char *str)
		{
			assert(len > 0);
			assert(pos >= 0);
			assert(pos < len);
			assert(str[pos] == 'n');
			std::string null;

			while (pos < len)
			{
				if (isalpha(str[pos]))
					null.push_back(str[pos]);
				else
					break;
				++pos;
			}
			if (null == "null")
				return true;
			return false;
		}
		static inline std::string get_num(
			bool &sym, int &pos, int len, const char *str)
		{
			sym = false;
			std::string tmp;
			while (pos < len)
			{
				if (str[pos] >= '0' &&str[pos] <= '9')
				{
					tmp.push_back(str[pos]);
					++pos;
				}
				else if (str[pos] == '.')
				{
					tmp.push_back(str[pos]);
					++pos;
					if (sym == false)
						sym = true;
					else
						return false;
				}
				else
					break;
			}
			return tmp;
		}

		static inline obj_t *get_obj(int &pos, int len, const char * str);
		static inline obj_t* get_vec(int &pos, int len, const char *str)
		{
			obj_t *vec = new obj_t;
			if (str[pos] == '[')
				pos++;
			while (pos < len)
			{
				switch (str[pos])
				{
				case ']':
					++pos;
					return vec;
				case '[':
				{
					obj_t * obj = get_vec(pos, len, str);
					if (!!!obj)
						goto fail;
					vec->add(obj);
					break;
				}
				case '"':
				{
					std::pair<bool, std::string > res = get_text(pos, len, str);
					if (res.first == false)
						goto fail;
					vec->add(res.second);
					break;
				}
				case 'n':
					if (get_null(pos, len, str))
					{
						vec->add(nullptr);
						break;
					}
				case '{':
				{
					obj_t *o = get_obj(pos, len, str);
					if (!o)
						goto fail;
					vec->add(o);
					break;
				}
				case 'f':
				case 't':
				{
					int b = get_bool(pos, len, str);
					if (b == 0)
						vec->add(false);
					else if (b == 1)
						vec->add(true);
					else
						goto fail;
					break;
				}
				case ',':
				case ' ':
				case '\r':
				case '\n':
				case '\t':
					++pos;
					break;
				default:
					if (str[pos] == '-' || str[pos] >= '0' && str[pos] <= '9')
					{
						bool is_float = false;
						std::string tmp = get_num(is_float, pos, len, str);
						errno = 0;
						if (is_float)
						{
							double d = std::strtod(tmp.c_str(), 0);
							if (errno == ERANGE)
								goto fail;
							vec->add(d);
						}
						else
						{
							int64_t val = std::strtoll(tmp.c_str(), 0, 10);
							if (errno == ERANGE)
								goto fail;
							vec->add(val);
						}
					}
				}
			}

		fail:
			delete vec;
			return NULL;
		}

		#define check_ahead(ch)\
		do{\
				if(!skip_space(pos, len, str))\
					goto fail; \
					if(str[pos] != ch)\
						goto fail;\
		} while(0)

		static inline obj_t *get_obj(int &pos, int len, const char * str)
		{
			obj_t *obj_ptr = new obj_t;
			obj_t &json = *obj_ptr;
			std::string key;

			skip_space(pos, len, str);
			if (pos >= len)
				goto fail;
			if (str[pos] == '{')
				++pos;
			while (pos < len)
			{
				switch (str[pos])
				{
				case '"':
				{
					std::pair<bool, std::string > res = get_text(pos, len, str);
					if (res.first == false)
						goto fail;
					if (key.empty())
					{
						key = res.second;
						check_ahead(':');
					}
					else
					{
						json[key] = res.second;
						key.clear();
					}
					break;
				}
				case 'f':
				case 't':
				{
					if (key.empty())
						goto fail;
					int b = get_bool(pos, len, str);
					if (b == 0)
						json[key] = false;
					else if (b == 1)
						json[key] = true;
					else
						goto fail;
					key.clear();
					break;
				}
				case 'n':
				{
					if (key.empty() || get_null(pos, len, str) == false)
						goto fail;
					json[key] = nullptr;
				}
				case '{':
				{
					if (key.empty())
						goto fail;
					obj_t *o = get_obj(pos, len, str);
					if (o == NULL)
						goto fail;
					json[key] = std::move(*o);
					key.clear();
					break;
				}
				case '}':
					if (key.size())
						goto fail;
					++pos;
					return obj_ptr;
				case ':':
				{
					if (key.empty())
						goto fail;
					++pos;
					break;
				}
				case ',':
				{
					++pos;
					check_ahead('"');
					break;
				}
				case ' ':
				case '\r':
				case '\n':
				case '\t':
					++pos;
					break;
				case '[':
				{
					obj_t *vec = get_vec(pos, len, str);
					if (!!!vec || key.empty())
						goto fail;
					json[key] = std::move(*vec);
					key.clear();
					break;
				}
				default:
					if (str[pos] == '-' || str[pos] >= '0' && str[pos] <= '9')
					{
						bool is_float = false;
						std::string tmp = get_num(is_float, pos, len, str);
						errno = 0;
						if (is_float)
						{
							double d = std::strtod(tmp.c_str(), 0);
							if (errno == ERANGE)
								goto fail;
							json[key] = d;
							key.clear();
						}
						else
						{
							int64_t val = std::strtoll(tmp.c_str(), 0, 10);
							if (errno == ERANGE)
								goto fail;
							json[key] = val;
							key.clear();
						}
					}
				}
			}

		fail:
			delete obj_ptr;
			return NULL;
		}
	}
	inline obj_t *build(const std::string &str)
	{
		int pos = 0;
		return json_parser::get_obj(pos, (int)str.size(), str.c_str());
	}
	inline obj_t *build(const char *str)
	{
		int pos = 0;
		return json_parser::get_obj(pos, (int)strlen(str), str);
	}
	inline void destory(obj_t *json)
	{
		delete json;
	}
}