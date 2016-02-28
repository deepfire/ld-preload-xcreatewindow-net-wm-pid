// gcc ld-preload-xcreatewindow.c -shared -fPIC -o ld-preload-xcreatewindow.so
// LD_PRELOAD=/path/to/ld-preload-xcreatewindow.so
//
//  Copyright(c) 2004, The Rasterman
//  Copyright(c) 2016, Kosyrev Serge
//
//  Based on extraction from:
//   http://www.mail-archive.com/devel@xfree86.org/msg05806.html

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>

/* prototypes */
static void __e_hack_set_properties(Display *display, Window window);

/* dlopened xlib so we can find the symbols in the real xlib to call them */
static void *lib_xlib = NULL;

/* the function that actually sets the properties on toplevel window */
static void
__e_hack_set_properties(Display *display, Window window)
{
   static Atom net_wm_pid = 0;

   if (!net_wm_pid)    net_wm_pid    = XInternAtom(display, "_NET_WM_PID", False);

     {
	pid_t pid;
	char buf[4096];

	pid = getpid();

	snprintf(buf, sizeof(buf), "%i", pid);
	XChangeProperty(display, window, net_wm_pid, XA_CARDINAL, 32, PropModeReplace, (void *) &pid, 1);
	fprintf (stderr, "-- trapped window creation: window=%d, pid=%d\n", window, pid);
     }
}

#define get_sym(func_var, lib_handle_var, soname, fname) {		\
   if (!lib_handle_var)							\
	   lib_handle_var = dlopen(soname, RTLD_GLOBAL | RTLD_LAZY);	\
   if (!func_var)							\
	   func_var = dlsym (RTLD_NEXT, #fname);			\
   if (!func_var) {							\
	   fprintf (stderr, "WARNING: dlsym (RTLD_NEXT, '" #fname "') failed: %s, retrying with '" soname "'.\n", dlerror ()); \
	   func_var = dlsym (lib_handle_var, #fname);			\
   }									\
   if (func_var == fname) {						\
	   fprintf (stderr, "FATAL: dlsym ('" #fname "') somehow points at us.\n"); \
	   exit (1);							\
   } else if (! func_var) {						\
	   fprintf (stderr, "FATAL: dlsym ('" soname "', '" #fname "') failed: %s.\n", dlerror ()); \
	   exit (1);							\
   }									\
}

/* XCreateWindow intercept hack */
Window
XCreateWindow(
	      Display *display,
	      Window parent,
	      int x, int y,
	      unsigned int width, unsigned int height,
	      unsigned int border_width,
	      int depth,
	      unsigned int class,
	      Visual *visual,
	      unsigned long valuemask,
	      XSetWindowAttributes *attributes
	      )
{
   static Window (*func)
      (
       Display *display,
       Window parent,
       int x, int y,
       unsigned int width, unsigned int height,
       unsigned int border_width,
       int depth,
       unsigned int class,
       Visual *visual,
       unsigned long valuemask,
       XSetWindowAttributes *attributes
       ) = NULL;
   int i;

   /* find the real Xlib and the real X function */
   get_sym (func, lib_xlib, "libX11.so", XCreateWindow);

   /* multihead screen handling loop */
   for (i = 0; i < ScreenCount(display); i++)
     {
	/* if the window is created as a toplevel window */
	if (parent == RootWindow(display, i))
	  {
	     Window window;

	     /* create it */
	     window = (*func) (display, parent, x, y, width, height,
				border_width, depth, class, visual, valuemask,
				attributes);
	     /* set properties */
	     __e_hack_set_properties(display, window);
	     /* return it */
	     return window;
	  }
     }
   /* normal child window - create as usual */
   return (*func) (display, parent, x, y, width, height, border_width, depth,
		   class, visual, valuemask, attributes);
}

/* XCreateSimpleWindow intercept hack */
Window
XCreateSimpleWindow(
		    Display *display,
		    Window parent,
		    int x, int y,
		    unsigned int width, unsigned int height,
		    unsigned int border_width,
		    unsigned long border,
		    unsigned long background
		    )
{
   static Window (*func)
      (
       Display *display,
       Window parent,
       int x, int y,
       unsigned int width, unsigned int height,
       unsigned int border_width,
       unsigned long border,
       unsigned long background
       ) = NULL;
   int i;

   /* find the real Xlib and the real X function */
   get_sym (func, lib_xlib, "libX11.so", XCreateSimpleWindow);

   /* multihead screen handling loop */
   for (i = 0; i < ScreenCount(display); i++)
     {
	/* if the window is created as a toplevel window */
	if (parent == RootWindow(display, i))
	  {
	     Window window;

	     /* create it */
	     window = (*func) (display, parent, x, y, width, height,
				border_width, border, background);
	     /* set properties */
	     __e_hack_set_properties(display, window);
	     /* return it */
	     return window;
	  }
     }
   /* normal child window - create as usual */
   return (*func) (display, parent, x, y, width, height,
		   border_width, border, background);
}

/* XReparentWindow intercept hack */
int
XReparentWindow(
		Display *display,
		Window window,
		Window parent,
		int x, int y
		)
{
   static int (*func)
      (
       Display *display,
       Window window,
       Window parent,
       int x, int y
       ) = NULL;
   int i;

   /* find the real Xlib and the real X function */
   get_sym (func, lib_xlib, "libX11.so", XReparentWindow);

   /* multihead screen handling loop */
   for (i = 0; i < ScreenCount(display); i++)
     {
	/* if the window is created as a toplevel window */
	if (parent == RootWindow(display, i))
	  {
	     /* set properties */
	     __e_hack_set_properties(display, window);
	     /* reparent it */
	     return (*func) (display, window, parent, x, y);
	  }
     }
   /* normal child window reparenting - reparent as usual */
   return (*func) (display, window, parent, x, y);
}
