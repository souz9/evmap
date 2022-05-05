#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#define MAX_KEY_CODES 1024
static bool pressed_keys[MAX_KEY_CODES];

void remap_key(const struct libevdev_uinput *out, int code, int value, int clear_code)
{
  if (value == 1)
    libevdev_uinput_write_event(out, EV_KEY, clear_code, 0);

  libevdev_uinput_write_event(out, EV_KEY, code, value);

  if (value == 0)
    libevdev_uinput_write_event(out, EV_KEY, clear_code, 1);
}

void handle_event(struct input_event ev, const struct libevdev_uinput *out)
{
  if (ev.type == EV_KEY && ev.code < MAX_KEY_CODES)
  {
    pressed_keys[ev.code] = ev.value != 0;
  }

  if (ev.type == EV_KEY && ev.code == KEY_LEFT && pressed_keys[KEY_LEFTMETA])
  {
    remap_key(out, KEY_HOME, ev.value, KEY_LEFTMETA);
  }
  else if (ev.type == EV_KEY && ev.code == KEY_RIGHT && pressed_keys[KEY_LEFTMETA])
  {
    remap_key(out, KEY_END, ev.value, KEY_LEFTMETA);
  }
  else if (ev.type == EV_KEY && ev.code == KEY_UP && pressed_keys[KEY_LEFTMETA])
  {
    remap_key(out, KEY_PAGEUP, ev.value, KEY_LEFTMETA);
  }
  else if (ev.type == EV_KEY && ev.code == KEY_DOWN && pressed_keys[KEY_LEFTMETA])
  {
    remap_key(out, KEY_PAGEDOWN, ev.value, KEY_LEFTMETA);
  }
  else
  {
    libevdev_uinput_write_event(out, ev.type, ev.code, ev.value);
  }
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
