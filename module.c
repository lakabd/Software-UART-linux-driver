#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h> 
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>

#include "soft_uart.h"
#include "circular_buffer.h"

#define GPIO_RX 27
#define GPIO_TX 17

#define FIFO_SIZE 1024

extern struct buffer tx_buffer;
extern struct buffer rx_buffer;

static struct uart_driver softUart_driver = 
{
  .owner = THIS_MODULE,
  .driver_name = "ttySOFT",
  .dev_name = "ttySOFT", /*the name that will apeare under /dev */
  .major = 240,
  .minor = 0,
  .nr = 1, /*number of ports in this driver*/
};

static unsigned int softUart_tx_empty(struct uart_port *port)
{
    printk("...........tx_empty ! \n");
    while(!isBufferEmpty(&tx_buffer)); // wait until buffer is empty
    return TIOCSER_TEMT;
}

static unsigned int softUart_get_mctrl(struct uart_port *port)
{
    printk("...........get_mctrl ! \n");
    return TIOCM_CAR | TIOCM_CTS | TIOCM_DSR;
}

static void softUart_stop_tx(struct uart_port *port)
{
    printk("...........stop_tx ! \n");
}

static void softUart_start_tx(struct uart_port *port)
{
    printk("...........start_tx ! \n");
    //printk("softUart : data to send : %ld byte \n",uart_circ_chars_pending(&port->state->xmit));
    /*copy data from xmit to tx_buffer*/
    push_string(&tx_buffer, (unsigned char *)(port->state->xmit.buf + port->state->xmit.tail), uart_circ_chars_pending(&port->state->xmit));
    /*update xmit tail*/
    port->state->xmit.tail = port->state->xmit.head;
    /*start tx*/
    uart_handle_tx(port);
}

static void softUart_set_mctrl(struct uart_port *port, unsigned int mctrl){}
static void softUart_throttle(struct uart_port *port){printk("...........throttle ! \n");}
static void softUart_unthrottle(struct uart_port *port){printk("...........unthrottle ! \n");}
static void softUart_send_xchar(struct uart_port *port, char ch){printk("...........send x_char ! \n");}
static void softUart_pm(struct uart_port *port, unsigned int state, unsigned int oldstate){}
static void softUart_release_port(struct uart_port *port){printk("...........release_port ! \n");}
static void softUart_config_port(struct uart_port *port, int kiki){printk("...........config_port ! \n");}
static void softUart_enable_ms(struct uart_port *port){printk("...........enable_ms ! \n");}
static void softUart_break_ctl(struct uart_port *port, int ctl){printk("...........break_ctl ! \n");}

static void softUart_stop_rx(struct uart_port *port)
{
    printk("...........stop_rx ! \n");
}

static int softUart_startup(struct uart_port *port)
{
    printk("...........startup! \n");
    int ret;
    ret = uart_init(GPIO_TX,GPIO_RX, port);
    return ret;
}

static void softUart_shutdown(struct uart_port *port)
{
    printk("...........shutdown! \n");
    uart_exit();
}

static void softUart_flush_buffer(struct uart_port *port)
{
    printk("...........flush_buffer ! \n");
    clear_buffer(&tx_buffer);
    clear_buffer(&rx_buffer);
}

static void softUart_set_termios(struct uart_port *port, struct ktermios *new, struct ktermios *old)
{
    printk("...........set_termios : %o \n", new->c_cflag);
    unsigned int baud;
    baud = tty_termios_baud_rate(new);
    if (baud >= 1200 || baud <= 19200)
    {
        uart_set_baudrate(baud);
    }
    else 
    {
        printk(KERN_WARNING "default Baudrate of 9600 is used  : min 1200, max 19200! \n");
    } 
    
    if((new->c_cflag & CS8) != CS8)
        printk(KERN_WARNING "Only 8bit data size is available ! \n");
    if((new->c_cflag & PARENB) || (new->c_cflag & PARODD))
        printk(KERN_WARNING "No parity bit is available ! \n");
}

static void softUart_set_ldisc(struct uart_port *port, struct ktermios *new)
{
    printk("...........set_ldisc ! \n");
    if (new->c_line == 0)
        printk("Line discipline changed to N_TTY \n");
    else if (new->c_line == 1)
        printk("Line discipline changed to N_SLIP \n");
}

const char *softUart_type(struct uart_port *port)
{
    printk("...........type ! \n");
    return NULL;
}


static int softUart_request_port(struct uart_port *port){
    printk("...........request_port ! \n");
    return 0;
}

static int softUart_verify_port(struct uart_port *port, struct serial_struct *kiki){
    printk("...........verify_port ! \n");
    return 0;
}

static int softUart_ioctl(struct uart_port *port, unsigned int cmd, unsigned long arg)
{
    printk("...........ioctl cmd : %x ! \n",cmd);

    if (cmd == TCSETS)
    {
        struct termios *tty = (struct termios *) arg;

        if((tty->c_cflag & B19200) == B19200)
        uart_set_baudrate(19200);
        else if((tty->c_cflag & B9600) == B9600)
        uart_set_baudrate(9600);
        else if((tty->c_cflag & B4800) == B4800)
            uart_set_baudrate(4800);
        else if((tty->c_cflag & B1200) == B1200)
            uart_set_baudrate(1200);
        else 
        {
            printk(" default Baudrate of 9600 is used ! \n");
        }

        return 0;
    }
    
    return -ENOIOCTLCMD;
}

static struct uart_ops softUart_ops = 
{
    .tx_empty = softUart_tx_empty, // return TIOCSER_TEMT if tx_buffer is empty, otherwise return 0
    .set_mctrl  = softUart_set_mctrl, // defined but ignored
    .get_mctrl  = softUart_get_mctrl, // get modem ctrl : return TIOCM_CAR 0x040 | TIOCM_CTS 0x020 | TIOCM_DSR 0x100  include/uapi/asm-generic/termios.h
    .stop_tx  = softUart_stop_tx, // stop transmitting chars : due to a received XOFF char
    .start_tx = softUart_start_tx, // called by write() : start transmitting chars
    .throttle = softUart_throttle, // defined but ignored
    .unthrottle = softUart_unthrottle, // defined but ignored
    .send_xchar = softUart_send_xchar, // defined but ignored
    .stop_rx  = softUart_stop_rx, // Stop receiving characters : defined but ignored
    .enable_ms  = softUart_enable_ms, // defined but ignored
    .break_ctl  = softUart_break_ctl, // defined but ignored
    .startup  = softUart_startup, // called by open() : Grab any interrupt resources and initialise any low level driver state
    .shutdown = softUart_shutdown, // called by close() : Disable the port, disable any break condition that may be in effect, and free any interrupt resources
    .flush_buffer = softUart_flush_buffer, // called whenever port->state->xmit circular buffer is cleared : flush the write buffer
    .set_termios  = softUart_set_termios, // change the port parameters : only the baudrate change is permitted in our emplementation
    .set_ldisc = softUart_set_ldisc, // defined but ignored : Notifier for discipline change
    .pm = softUart_pm, // called by open() : for power managment : defined but ignored
    .type   = softUart_type, // return NULL;
    .release_port = softUart_release_port, // defined but ignored
    .request_port = softUart_request_port, // defined but ignored
    .config_port  = softUart_config_port, // defined but ignored
    .verify_port  = softUart_verify_port, // defined but ignored
    .ioctl = softUart_ioctl, // Perform any port specific IOCTLs.  IOCTL commands must be defined using the standard numbering system found in <asm/ioctl.h>
    //.poll_init = softUart_poll_init, // ignored
    //.poll_put_char = softUart_poll_put_char, // ignored
    //.poll_get_char = softUart_poll_get_char, // ignored
};

struct uart_port softUart_port = 
{
    .fifosize = FIFO_SIZE, /*tx & rx buffer size*/
    .ops = &softUart_ops,
    .type = TTY_DRIVER_TYPE_SERIAL, /*driver of type serial*/
};


static int __init  mymodule_init(void)
{
    int ret;

    ret = uart_register_driver(&softUart_driver);
    if (ret) {
        printk(KERN_ERR "softUart: could not register driver: %d\n", ret);
        return ret;
    }
    ret = uart_add_one_port(&softUart_driver, &softUart_port);
    if (ret) {
        printk(KERN_ERR "softUart: could not add port: %d\n", ret);
        uart_unregister_driver(&softUart_driver);
        return ret;
    }
    printk(KERN_INFO "Module initilized ! \n");
	return 0;
}


static  void __exit  mymodule_exit(void)
{
    uart_remove_one_port(&softUart_driver, &softUart_port);
    uart_unregister_driver(&softUart_driver);
    printk(KERN_INFO "Bye. \n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("LAKBIR Abderrahim");
MODULE_DESCRIPTION("My softUart kernel module");
module_init(mymodule_init);
module_exit(mymodule_exit);