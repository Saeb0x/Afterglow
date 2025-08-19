#pragma once

#include "Log.h"
#include "Assert.h"

#define AG_BIT(x) (1 << x)
#define AG_BIND_FUNC(func) std::bind(&func, this, std::placeholders::_1)