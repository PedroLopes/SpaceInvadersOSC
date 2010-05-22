#ifndef CONFIG_H
#define CONFIG_H
@TOP@

/* the name of package */
#undef PACKAGE

/* the version of package */
#undef VERSION

/* Define to 1 if NLS is requested.  */
#undef ENABLE_NLS

/* Define as 1 if you have catgets and don't want to use GNU gettext.  */
#undef HAVE_CATGETS

/* Define as 1 if you have gettext and don't want to use GNU gettext.  */
#undef HAVE_GETTEXT

/* Define if your locale.h file contains LC_MESSAGES.  */
#undef HAVE_LC_MESSAGES

/* Define as 1 if you have the stpcpy function.  */
#undef HAVE_STPCPY

@BOTTOM@

#ifndef HAVE_SETLOCALE
# define setlocale(Category, Locale)
#endif

#if ENABLE_NLS
# ifndef HAVE_LIBINTL_H
#  define HAVE_LIBINTL_H 1
# endif
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define bindtextdomain(Domain, Directory)
# define textdomain(Domain)
# define _(Text) Text
# define gettext(z) z
#endif
#define N_(Text) Text

#endif /* CONFIG_H */
