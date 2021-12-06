/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 57-7 */

/* ud_ucase_cl.c
   A UNIX domain client that communicates with the server in ud_ucase_sv.c.
   This client sends each command-line argument as a datagram to the server,
   and then displays the contents of the server's response datagram.
*/
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include "tlpi_hdr.h"
#include "ud_ucase.h"
#include "PJ_RPI.h"
#include <stdio.h>

#define SV_SOCK_PATH "/tmp/ud_ucase"
#define BUF_SIZE 10

int main(int argc, char *argv[])
{
    struct sockaddr_un svaddr, claddr;
    int sfd, j;
    size_t msgLen;
    ssize_t numBytes;
    char resp[BUF_SIZE];

    if(map_peripheral(&gpio) == -1) 
	{
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
		return -1;
	}

    /* Create client socket; bind to unique pathname (based on PID) */

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1)
        errExit("socket");

    memset(&claddr, 0, sizeof(struct sockaddr_un));
    claddr.sun_family = AF_UNIX;
    snprintf(claddr.sun_path, sizeof(claddr.sun_path),
            "/tmp/ud_ucase_cl.%ld", (long) getpid());

    if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    /* Construct address of server */

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    /* Send messages to server; echo responses on stdout */

    t_data data;

    printf("Choose an GPIO-pin: ");
    scanf("%d", &data.IO);  
    printf("Gpio = %d \n",data.IO);

    printf("Enter the period to toggle the leds (in sec): ");
    scanf("%d", &data.period);  
    printf("Period = %d \n",data.period);

    printf("Enter number of toggles: ");
    scanf("%d", &data.toggles);  
    printf("Toggles = %d \n",data.toggles);

    for (j = 1; j < 2; j++) {
        if (sendto(sfd, &data, sizeof(data), 0, (struct sockaddr *) &svaddr,
                sizeof(struct sockaddr_un)) != sizeof(data))
            fatal("sendto");
        
        t_data response;
        numBytes = recvfrom(sfd, &response, sizeof(response), 0, NULL, NULL);

        if (numBytes == -1)
            errExit("recvfrom");
        printf("Response => GPIO: %d, PERIOD: %d, TOGGLES: %d\n", data.IO,data.period,data.toggles);
    }
    exit(EXIT_SUCCESS);         /* Closes our socket; server sees EOF */
}
