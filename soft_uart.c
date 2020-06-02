#include <linux/kernel.h>
#include <linux/gpio.h> 
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#include "soft_uart.h"
#include "circular_buffer.h"

static int gpio_tx = 0;
static int gpio_rx = 0;
struct buffer tx_buffer;
struct buffer rx_buffer;
unsigned char send_char='\0';
unsigned char receive_char='\0';
bool stop_rx = false;

unsigned long sleep_interval_ns = 1000000000/9600; // defautl baudrate B9600

//tasklet to handle tty rx_buffer flip
struct tasklet_struct rx_tasklet;

void uart_set_baudrate(unsigned int rate)
{
  sleep_interval_ns = 1000000000/rate;
  printk("Current Baudrate  = %d \n", rate);
}


int uart_init(const int tx, const int rx, struct uart_port *port)
{
  int ret;

  //rx_tasklet init 
  tasklet_init(&rx_tasklet, rx_tasklet_function, (unsigned long) port);

  //initilize buffers
  initialize_buffer(&tx_buffer);
  initialize_buffer(&rx_buffer);

  // Initializes the GPIO pins.
  gpio_tx = tx;
  gpio_rx = rx;

  ret = gpio_request(gpio_tx, "uart_tx");
    if(ret<0) {printk(KERN_WARNING "gpio not reserved : %d\n", ret); return ret;}
  ret = gpio_direction_output(gpio_tx, 1);
    if(ret<0) {printk(KERN_WARNING "gpio direction not set : %d\n", ret); return ret;}
  ret = gpio_request(gpio_rx, "uart_tx");
    if(ret<0) {printk(KERN_WARNING "gpio not reserved : %d\n", ret); return ret;}
  ret = gpio_direction_input(gpio_rx);
    if(ret<0) {printk(KERN_WARNING "gpio direction not set : %d\n", ret); return ret;}

  //Initialize interrupt : rx trigger
  ret = request_irq(gpio_to_irq(gpio_rx), (irq_handler_t) uart_handle_rx, IRQF_TRIGGER_FALLING, "Rx_handler", NULL);
  if(ret<0) {printk(KERN_WARNING "irq request error : %d\n", ret);return ret;}
  else printk(KERN_INFO "irq requested succesfully ! \n");

  return 0;
}

irqreturn_t uart_handle_rx(int irq, void *dev)
{   
    disable_irq_nosync(irq); //disable the interrupt, so it's not triggered by the data bits |__|--|__|--

    //start bit
    ndelay(sleep_interval_ns + sleep_interval_ns/4); //adding 1/4 offset time to avoid sampling at the edge
    //data
    receive_char |= (gpio_get_value(gpio_rx) << 0);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 1);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 2);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 3);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 4);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 5);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 6);
    ndelay(sleep_interval_ns);
    receive_char |= (gpio_get_value(gpio_rx) << 7);
    //stop bit
    ndelay(sleep_interval_ns);

    //reenable the interrupt
    enable_irq(irq);

    //push_character to rx_buffer
    push_character(&rx_buffer, receive_char);
    //printk("received chars : ' %c ' \n",receive_char);
    
    //schedule tasklet
    tasklet_schedule(&rx_tasklet);

    //reeinitilize receive char
    receive_char = '\0';

    return IRQ_HANDLED;
}

void rx_tasklet_function(unsigned long data)
{
  struct uart_port *port = (struct uart_port *) data;
  unsigned char c;
  while(pull_character(&rx_buffer, &c) == 0)
  {
    uart_insert_char(port, 0, 0, c, TTY_NORMAL);
    port->icount.rx++;
  }
  //send data to tty layer
  tty_flip_buffer_push(&port->state->port);
}


int uart_handle_tx(struct uart_port *port)
{ 
  //printk("uart_tx started from start function \n");
  uart_write_wakeup(port);
  while(pull_character(&tx_buffer, &send_char) == 0)
  {
    //startbit
    gpio_set_value(gpio_tx, 0);
    ndelay(sleep_interval_ns);
    //data bits
    gpio_set_value(gpio_tx, 1 & (send_char >> 0));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 1));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 2));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 3));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 4));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 5));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 6));
    ndelay(sleep_interval_ns);
    gpio_set_value(gpio_tx, 1 & (send_char >> 7));
    ndelay(sleep_interval_ns);
    //stop bit
    gpio_set_value(gpio_tx, 1);
    ndelay(sleep_interval_ns);
    //statistics : increment port tx count
    port->icount.tx++;
  }
  return 0;
}


int uart_exit(void)
{
    /*free IRQ*/
    free_irq(gpio_to_irq(gpio_rx), NULL);

    /*free gpio*/
    gpio_free(gpio_tx);
    gpio_free(gpio_rx);

    /*kill rx tasklet*/
    tasklet_kill(&rx_tasklet);
    return 0;
}