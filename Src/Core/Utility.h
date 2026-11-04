#if !defined(AG_UTILITY_H)
#define AG_UTILITY_H

#define AG_ARRAYCOUNT(array) (sizeof(array) / sizeof((array)[0]))

#define AG_KIB(count) ((count) * 1024ULL)
#define AG_MIB(count) ((count) * 1024ULL * 1024ULL)
#define AG_GIB(count) ((count) * 1024ULL * 1024ULL * 1024ULL)

#define AG_ALIGNUP(address, alignmentBytes)                                                        \
	((((usize)(address)) + ((usize)(alignmentBytes)) - 1) & (~(((usize)(alignmentBytes)) - 1)))
#define AG_ALIGNDOWN(address, alignmentBytes)                                                      \
	((((usize)(address))) & (~(((usize)(alignmentBytes)) - 1)))

#endif