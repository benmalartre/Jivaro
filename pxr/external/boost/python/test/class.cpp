//
// Copyright 2024 Pixar
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include "pxr/external/boost/python/module.hpp"
#include "pxr/external/boost/python/def.hpp"
#include "pxr/external/boost/python/object.hpp"
#include "pxr/external/boost/python/class.hpp"

using namespace PXR_BOOST_NAMESPACE::python;

struct X
{
    int x;
    X(int n) : x(n) { }
};

int x_function(X& x)
{   return x.x;
}


PXR_BOOST_PYTHON_MODULE(class_ext)
{
    class_<X>("X", init<int>());
    def("x_function", x_function);
}

#include "module_tail.cpp"
