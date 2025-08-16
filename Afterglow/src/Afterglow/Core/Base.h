#pragma once

#define AG_BIT(x) (1 << x)

#define AG_BIND_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#include "Log.h"
#include "Assert.h"