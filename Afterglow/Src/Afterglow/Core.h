#pragma once

#ifdef AG_ENABLE_ASSERTS
	#define AG_ASSERT(x, ...) if(!(x)) { AG_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); }
#else
	#define AG_ASSERT(x, ...)
#endif