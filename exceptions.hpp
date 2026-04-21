#ifndef SJTU_EXCEPTIONS_HPP
#define SJTU_EXCEPTIONS_HPP

#include <cstddef>
#include <cstring>
#include <string>

namespace sjtu {

class exception {
protected:
	const std::string variant = "";
	std::string detail = "";
public:
	exception() {}
	exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
	virtual std::string what() {
		return variant + " " + detail;
	}
};

/**
 * TODO
 * Please complete them.
 */
class index_out_of_bound : public exception {
public:
	index_out_of_bound() {}
	index_out_of_bound(const std::string &msg) {
		detail = msg;
	}
};

class runtime_error : public exception {
public:
	runtime_error() {}
	runtime_error(const std::string &msg) {
		detail = msg;
	}
};

class invalid_iterator : public exception {
public:
	invalid_iterator() {}
	invalid_iterator(const std::string &msg) {
		detail = msg;
	}
};

class container_is_empty : public exception {
public:
	container_is_empty() {}
	container_is_empty(const std::string &msg) {
		detail = msg;
	}
};
}

#endif
