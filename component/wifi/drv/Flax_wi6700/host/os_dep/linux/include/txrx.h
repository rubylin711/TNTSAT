/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Acrospeed Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief
*   \author Acrospeed
*/

#ifndef __TXRX_H__
#define __TXRX_H__

void lynx_rx_tasklet(unsigned long data);
int lynx_tx_send(struct lynx *lnx, struct sk_buff *skb);

#endif
