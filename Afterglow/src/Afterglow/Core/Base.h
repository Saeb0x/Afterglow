#pragma once

#define agBIT(x) (1 << x)

#define agBIND_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#include "Log.h"
#include "Assert.h"