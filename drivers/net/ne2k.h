#ifndef NE2K_H
#define NE2K_H

#define NE2K_PACKET_MAX 1518

void ne2k_init();
unsigned int ne2k_present();
unsigned char* ne2k_mac();
unsigned int ne2k_tx_count();
unsigned int ne2k_rx_count();
unsigned int ne2k_last_status();
int ne2k_send(unsigned char* data,unsigned int length);
int ne2k_poll(unsigned char* buffer,unsigned int* length);

#endif
