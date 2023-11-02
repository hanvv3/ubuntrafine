//
// Created by nagato on 9/9/20.
//

#include <cstring>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>

#include "UltrafineDisplay.h"
#include "Globals.h"

UltrafineDisplay::UltrafineDisplay(libusb_device *lg_usb) {
  lg_dev = lg_usb;
  int open_result = libusb_open(lg_dev, &lg_handle);
  // handle error, for instance:
  if (open_result != LIBUSB_SUCCESS) {
    throw std::runtime_error("Failed to open USB device.");
  }

  struct libusb_device_descriptor desc;
  int r = libusb_get_device_descriptor(lg_dev, &desc);
  if (r != LIBUSB_SUCCESS) {
    throw std::runtime_error("Failed to get device descriptor.");
  }

  product_id = desc.idProduct;
  vendor_id = desc.idVendor;
  // libusb_get_string_descriptor_ascii(lg_handle, desc.iProduct, DisplayType, 127);
  strcpy(DisplayType, support_device[product_id]);
  int detach_result = libusb_set_auto_detach_kernel_driver(lg_handle, 1);
  if (detach_result != LIBUSB_SUCCESS) {
      throw std::runtime_error("Failed to set auto-detach kernel driver.");
  }
  
  libusb_success = libusb_claim_interface(lg_handle, lg_iface);
  if (libusb_success != LIBUSB_SUCCESS) {
	  printw("failed to claim interface %d. Error: %d\n", lg_iface, libusb_success);
	  printw("Error: %s\n", libusb_error_name(libusb_success));
    throw std::runtime_error("Failed to claim interface.");
  }
}

UltrafineDisplay::UltrafineDisplay(const UltrafineDisplay &a) {
  strcpy(DisplayType, a.DisplayType);
  product_id = a.product_id;
  vendor_id = a.vendor_id;
  lg_dev = a.lg_dev;
  lg_handle = a.lg_handle;
  lg_iface = 1;
  libusb_success = 0;
  brightness_step = 5;
}

UltrafineDisplay::~UltrafineDisplay(){
}


int UltrafineDisplay::get_lg_ultrafine_usb_devices(libusb_device **devs, int usb_cnt, libusb_device ***lg_devs) {
  // check the input
  if (!devs || usb_cnt <= 0) {
    std::cerr << "Invalid USB device list or count." << std::endl;
    return 0;  // Return 0 indicating no LG Ultrafine devices found
  }
  
  int lg_cnt = 0;
  struct libusb_device_descriptor desc;

  for (int i = 0; i < usb_cnt; i++) {
    int r = libusb_get_device_descriptor(devs[i], &desc);
	if(desc.idVendor != LG_ID_Vendor) continue;
	auto lg_search = support_device.find(desc.idProduct);
	if (lg_search != support_device.end()) {
	  lg_cnt++;
    }
  }
  if (lg_cnt == 0) return 0;
  *lg_devs = (libusb_device **)malloc(sizeof(libusb_device *) * lg_cnt);

  int k = 0;
  for (int i = 0; i < usb_cnt; i++) {
	  int r = libusb_get_device_descriptor(devs[i], &desc);
	  auto lg_search = support_device.find(desc.idProduct);
	  if (lg_search != support_device.end()) {
	    (*lg_devs)[k++] = devs[i];
	  }
  }

  return lg_cnt;
}

void UltrafineDisplay::LG_Close(){
  if (libusb_success == LIBUSB_SUCCESS){
	libusb_release_interface(lg_handle, lg_iface);
	libusb_attach_kernel_driver(lg_handle, lg_iface);
  }
  libusb_close(lg_handle);
}

uint16_t UltrafineDisplay::get_brightness() {
  u_char data[8] = {0x00};
  int res= libusb_control_transfer(lg_handle, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
								   HID_GET_REPORT, (HID_REPORT_TYPE_FEATURE << 8) | 0, 1, data, sizeof(data), 0);
  if (res < 0){
  }
  uint16_t val = data[0] + (data[1] << 8);
  return val;
}

void UltrafineDisplay::set_brightness(uint16_t val) {
  uint8_t data[6] = {
  	uint8_t(val & 0x00ff),
  	uint8_t((val >> 8) & 0x00ff),
  	0x00,
  	0x00,
  	0x00,
  	0x00
  };
  int res = libusb_control_transfer(lg_handle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
									HID_SET_REPORT, (HID_REPORT_TYPE_FEATURE << 8) | 0, 1, data, sizeof(data), 0);
}

uint8_t UltrafineDisplay::get_brightness_level() {
  uint16_t brightness = get_brightness();
  uint8_t brightness_percentage = static_cast<uint8_t>((float(brightness) * 100.0 / 54000));
  return std::min(std::max(brightness_percentage, uint8_t(0)), uint8_t(255));
}

void UltrafineDisplay::set_brightness_level(uint8_t val) {
  uint16_t brightess = val * 54000 / 100;
  set_brightness(brightess);
}

void UltrafineDisplay::dim_step() {
  int8_t brightnessVolume = get_brightness_level();
  brightnessVolume -= brightness_step;
  brightnessVolume = 0 > brightnessVolume ? 0 : brightnessVolume;
  set_brightness_level(brightnessVolume);
}

void UltrafineDisplay::brighten_step() {
  uint8_t brightnessVolume = get_brightness_level();
  brightnessVolume += brightness_step;
  brightnessVolume = 100 < brightnessVolume ? 100 : brightnessVolume;
  set_brightness_level(brightnessVolume);
}

char * UltrafineDisplay::getDisplayName() {
  return DisplayType;
}

void UltrafineDisplay::interactive() {
  int8_t brightnessVolume;
  brightnessVolume = get_brightness_level();

  printw("Press '-' or '=' to adjust brightnes.\n");
  printw("press 'i' to set brightness from 0 to 100.\n");
  printw("press 'm' to set max brightness.\n");
  printw("press 'q' to quit.\n");
  printw("Current brghitness volume : %d\r", get_brightness_level());
  int stop = false;
  while(!stop){
	clrtoeol();
	usleep(70 * 1000);
	printw("Current brghitness volume : %d\r", get_brightness_level());
	int c = getch();
	char buffer[128];
	int input_volume;

	switch(c){
	  case '+':
	  case '=':
		brightnessVolume += brightness_step;
		brightnessVolume = brightnessVolume >= 100 ? 100 : brightnessVolume;
	    set_brightness_level(brightnessVolume);
	    break;
	  case '-':
	  case '_':
	    brightnessVolume -= brightness_step;
	    brightnessVolume = brightnessVolume <= 0 ? 0 : brightnessVolume;
	    set_brightness_level(brightnessVolume);
	    break;
	  case 'i':
	  case 'I':
	    clear();
	    printw("Input brightness from 0 to 100\n");
	    printw("current brightness : %d\n", get_brightness_level());
	    printw("Input your brightness : ");
		echo();
	    getstr(buffer);
	    noecho();
	    input_volume = atoi(buffer);
	    brightnessVolume = input_volume > 100 ? 100 : input_volume;
		set_brightness_level(input_volume);
		clear();
		printw("Press '-' or '=' to adjust brightnes.\n");
		printw("press 'i' to set brightness from 0 to 100.\n");
		printw("press 'm' to set max brightness.\n");
		printw("press 'q' to quit.\n");
		break;
	  case 'm':
	    set_brightness_level(100);
	    break;
	  case 'q':
	  case '\n':
		stop = true;
		endwin();
		break;
	  default: break;
	}
  }
}