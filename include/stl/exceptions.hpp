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
    exception(const std::string& var = "") : variant(var) {
    }
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {
    }
    virtual std::string what() {
        return variant + " " + detail;
    }
};

class index_out_of_bound : public exception {
    /* __________________________ */
public:
    index_out_of_bound(const std::string& det = "") : exception("IndexOutOfBound") {
        exception::detail = det;
    }
};

class runtime_error : public exception {
    /* __________________________ */
public:
    runtime_error(const std::string& det = "") : exception("RuntimeError") {
        exception::detail = det;
    }
};

class invalid_iterator : public exception {
    /* __________________________ */
public:
    invalid_iterator(const std::string& det = "") : exception("InvalidIterator") {
        exception::detail = det;
    }
};

class container_is_empty : public exception {
    /* __________________________ */
public:
    container_is_empty(const std::string& det = "") : exception("ContainerIsEmpty") {
        exception::detail = det;
    }
};
}  // namespace sjtu

#endif
