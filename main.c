#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    system("gnome-terminal -e ./cloud");
    sleep(2);
    system("gnome-terminal -e ./controller");
    system("gnome-terminal -e \"./device Temp1 0 20\"");
    system("gnome-terminal -e \"./device Act1 1 20\"");
}