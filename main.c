// Part of https://github.com/tempelmann/picow-udp-example
//
// Based on code by danjperron at https://forums.raspberrypi.com/viewtopic.php?t=340208#p2039727
// Special thanks to hippy (https://forums.raspberrypi.com/memberlist.php?mode=viewprofile&u=1729)
// Discussion that led to this: https://forums.raspberrypi.com/viewtopic.php?t=373977
//

#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "secret-credentials.h"	// create this file yourself and define WIFI_SSID and WIFI_PASSWORD in there

#define RECEIVER_IP  "192.168.4.8"	// update this to the IP of the computer that runs the "udp_sender.py" program
#define RECEIVER_PORT 6001
#define OWN_PORT 6002

#define DEBUG_printf  printf

bool TimerFlag = false;

static bool Send_udp_stuff(repeating_timer_t *rt) {
	TimerFlag = true;
}

static struct udp_pcb *upcb;

void SendUDP (char *ip, int port, void * data, int data_size) {
	ip_addr_t destAddr;
	ip4addr_aton (ip, &destAddr);
	struct pbuf *p = pbuf_alloc (PBUF_TRANSPORT, data_size, PBUF_RAM);
	memcpy (p->payload, data, data_size);
	cyw43_arch_lwip_begin();
	udp_sendto (upcb, p, &destAddr, port);
	cyw43_arch_lwip_end();
	pbuf_free(p);
}

void RcvFromUDP (void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
	char ip[32];
	ip4addr_ntoa_r (addr, ip, sizeof(ip));
	if (p->tot_len == p->len) {
		// single fragment
		printf ("Rcv from %s:%d, len %d\n", ip, port, p->tot_len);
	} else {
		printf ("Rcv from %s:%d, total %d [", ip, port, p->tot_len);
		for (struct pbuf *q = p; q != NULL; q = q->next) {	// we need to go over all fragments (it'll be more than one if size > MTU)
			printf("%d", q->len);
			//printf (" (%s)", (char*) p->payload);
			if (q->next) printf(" ");
		}
		printf("]\n");
	}
	pbuf_free(p);	// this is needed or we'll stop receiving packets after a while!
}


int main() {
	char buffer[32];
	stdio_init_all();

	unsigned int loop = 0;
	alarm_pool_t *alarmP;
	repeating_timer_t RptTimer;
	struct udp_pcb *spcb;

	if (cyw43_arch_init()) {
		DEBUG_printf("failed to initialise\n");
		return 1;
	}
	cyw43_arch_enable_sta_mode();

	printf("Connecting to WiFi...\n");
	while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0) {
		printf("Failed to connect. Retrying\n");
	}
	
	// get own IP address
	while (cyw43_tcpip_link_status(&cyw43_state, 0) != CYW43_LINK_UP) {
		sleep_ms(10);
	}
	bool linkup = netif_is_link_up(netif_default);
	bool anyip = ip4_addr_isany_val(*netif_ip4_addr(netif_default));
	char ip[32];
	ip4addr_ntoa_r (&cyw43_state.netif[0].ip_addr, ip, sizeof(ip));
	printf("IP: %s.\n", ip);

	alarm_pool_init_default();
	alarmP = alarm_pool_get_default();

	alarm_pool_add_repeating_timer_ms (alarmP, 1000, Send_udp_stuff, NULL, &RptTimer);

	upcb = udp_new();
	spcb = udp_new();

	err_t err = udp_bind(spcb, IP_ADDR_ANY, OWN_PORT);

	udp_recv (spcb, RcvFromUDP, NULL);

	while (1) {
		if (TimerFlag) {
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"%u\n",loop++);
			SendUDP (RECEIVER_IP, RECEIVER_PORT, buffer, sizeof(buffer));
			TimerFlag=false;
		}
		cyw43_arch_poll();
	}
	udp_remove(upcb);
	udp_remove(spcb);
	cyw43_arch_deinit();
	return 0;
}
