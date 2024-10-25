//
// Copyright 2024 Pixar
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef PXR_EXTERNAL_BOOST_PYTHON_CLASS_FWD_HPP
# define PXR_EXTERNAL_BOOST_PYTHON_CLASS_FWD_HPP

#include "pxr/pxr.h"
#include "pxr/external/boost/python/common.hpp"

#ifndef PXR_USE_INTERNAL_BOOST_PYTHON
#include <boost/python/class_fwd.hpp>
#else

# include "pxr/external/boost/python/detail/prefix.hpp"
# include "pxr/external/boost/python/detail/not_specified.hpp"

namespace PXR_BOOST_NAMESPACE { namespace python { 

template <
    class T // class being wrapped
    // arbitrarily-ordered optional arguments. Full qualification needed for MSVC6
    , class X1 = ::PXR_BOOST_NAMESPACE::python::detail::not_specified
    , class X2 = ::PXR_BOOST_NAMESPACE::python::detail::not_specified
    , class X3 = ::PXR_BOOST_NAMESPACE::python::detail::not_specified
    >
class class_;

}} // namespace PXR_BOOST_NAMESPACE::python

#endif // PXR_USE_INTERNAL_BOOST_PYTHON
#endif // PXR_EXTERNAL_BOOST_PYTHON_CLASS_FWD_HPP
