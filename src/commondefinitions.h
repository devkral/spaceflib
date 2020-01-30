#ifndef spacefconfig
#define spacefconfig

// for only disabling expensive validations
// that are:
//   simple loader, is node counter node
//#define NO_EXPENSIVE_VALIDATIONS 1
//#define NDEBUG 1
#ifdef __cplusplus
#include <cassert>
#include <cinttypes>
#include <iterator>
typedef std::iterator<std::input_iterator_tag, uint64_t> uint64_iterator;
typedef std::iterator_traits<uint64_t> uint64_iterator_trait;
// are exceptions possible
#ifdef __cpp_exceptions
#include <stdexcept>
#else
#include <cstdio>
#endif

#else
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#endif

#ifndef SFL_ID_SIZE
    typedef uint64_t SFL_ID_SIZE;
#endif
#ifndef SFL_POS_SIZE
    typedef uint64_t SFL_POS_SIZE;
#endif
//const static uint8_t SFL_ERROR=0;

// SFLCHECK should be used for arbitary input. NDEBUG does not disable it
// it uses exceptions for c++ with exceptions elsewise fprintf and exit
#if defined(__cplusplus) && defined(__cpp_exceptions)

#define STR1__(x) #x
#define STR2__(x) STR1__(x)
#define LOCATION __FILE__ " : " STR2__(__LINE__) ": "
#define SFLCHECK(cob) if (!(cob)){throw std::logic_error(LOCATION "check failed: " #cob);}
#else
#define SFLCHECK(cob) if (!(cob)){fprintf(stderr, "%s: %s: check failed: %s\n", __FILE__, __LINE__, #cob);exit(1);}
#endif


#endif
