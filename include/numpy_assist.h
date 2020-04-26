#pragma once

#include <boost/python.hpp>
#include <exception>

#include "exceptions.h"


// check_buffer_type<T>(const Py_buffer &view)
//
// This function checks whether the view is consistent with the
// requested type, by checking view.format and view.itemsize.  Returns
// true if so, and false otherwise.

template <typename T>
static bool _check_buffer_helper(const Py_buffer &view, std::string opts) {
    bool code_ok = false;
    if (view.format == NULL || *view.format == 0)
        return false;
    for (auto c: opts)
        code_ok = code_ok || (view.format[0] == c);
    return code_ok && view.itemsize == sizeof(T);
}

template <typename T,
          typename std::enable_if<std::is_integral<T>::value,
                                  int>::type* = nullptr>
static bool check_buffer_type(const Py_buffer &view) {
    return _check_buffer_helper<T>(view, "bhilq");
}

template <typename T,
          typename std::enable_if<!std::is_integral<T>::value,
                                  int>::type* = nullptr>
static bool check_buffer_type(const Py_buffer &view) {
    return false;
}

template <>
bool check_buffer_type<float>(const Py_buffer &view) {
    return _check_buffer_helper<float>(view, "f");
}

template <>
bool check_buffer_type<double>(const Py_buffer &view) {
    return _check_buffer_helper<double>(view, "d");
}


// type_name<T>()
//
// This function returns a string description of type T, for reporting
// in exceptions.  The strings here should probably look like
// numpy.dtype names; i.e. "int32" not "int" or "int32_t".

template <typename T>
static std::string type_name() {
    return "unknown";
}

template <>
std::string type_name<int32_t>() {
    return "int32";
}

template <>
std::string type_name<int64_t>() {
    return "int64";
}

template <>
std::string type_name<float>() {
    return "float32";
}

template <>
std::string type_name<double>() {
    return "float64";
}


static std::string shape_string(std::vector<int> shape)
{
    std::ostringstream s;
    s << "(";
    for (int i=0; i<shape.size(); i++) {
        if (i > 0)
            s << ", ";
        if (shape[i] >= 0)
            s << shape[i];
        else if (shape[i] == -1)
            s << "*";
        else if (shape[i] == -2)
            s << "...->";
        else if (shape[i] == -3)
            s << "->...";
        else
            s << "!error";
    }
    s << ")";
    return s.str();
}

// class Py_buffer_wrapper
//
// Enriches Py_buffer with a destructor that calls PyBuffer_Release.
// Without this, raising exceptions after calling PyObject_GetBuffer
// could leak (large amounts of) memory.
//
// Also has private copy constructor so you can't duplicate references
// to the buffer.  Later, we carry these objects only in shared
// pointers and we are nicely protected from segfaults and so on.

class Py_buffer_wrapper : public Py_buffer {
public:
    Py_buffer_wrapper() : Py_buffer() {};
    ~Py_buffer_wrapper() {
        PyBuffer_Release(this);
    }
private:
    Py_buffer_wrapper(const Py_buffer_wrapper *);
    Py_buffer_wrapper& operator=(const Py_buffer_wrapper &);
};


// class BufferWrapper
//
// A container for a Py_buffer over a certain type.  The Py_buffer is
// reference counted so its safe to copy these objects.  Various
// constructors eliminate boilerplate when wrapping objects that must
// be of a certain shape / type.

template <typename T>
class BufferWrapper {
public:
    std::shared_ptr<Py_buffer_wrapper> view;

    BufferWrapper() {
        view = std::make_shared<Py_buffer_wrapper>();
    }

    // Constructor with no shape or type checking.
    BufferWrapper(std::string name, const bp::object &src, bool optional)
        : BufferWrapper() {
        if (PyObject_GetBuffer(src.ptr(), view.get(),
                               PyBUF_RECORDS) == -1) {
            PyErr_Clear();
            if (optional)
                return;
            throw buffer_exception(name);
        }
    }

    // Constructor with shape and type checking.
    BufferWrapper(std::string name, const bp::object &src, bool optional,
                  std::vector<int> shape)
        : BufferWrapper(name, src, optional) {

        // "optional" items will cause the parent constructor to
        // succeed, but will leave buffer pointer unset.
        if (view->buf == NULL)
            return;

        if (!check_buffer_type<T>(*view.get()))
            throw dtype_exception(name, type_name<T>());

        std::vector<int> vshape;
        for (int i=0; i<view->ndim; i++)
            vshape.push_back(view->shape[i]);

        // Note special values (-1,-2,-3) permit unknown, arbitrary
        // leading, and arbitrary trailing elements in buffer's shape.
        int i=0, j=0;
        while (i < shape.size() && j < vshape.size()) {
            if (shape[i] == -1) {
                // Match any single entry.
                j++;
            } else if (shape[i] == -2) {
                // Ignore 0 or more leading entries.
                j = vshape.size() - (shape.size() - i) + 1;
            } else if (shape[i] == -3) {
                // Ignore 0 or more trailing entries.
                j = vshape.size();
            } else if (shape[i] == vshape[j]) {
                // Matched exactly.
                j++;
            } else
                break;
            i++;
        }
        if (i != shape.size() || j != vshape.size()) {
            std::ostringstream s;
            s << "Expected " << shape_string(shape) << " but got " <<
                shape_string(vshape) << ".";
            throw shape_exception(name, s.str());
        }
    }        
};
