#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#define MAX_KEY_CODES 256
static int rcvd_keys[MAX_KEY_CODES];
static int sent_keys[MAX_KEY_CODES];

void send_key(const struct libevdev_uinput *out, int code, int value) {
  libevdev_uinput_write_event(out, EV_KEY, code, value);
  sent_keys[code] = value;

  // fprintf(stderr, "\ncode=%d value=%d meta=%d left=%d home=%d",
  //   code, value,  sent_keys[KEY_LEFTMETA], sent_keys[KEY_LEFT], sent_keys[KEY_HOME]);
}

void sync_key(const struct libevdev_uinput *out, int code, int value) {
  if (sent_keys[code] == 0 && value != 0)
    send_key(out, code, 1);
  if (sent_keys[code] != 0 && value == 0)
    send_key(out, code, 0);
}

void sync_keys(const struct libevdev_uinput *out)
{
  for (int i = 0; i < MAX_KEY_CODES; i++)
    sync_key(out, i, rcvd_keys[i]);
}

bool remap_key(const struct libevdev_uinput *out, struct input_event ev, int code, int mod, int new_code)
{
  if ((ev.code == code && rcvd_keys[mod]  != 0)
   || (ev.code == mod  && rcvd_keys[code] != 0))
  {
    if (ev.value == 1) {
      sync_key(out, code, 0);
      sync_key(out, mod, 0);
    }

    send_key(out, new_code, ev.value);

    if (ev.value == 0) {
      sync_key(out, mod, rcvd_keys[mod]);
    }
    return true;
  }
  return false;
}

void handle_event(struct input_event ev, const struct libevdev_uinput *out)
{
  if (ev.type != EV_KEY) {
    libevdev_uinput_write_event(out, ev.type, ev.code, ev.value);
    return;
  }

  rcvd_keys[ev.code] = ev.value;

  bool match = remap_key(out, ev, KEY_LEFT,  KEY_LEFTMETA, KEY_HOME    )
            || remap_key(out, ev, KEY_RIGHT, KEY_LEFTMETA, KEY_END     )
            || remap_key(out, ev, KEY_UP,    KEY_LEFTMETA, KEY_PAGEUP  )
            || remap_key(out, ev, KEY_DOWN,  KEY_LEFTMETA, KEY_PAGEDOWN);
  if (match) return;

  send_key(out, ev.code, ev.value);
}

int main(int argc, char **argv)
{
  char *dev_path = argv[1];

  int fd = open(dev_path, O_RDONLY);
  if (fd < 0)
  {
    perror("open input device");
    exit(1);
  }

  struct libevdev *in;
  if (libevdev_new_from_fd(fd, &in) < 0)
  {
    perror("device");
    exit(1);
  }
  fprintf(stderr, "Input device name: %s\n", libevdev_get_name(in));

  if (libevdev_grab(in, LIBEVDEV_GRAB) < 0)
  {
    perror("grab input device");
    exit(1);
  }
  // libevdev_grab(in, LIBEVDEV_UNGRAB);

  struct libevdev_uinput *out;
  if (libevdev_uinput_create_from_device(in, LIBEVDEV_UINPUT_OPEN_MANAGED, &out) < 0)
  {
    perror("uinput device");
    exit(1);
  }

  while (1)
  {
    struct input_event ev;
    int ok = libevdev_next_event(in, LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_BLOCKING, &ev);
    if (ok == LIBEVDEV_READ_STATUS_SUCCESS)
    {
      // printf("event: %s %s\n",
      //        libevdev_event_type_get_name(ev.type),
      //        libevdev_event_code_get_name(ev.type, ev.code));
      handle_event(ev, out);
    }
    else if (ok == LIBEVDEV_READ_STATUS_SYNC)
    {
      fprintf(stderr, "need to resync input device!\n");
    }
  }
}
