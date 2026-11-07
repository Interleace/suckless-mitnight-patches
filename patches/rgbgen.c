/* RGB stream source: one triplet per line, stdout is the interface. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void usage(void) { fprintf(stderr,"usage: rgbgen [-m rainbow|pulse|solid] [-s cycles/second] [-f frames/second] [-r R] [-g G] [-b B]\n"); exit(2); }
static unsigned char byte(double x) { if (x<0) x=0; if(x>1) x=1; return (unsigned char)lround(x*255.0); }
static void hsv(double h, double *r, double *g, double *b) {
    double x=h*6.0, c=1.0, q=1.0-fabs(fmod(x,2.0)-1.0);
    switch ((int)x%6) {
        case 0:*r=c;*g=q;*b=0;break;
        case 1:*r=q;*g=c;*b=0;break;
        case 2:*r=0;*g=c;*b=q;break;
        case 3:*r=0;*g=q;*b=c;break;
        case 4:*r=q;*g=0;*b=c;break;
        default:*r=c;*g=0;*b=q;break;
    }
}
int main(int argc, char **argv) {
    const char *mode="rainbow";
    double speed=0.10, fps=25.0;
    int sr=255,sg=0,sb=90,i, commands=1;
    double phase=0.0;
    char control[128];
    struct pollfd pollin = {.fd=STDIN_FILENO,.events=POLLIN};
    struct timespec next, step;
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"-m") && i+1<argc) mode=argv[++i];
        else if(!strcmp(argv[i],"-s") && i+1<argc) speed=strtod(argv[++i],NULL);
        else if(!strcmp(argv[i],"-f") && i+1<argc) fps=strtod(argv[++i],NULL);
        else if(!strcmp(argv[i],"-r") && i+1<argc) sr=atoi(argv[++i]);
        else if(!strcmp(argv[i],"-g") && i+1<argc) sg=atoi(argv[++i]);
        else if(!strcmp(argv[i],"-b") && i+1<argc) sb=atoi(argv[++i]);
        else usage();
    }
    if (fps<1 || fps>60 || speed<0 || speed>10 || sr<0 || sr>255 || sg<0 || sg>255 || sb<0 || sb>255 ||
        (strcmp(mode,"rainbow") && strcmp(mode,"pulse") && strcmp(mode,"solid"))) usage();
    step.tv_sec=(time_t)(1.0/fps); step.tv_nsec=(long)((1.0/fps-step.tv_sec)*1e9);
    clock_gettime(CLOCK_MONOTONIC,&next);
    for (unsigned long long frame=0;;frame++) {
        double x,y,z,level;
        if (commands && poll(&pollin,1,0)>0) {
            if (pollin.revents & POLLIN) {
                double requested;
                int cr,cg,cb;
                if (!fgets(control,sizeof control,stdin)) commands=0;
                else if (sscanf(control,"speed %lf",&requested)==1 && requested>=0 && requested<=10)
                    speed=requested;
                else if (sscanf(control,"color %d %d %d",&cr,&cg,&cb)==3 &&
                    cr>=0&&cr<=255&&cg>=0&&cg<=255&&cb>=0&&cb<=255) {sr=cr;sg=cg;sb=cb;}
                else if (!strncmp(control,"mode rainbow",12)) mode="rainbow";
                else if (!strncmp(control,"mode pulse",10)) mode="pulse";
                else if (!strncmp(control,"mode solid",10)) mode="solid";
            } else if (pollin.revents & (POLLHUP|POLLERR|POLLNVAL)) commands=0;
        }
        if(!strcmp(mode,"rainbow")) hsv(phase,&x,&y,&z);
        else if(!strcmp(mode,"pulse")) { level=(1.0-cos(phase*2*3.14159265358979323846))*0.5; x=sr/255.0*level; y=sg/255.0*level;z=sb/255.0*level; }
        else { x=sr/255.0;y=sg/255.0;z=sb/255.0; }
        if(printf("%u %u %u\n",byte(x),byte(y),byte(z))<0 || fflush(stdout)==EOF) return 0;
        phase=fmod(phase + speed/fps,1.0);
        next.tv_nsec += step.tv_nsec; next.tv_sec += step.tv_sec + next.tv_nsec/1000000000L;
        next.tv_nsec %= 1000000000L;
        while(clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&next,NULL)==EINTR) {}
    }
}
