#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <linux/input.h> // this does not compile
#include <errno.h>

// from <linux/input.h>

struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

struct event_entry {
    struct input_event event;
    float time;
};

#define EVIOCGVERSION       _IOR('E', 0x01, int)            /* get driver version */
#define EVIOCGID        _IOR('E', 0x02, struct input_id)    /* get device ID */
#define EVIOCGKEYCODE       _IOR('E', 0x04, int[2])         /* get keycode */
#define EVIOCSKEYCODE       _IOW('E', 0x04, int[2])         /* set keycode */

#define EVIOCGNAME(len)     _IOC(_IOC_READ, 'E', 0x06, len)     /* get device name */
#define EVIOCGPHYS(len)     _IOC(_IOC_READ, 'E', 0x07, len)     /* get physical location */
#define EVIOCGUNIQ(len)     _IOC(_IOC_READ, 'E', 0x08, len)     /* get unique identifier */

#define EVIOCGKEY(len)      _IOC(_IOC_READ, 'E', 0x18, len)     /* get global keystate */
#define EVIOCGLED(len)      _IOC(_IOC_READ, 'E', 0x19, len)     /* get all LEDs */
#define EVIOCGSND(len)      _IOC(_IOC_READ, 'E', 0x1a, len)     /* get all sounds status */
#define EVIOCGSW(len)       _IOC(_IOC_READ, 'E', 0x1b, len)     /* get all switch states */

#define EVIOCGBIT(ev,len)   _IOC(_IOC_READ, 'E', 0x20 + ev, len)    /* get event bits */
#define EVIOCGABS(abs)      _IOR('E', 0x40 + abs, struct input_absinfo)     /* get abs value/limits */
#define EVIOCSABS(abs)      _IOW('E', 0xc0 + abs, struct input_absinfo)     /* set abs value/limits */

#define EVIOCSFF        _IOC(_IOC_WRITE, 'E', 0x80, sizeof(struct ff_effect))   /* send a force effect to a force feedback device */
#define EVIOCRMFF       _IOW('E', 0x81, int)            /* Erase a force effect */
#define EVIOCGEFFECTS       _IOR('E', 0x84, int)            /* Report number of effects playable at the same time */

#define EVIOCGRAB       _IOW('E', 0x90, int)            /* Grab/Release device */

// end <linux/input.h>

#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd = open("/dev/input/event1", O_RDWR);
    if(fd < 0) {
        fprintf(stderr, "could not open device, %s\n", strerror(errno));
        return -1;
    }
    int version;
    if (ioctl(fd, EVIOCGVERSION, &version)) {
        fprintf(stderr, "could not get driver version, %s\n", strerror(errno));
        return 1;
    }

    struct timeval time1;
    gettimeofday(&time1, NULL);

    while (true) {
        // ファイルから1行ずつ読み込む
        char buffer[256];
        struct event_entry entry;
        memset(&entry, 0, sizeof(entry));
        if (fgets(buffer, 256, stdin) == NULL) {
            break;
        }
        sscanf(buffer, "%u%u%u%f", &(entry.event.type), &(entry.event.code), &(entry.event.value), &(entry.time));

        // 必要に応じて待機する
        struct timeval time2;
        gettimeofday(&time2, NULL);
        int diff = (time2.tv_sec - time1.tv_sec) * 1000 * 1000 + (time2.tv_usec - time1.tv_usec);
        int usec = entry.time * 1000 * 1000 - diff;
        if (usec > 0) {
            usleep(usec);
        }

        // デバイスファイルに書き込む
        struct input_event event;
        memset(&event, 0, sizeof(event));
        event.type = entry.event.type;
        event.code = entry.event.code;
        event.value = entry.event.value;
        int ret = write(fd, &event, sizeof(event));
        if(ret < sizeof(event)) {
            fprintf(stderr, "write event failed, %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}