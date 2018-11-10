#ifndef SU_UTILS_RUBYLIB_H_
#define SU_UTILS_RUBYLIB_H_

#if defined(_MSC_VER)
# if _MSC_VER >= 1900
// Why the x64-mswin64_140/ruby/config.h doesn't have these macros defined?
#define HAVE_ACOSH
#define HAVE_ROUND
#define HAVE_ERF
#define HAVE_TGAMMA
#define HAVE_CBRT
#define HAVE_NEXTAFTER
#endif
#endif

#include <ruby.h>
#ifdef HAVE_RUBY_ENCODING_H
#include <ruby/encoding.h>
#endif

#endif // SU_UTILS_RUBYLIB_H_
