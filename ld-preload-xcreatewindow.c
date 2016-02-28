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
   if (!lib_xlib) lib_xlib = dlopen("libX11.so", RTLD_GLOBAL | RTLD_LAZY);
   if (!func) func = dlsym (RTLD_NEXT, "XCreateWindow");
   if (!func) func = dlsym (lib_xlib, "XCreateWindow");
   if (func == XCreateWindow) {
	   fprintf (stderr, "FATAL: dlsym ('XCreateWindow') somehow points at us.\n");
	   exit (1);
   } else if (! func) {
	   fprintf (stderr, "FATAL: dlsym ('XCreateWindow') somehow points at NULL.\n");
	   exit (1);
   }

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
   if (!lib_xlib) lib_xlib = dlopen("libX11.so", RTLD_GLOBAL | RTLD_LAZY);
   if (!func) func = dlsym (RTLD_NEXT, "XCreateSimpleWindow");
   if (!func) func = dlsym (lib_xlib, "XCreateSimpleWindow");
   if (func == XCreateSimpleWindow) {
	   fprintf (stderr, "FATAL: dlsym ('XCreateSimpleWindow') somehow points at us.\n");
	   exit (1);
   } else if (! func) {
	   fprintf (stderr, "FATAL: dlsym ('XCreateSimpleWindow') somehow points at NULL.\n");
	   exit (1);
   }

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
   if (!lib_xlib) lib_xlib = dlopen("libX11.so", RTLD_GLOBAL | RTLD_LAZY);
   if (!func) func = dlsym (RTLD_NEXT, "XReparentWindow");
   if (!func) func = dlsym (lib_xlib, "XReparentWindow");
   if (func == XReparentWindow) {
	   fprintf (stderr, "FATAL: dlsym ('XReparentWindow') somehow points at us.\n");
	   exit (1);
   } else if (! func) {
	   fprintf (stderr, "FATAL: dlsym ('XReparentWindow') somehow points at NULL.\n");
	   exit (1);
   }

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
