/* Forward validated triplets to the X root property; do not touch WM_NAME. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
int main(int argc,char **argv) {
    Display *d; Window root; Atom atom; char line[128],hex[8],tail; int r,g,b;
    if(argc>2 || (argc==2 && strcmp(argv[1],"-c"))) {fprintf(stderr,"usage: rgbx [-c]\n");return 2;}
    if(!(d=XOpenDisplay(NULL))) {fprintf(stderr,"rgbx: cannot open DISPLAY\n");return 1;}
    root=DefaultRootWindow(d); atom=XInternAtom(d,"_SUCKLESS_RGB",False);
    if(argc==2) {XDeleteProperty(d,root,atom);XSync(d,False);XCloseDisplay(d);return 0;}
    while(fgets(line,sizeof line,stdin)) {
        if(sscanf(line," %d %d %d %c",&r,&g,&b,&tail)!=3 || r<0||r>255||g<0||g>255||b<0||b>255) continue;
        snprintf(hex,sizeof hex,"#%02X%02X%02X",r,g,b);
        XChangeProperty(d,root,atom,XA_STRING,8,PropModeReplace,(unsigned char*)hex,7);
        XFlush(d);
    }
    XCloseDisplay(d);
    return 0;
}
