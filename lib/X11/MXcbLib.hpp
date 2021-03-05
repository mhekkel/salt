//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <xcb/xcb.h>
#include <xcb/render.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>

#include "MLib.hpp"

#include "MTypes.hpp"

// X11 utils

uint32_t MapModifier(uint32_t inModifier);
uint32_t MapKeyCode(uint32_t inKeyValue);
