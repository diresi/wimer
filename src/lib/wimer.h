#ifndef _LIBWIMER_H_
#define _LIBWIMER_H_

#ifdef _LIBWIMER_H_SHARED_
#include "libwimer_export.h"
#else
#define LIBWIMER_EXPORT
#endif // _LIBWIMER_H_SHARED_

LIBWIMER_EXPORT int CALLBACK libwimer_main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);

#endif // _LIBWIMER_H_
