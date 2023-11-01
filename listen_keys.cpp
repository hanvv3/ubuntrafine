#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <libevdev/libevdev.h>

const char* findMagicKeyboard() {

    const char* dev_name = "py-evdev-uinput";

    DIR* dir;
    struct dirent* ent;
    const char* base_path = "/dev/input/";
    //const char* dev_name = "Magic Keyboard";
    char* device_path = nullptr;

    if ((dir = opendir(base_path)) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (strncmp(ent->d_name, "event", 5) == 0) {
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s%s", base_path, ent->d_name);
                // for debugging
                //std::cerr << "looking in dir: " << full_path << std::endl;
                int fd = open(full_path, O_RDONLY | O_NONBLOCK);
                // for debugging
                //std::cerr << fd << std::endl;
                if (fd < 0) continue;

                struct libevdev* dev = nullptr;
                int result = libevdev_new_from_fd(fd, &dev);
                if (result >= 0) {
                    const char* name = libevdev_get_name(dev);
                    /* for debugging
                    if (name) {
                        std::cerr << "dev name: " << name << std::endl;
                    }*/
                    if (name && strstr(name, dev_name) != nullptr) {
                        // for debugging
                        //std::cerr << "match found!" << std::endl;
                        // Check if it's accessible without sudo
                        if (libevdev_grab(dev, LIBEVDEV_GRAB) == 0) {
                            device_path = strdup(full_path);
                            // for debugging
                            //std::cerr << "final dev_path: " << device_path << std::endl;
                            libevdev_grab(dev, LIBEVDEV_UNGRAB);
                            libevdev_free(dev);
                            close(fd);
                            break;
                        }
                    }
                    libevdev_free(dev);
                }
                close(fd);
            }
        }
        closedir(dir);
    }

    return device_path;
}


int main() {
    const char *devicePath = findMagicKeyboard();
    if (!devicePath) {
        std::cerr << "Magic Keyboard not found" << std::endl;
        return 1;
    }

    int fd = open(devicePath, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Error opening device" << std::endl;
        free((void*)devicePath);  // Clean up the allocated string
        return 1;
    }

    struct libevdev *device = nullptr;
    int result = libevdev_new_from_fd(fd, &device);
    if (result < 0) {
        std::cerr << "Failed to initialize libevdev" << std::endl;
        free((void*)devicePath);  // Clean up the allocated string
        return 1;
    }

    std::cout << "Listening for key events on " << devicePath << "..." << std::endl;
    free((void*)devicePath);  // Clean up the allocated string

    while (true) {
        struct input_event ev;
        int rc = libevdev_next_event(device, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            if (ev.type == EV_KEY && (ev.value == 1 || ev.value == 2)) { // Key press or key repeat event
                if (ev.code == KEY_BRIGHTNESSUP) {
                    //std::cout << "Brightness Up key pressed or held down." << std::endl;
                    system("env TERM=xterm ufctl +");  // Execute the brightness UP command
                }
                if (ev.code == KEY_BRIGHTNESSDOWN) {
                    //std::cout << "Brightness Down key pressed or held down." << std::endl;
                    system("env TERM=xterm ufctl -");  // Execute the brightness DOWN command
                }
            }
        }
    }

    libevdev_free(device);
    close(fd);
    return 0;
}
