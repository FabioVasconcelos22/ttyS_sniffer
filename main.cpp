#define _XOPEN_SOURCE
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

//LibDrivers ---- ptr ---- /dev/ttySx ---- device

bool set_interface_attribs (int fd)
{
    struct termios port_settings;

    if (tcgetattr (fd, &port_settings) != 0) {
        std::cout << "tcgetattr" << std::endl;
        return false;
    }

    cfsetospeed (&port_settings, B9600);
    cfsetispeed (&port_settings, B9600);

    port_settings.c_cflag =
            B9600 | CS8 | CLOCAL | CREAD; // setup to 8 bits/byte; parity: false; 1 stop bit; 1 start bit
    port_settings.c_iflag = INPCK | IGNPAR;                  // Enable parity check; Ignore parity errors
    port_settings.c_oflag = 0;
    port_settings.c_cc[VTIME] = 0; /* inter-character timer unused */
    port_settings.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */


    if (tcsetattr (fd, TCSANOW, &port_settings) != 0)
    {
        std::cout << "tcsetattr error";
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    std::string const path = "/dev/ttyS2";

    int pt = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (pt < 0)
    {
        perror("open /dev/ptmx");
        return 1;
    }
    grantpt(pt);
    unlockpt(pt);
    fprintf(stderr, "Slave device: %s\n", ptsname(pt));

    int fd = open (path.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cout << "Error while opening fd" << std::endl;
        return -1;
    }

    if(!set_interface_attribs (fd)) {
        return -1;
    }

    while (1) {
        char buf [100];
        int n = read (pt, buf, sizeof buf);
        if (n>0) {
            std::cout << "--> ";
            for (int i = 0; i< sizeof (buf); ++i) {
                std::cout << buf [i];
            }
            std::cout <<std::endl;
            write(fd, buf, sizeof(buf));
        }

        n = read (fd, buf, sizeof buf);
        if (n>0) {
            std::cout << "<-- ";
            for (int i = 0; i< sizeof (buf); ++i) {
                std::cout << buf [i];
            }
            std::cout <<std::endl;
            write(pt, buf, sizeof(buf));
        }

    }

    close(fd);
    close(pt);

    return 0;
}