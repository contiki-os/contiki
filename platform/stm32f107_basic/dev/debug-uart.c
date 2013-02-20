#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/f1/gpio.h>

#include <gqueue.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

DECLARE_QUEUE(char, uart_tx_buf, 256);
DECLARE_QUEUE(char, uart_rx_buf, 256);

static void uart_init_gpio(void)
{
	gpio_set_mode(GPIOA,
	              GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
	              GPIO9);

	gpio_set_mode(GPIOA,
	              GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_FLOAT,
	              GPIO10);

	/* USART lines should idle high */
	gpio_set(GPIOA, GPIO9);
	gpio_set(GPIOA, GPIO10);
}

void uart_init(int baud)
{
	uart_init_gpio();

	nvic_enable_irq(NVIC_USART1_IRQ);
	nvic_set_priority(NVIC_USART1_IRQ, 2);

	usart_set_baudrate(USART1, baud);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_stopbits(USART1, USART_CR2_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_enable_rx_interrupt(USART1);
	usart_enable(USART1);

	/* This ensures stdio doesn't use its own buffers */
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
}

int uart_putchar(char c)
{
	if( c == '\n')
		uart_putchar('\r');
	if(!queue_is_full(&uart_tx_buf)) {
		queue_enqueue(&uart_tx_buf, &c);
	} else {
		return -ENOMEM;
	}

	usart_enable_tx_interrupt(USART1);

	return 0;
}

int uart_getchar(char *c)
{
	if(!queue_is_empty(&uart_rx_buf)) {
		queue_dequeue(&uart_rx_buf, c);
		return 0;
	} else {
		return -ENODATA;
	}
}

void usart1_isr(void)
{
	char c;
	if (usart_get_flag(USART1, USART_SR_TXE)) {
		if (!queue_is_empty(&uart_tx_buf)) {
			queue_dequeue(&uart_tx_buf, &c);
			usart_send(USART1, (uint16_t)c);
		} else {
			usart_disable_tx_interrupt(USART1);
		}
	}


	if (usart_get_flag(USART1, USART_SR_RXNE)) {
		if (!queue_is_full(&uart_rx_buf)) {
			c = usart_recv(USART1);
			queue_enqueue(&uart_rx_buf, &c);
		}
	}
}
