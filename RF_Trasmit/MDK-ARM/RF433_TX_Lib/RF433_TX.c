#include "RF433_TX.h"

static uint16_t vw_tx_msg_count = 0; //Tong so tin nhan da gui
static uint8_t vw_tx_index = 0;      //Chi muc cua ky kieu tiep theo de gui, pham vi tu 0 den vw_tx_len
static uint8_t vw_tx_bit = 0;        //So bit cua bit tiep theo de gui
static uint8_t vw_tx_sample = 0;     //Mau may thu hien tai
static uint8_t vw_tx_len = 0;        //Gioi han chuoi truyen
static volatile uint8_t vw_tx_enabled = 0;
static uint8_t vw_tx_buf[(VW_MAX_MESSAGE_LEN * 2) + VW_HEADER_LEN] 
     = {0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x38, 0x2c};

//Bang chuyen doi ky hieu tu 4 bit sang 6 bit duoc su dung de chuyen doi nua byte cao va thap
// cua du lieu duoc truyen thanh cac ky hieu 6 bit de truyen, moi bieu tuong 6 bit co 3 so 1
// va 3 so 0.
static uint8_t symbols[] =
{
    0xd,  0xe,  0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

/*-------Ham doi qua trinh truyen du lieu hoan tat------------- */
void vw_wait_tx()
{
    while (vw_tx_enabled);
}	

/*----------------Ham tinh crc16 citt ---------------------------*/
uint16_t _crc_ccitt_update (uint16_t crc, uint8_t data)
{
		data ^= lo8 (crc);
		data ^= data << 4;

		return ((((uint16_t)data << 8) | hi8 (crc)) ^ (uint8_t)(data >> 4) 
						^ ((uint16_t)data << 3));
}

/*-------------Bat dau qua trinh truyen du lieu -----------*/
void vw_tx_start()
{
    vw_tx_index = 0;
    vw_tx_bit = 0;
    vw_tx_sample = 0;

    // Next tick interrupt will send the first bit
    vw_tx_enabled = 1;
}

/*---------Ham dung qua trinh truyen du lieu --------------*/
void vw_tx_stop()
{
    vw_digitalWrite_tx(0);

    // No more ticks for the transmitter
    vw_tx_enabled = 0;
}

/*---------Ham dong goi du lieu de gui di------------------*/
uint8_t vw_send(uint8_t* buf, uint8_t len)
{
    uint8_t i;
    uint8_t index = 0;
    uint16_t crc = 0xffff;
    uint8_t *p = vw_tx_buf + VW_HEADER_LEN; // start of the message area
    uint8_t count = len + 3; // Added byte count and FCS to get total number of bytes

    if (len > VW_MAX_PAYLOAD)
	return 0;

    // Wait for transmitter to become available
    vw_wait_tx();

    // Encode the message length
    crc = _crc_ccitt_update(crc, count);
    p[index++] = symbols[count >> 4];
    p[index++] = symbols[count & 0xf];

    // Encode the message into 6 bit symbols. Each byte is converted into 
    // 2 6-bit symbols, high nybble first, low nybble second
    for (i = 0; i < len; i++)
    {
	crc = _crc_ccitt_update(crc, buf[i]);
	p[index++] = symbols[buf[i] >> 4];
	p[index++] = symbols[buf[i] & 0xf];
    }

    // Append the fcs, 16 bits before encoding (4 6-bit symbols after encoding)
    // Caution: VW expects the _ones_complement_ of the CCITT CRC-16 as the FCS
    // VW sends FCS as low byte then hi byte
    crc = ~crc;
    p[index++] = symbols[(crc >> 4)  & 0xf];
    p[index++] = symbols[crc & 0xf];
    p[index++] = symbols[(crc >> 12) & 0xf];
    p[index++] = symbols[(crc >> 8)  & 0xf];

    // Total number of 6-bit symbols to send
    vw_tx_len = index + VW_HEADER_LEN;

    // Start the low level interrupt handler sending symbols
    vw_tx_start();

    return 1;
}

/*--------------- Ham ngat Timer truyen du du lieu ---------------*/
void transmit_Data_In_Interrupt()
{
		if (vw_tx_enabled && vw_tx_sample++ == 0)
    {
			if (vw_tx_index >= vw_tx_len)
			{
					vw_tx_stop();
					vw_tx_msg_count++;
			}
			else
			{
					vw_digitalWrite_tx((GPIO_PinState)(vw_tx_buf[vw_tx_index] & (0x01<<(vw_tx_bit++))));
					if (vw_tx_bit >= 6)
					{
					 vw_tx_bit = 0;
					 vw_tx_index++;
					}
			}
	  }
				if (vw_tx_sample > 7)
			vw_tx_sample = 0;
}

