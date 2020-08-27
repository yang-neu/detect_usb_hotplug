#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <libusb.h>
#include <libusb-1.0/libusb.h>
 
static int count = 0;

void add_callback(int fd, short events, void *user_data)
{
  printf("add_callback");
  return;
}

void rm_callback(int fd, void *user_data)
{
  printf("rm_callback");
  return;
}
 
int hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
                     libusb_hotplug_event event, void *user_data) {
  static libusb_device_handle *dev_handle = NULL;
  struct libusb_device_descriptor desc;
  int rc;
 
  printf("hotplug_callback.\n");
  (void)libusb_get_device_descriptor(dev, &desc);
 
  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
    rc = libusb_open(dev, &dev_handle);
    if (LIBUSB_SUCCESS != rc) {
      printf("Could not open USB device\n");
    }
  } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
    printf("USB device disconnected\n");
    if (dev_handle) {
      libusb_close(dev_handle);
      dev_handle = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  count++;
 
  return 0;
}
 
int main (void) {
  libusb_hotplug_callback_handle callback_handle;
  int rc;
  libusb_context *ctx;
 
  //libusb_init(NULL);
  libusb_init(&ctx);
  libusb_set_pollfd_notifiers(ctx, add_callback, rm_callback, NULL);
 
  rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                        LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, 0x067b, 0x2303,
                                        LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL,
                                        &callback_handle);
  if (LIBUSB_SUCCESS != rc) {
    printf("Error creating a hotplug callback\n");
    libusb_exit(NULL);
    return EXIT_FAILURE;
  }
 
  while (count < 22) {
    printf("Waiting...\n");
    //libusb_handle_events_completed(NULL, NULL);
    libusb_handle_events_completed(ctx, NULL);
    printf("Completed.\n");
    nanosleep(&(struct timespec){0, 10000000UL}, NULL);
  }
 
  libusb_hotplug_deregister_callback(NULL, callback_handle);
  libusb_exit(NULL);
 
  return 0;
}
