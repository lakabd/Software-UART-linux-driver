#ifndef SOFT_UART_H
#define SOFT_UART_H


int uart_init(const int tx, const int rx, struct uart_port *port);
irqreturn_t uart_handle_rx(int irq, void* dev);
int uart_handle_tx(struct uart_port *port);
int uart_exit(void);
void uart_set_baudrate(unsigned int rate);
void rx_tasklet_function(unsigned long data);


#endif