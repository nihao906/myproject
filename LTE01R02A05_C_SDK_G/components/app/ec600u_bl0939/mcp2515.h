
#ifndef _2515_H
#define _2515_H
/*
** Register offsets into the transmit buffers.
*/
#define TXBnCTRL 0
#define TXBnSIDH 1
#define TXBnSIDL 2
#define TXBnEID8 3
#define TXBnEID0 4
#define TXBnDLC 5
#define TXBnD0 6
#define TXBnD1 7
#define TXBnD2 8
#define TXBnD3 9
#define TXBnD4 10
#define TXBnD5 11
#define TXBnD6 12
#define TXBnD7 13
#define CANSTAT 14
#define CANCTRL 15

#define SIDH 0
#define SIDL 1
#define EID8 2
#define EID0 3

/*
** Register offsets into the receive buffers.
*/
#define RXBnCTRL 0
#define RXBnSIDH 1
#define RXBnSIDL 2
#define RXBnEID8 3
#define RXBnEID0 4
#define RXBnDLC 5
#define RXBnD0 6
#define RXBnD1 7
#define RXBnD2 8
#define RXBnD3 9
#define RXBnD4 10
#define RXBnD5 11
#define RXBnD6 12
#define RXBnD7 13

/*
** Bits in the TXBnCTRL registers.
*/
#define TXB_TXBUFE_M 0x80
#define TXB_ABTF_M 0x40
#define TXB_MLOA_M 0x20
#define TXB_TXERR_M 0x10
#define TXB_TXREQ_M 0x08
#define TXB_TXIE_M 0x04
#define TXB_TXP10_M 0x03

#define DLC_MASK 0x0F
#define RTR_MASK 0x40

#define TXB0CTRL 0x30
#define TXB0SIDH 0x31

#define TXB1CTRL 0x40
#define TXB1SIDH 0x41

#define TXB2CTRL 0x50
#define TXB2SIDH 0x51

#define TXPRIOHIGH 0x03
#define TXPRIOHIGHLOW 0x02
#define TXPRIOLOWHIGH 0x01
#define TXPRIOLOW 0x00

#define TXB_EXIDE_M 0x08 // In TXBnSIDL
#define TXB_RTR_M 0x40   // In TXBnDLC

#define RXB_IDE_M 0x08 // In RXBnSIDL
#define RXB_RTR_M 0x40 // In RXBnDLC

#define BFPCTRL 0x0C

#define B2RTS 0x20
#define B1RTS 0x10
#define B0RTS 0x08
#define B2RTSM 0x04
#define B1RTSM 0x02
#define B0RTSM 0x01

#define TEC 0x1C
#define REC 0x1D
#define CLKCTRL CANCTRL

#define RXF0SIDH 0
#define RXF0SIDL 1
#define RXF0EID8 2
#define RXF0EID0 3
#define RXF1SIDH 4
#define RXF1SIDL 5
#define RXF1EID8 6
#define RXF1EID0 7
#define RXF2SIDH 8
#define RXF2SIDL 9
#define RXF2EID8 10
#define RXF2EID0 11

#define RXF3SIDH 16
#define RXF3SIDL 17
#define RXF3EID8 18
#define RXF3EID0 19
#define RXF4SIDH 20
#define RXF4SIDL 21
#define RXF4EID8 22
#define RXF4EID0 23
#define RXF5SIDH 24
#define RXF5SIDL 25
#define RXF5EID8 26
#define RXF5EID0 27

#define RXF_EXIDE_M 0x08

#define RXM0SIDH 0x20
#define RXM1SIDH 0x24
#define CNF3 0x28
#define CNF2 0x29 //波特率·
#define CNF1 0x2A
#define CANINTE 0x2B
#define CANINTF 0x2C
#define EFLG 0x2D
#define TXRTSCTRL 0x0D

#define EFLG_RX1OVR 0x80
#define EFLG_RX0OVR 0x40
#define EFLG_TXBO 0x20
#define EFLG_TXEP 0x10
#define EFLG_RXEP 0x08
#define EFLG_TXWAR 0x04
#define EFLG_RXWAR 0x02
#define EFLG_EWARN 0x01

#define SJW1 0x00
#define SJW2 0x40
#define SJW3 0x80
#define SJW4 0xC0

#define BTLMODE_CNF3 0x80

#define SAMP1 0x00
#define SAMP3 0x40

#define SEG1 0x00
#define SEG2 0x01
#define SEG3 0x02
#define SEG4 0x03
#define SEG5 0x04
#define SEG6 0x05
#define SEG7 0x06
#define SEG8 0x07

#define BRP1 0x00
#define BRP2 0x01
#define BRP3 0x02
#define BRP4 0x03
#define BRP5 0x04
#define BRP6 0x05
#define BRP7 0x06
#define BRP8 0x07

#define IVRIE 0x80
#define WAKIE 0x40
#define ERRIE 0x20
#define TX2IE 0x10
#define TX1IE 0x08
#define TX0IE 0x04
#define RX1IE 0x02
#define RX0IE 0x01
#define NO_IE 0x00

#define IVRINT 0x80
#define WAKINT 0x40
#define ERRINT 0x20
#define TX2INT 0x10
#define TX1INT 0x08
#define TX0INT 0x04
#define RX1INT 0x02
#define RX0INT 0x01
#define NO_INT 0x00

#define RXB0CTRL 0x60
#define RXB1CTRL 0x70

#define RXB_RXRDY 0x80
#define RXB_RXM1 0x40
#define RXB_RXM0 0x20
#define RXB_RX_ANY 0x60
#define RXB_RX_EXT 0x40
#define RXB_RX_STD 0x20
#define RXB_RX_STDEXT 0x00
#define RXB_RXMx_M 0x60
// #define RXB_RXIE_M      0x10
#define RXB_RXRTR 0x08 // In RXBnCTRL
#define RXB_BUKT 0x04
#define RXB_BUKT_RO 0x02

#define RXB_FILHIT 0x01
#define RXB_FILHIT2 0x04
#define RXB_FILHIT1 0x02
#define RXB_FILHIT_M 0x07
#define RXB_RXF5 0x05
#define RXB_RXF4 0x04
#define RXB_RXF3 0x03
#define RXB_RXF2 0x02
#define RXB_RXF1 0x01
#define RXB_RXF0 0x00

#define CLKEN 0x04

#define CLK1 0x00
#define CLK2 0x01
#define CLK4 0x02
#define CLK8 0x03

#define MODE_NORMAL 0x00
#define MODE_SLEEP 0x20
#define MODE_LOOPBACK 0x40
#define MODE_LISTENONLY 0x60
#define MODE_CONFIG 0xE0
#define ABORT 0x10

#define RECEIVE_BUFFER(x) (0x60 + 0x10 * (x))
#define TRANSMIT_BUFFER(x) (0x30 + 0x10 * (x))
#define RESET 0xc0   // Reset internal registers to default state, set conf mode
#define RTS 0x80     // Trigg transmission
#define RD_STAT 0xA0 // Start reading status
#define BIT_MOD 0x05 // Bit modify command data == MASK, BITS
#define READ 0x03    // Read data from memory
#define WRITE 0x02   // Write data to memory
#define STATUS 0xa0
#define LED0_ON(fd) mcp_wr_bit(fd, BFPCTRL, 0Xff, 0x10)
#define LED1_ON(fd) mcp_wr_bit(fd, BFPCTRL, 0Xff, 0x20)
#define LED0_OFF(fd) mcp_wr_bit(fd, BFPCTRL, 0X00, 0x10)
#define LED1_OFF(fd) mcp_wr_bit(fd, BFPCTRL, 0X00, 0x20)

typedef struct
{
    uint32_t Id; /*!< Specifies the standard identifier.
                         This parameter can be a value between 0 to 0x7FF. */

    uint8_t IDE; /*!< Specifies the type of identifier for the message that
                      will be received. This parameter can be a value of
                      @ref CAN_identifier_type */

    uint8_t RTR; /*!< Specifies the type of frame for the received message.
                      This parameter can be a value of
                      @ref CAN_remote_transmission_request */

    uint8_t DLC; /*!< Specifies the length of the frame that will be received.
                      This parameter can be a value between 0 to 8 */

    uint8_t FMI; /*!< Specifies the index of the filter the message stored in
                      the mailbox passes through. This parameter can be a
                      value between 0 to 0xFF */

    uint8_t Data[8]; /*!< Contains the data to be received. It ranges from 0 to
                          0xFF. */

} CanRxMsg;

typedef struct
{
    uint32_t Id; /*!< Specifies the standard identifier.
                         This parameter can be a value between 0 to 0x7FF. */

    uint8_t IDE; /*!< Specifies the type of identifier for the message that
                      will be transmitted. This parameter can be a value
                      of @ref CAN_identifier_type */

    uint8_t RTR; /*!< Specifies the type of frame for the message that will
                      be transmitted. This parameter can be a value of
                      @ref CAN_remote_transmission_request */

    uint8_t DLC; /*!< Specifies the length of the frame that will be
                      transmitted. This parameter can be a value between
                      0 to 8 */
    uint8_t Rsv;
    uint8_t Data[8]; /*!< Contains the data to be transmitted. It ranges from 0
                          to 0xFF. */
} CanTxMsg;

#define CAN_Id_Standard ((uint32_t)0x00000000) /*!< Standard Id */
#define CAN_Id_Extended ((uint32_t)0x00000004) /*!< Extended Id */

#define CAN_ID_STD CAN_Id_Standard
#define CAN_ID_EXT CAN_Id_Extended
#define CAN_EFF_FLAG 0x80000000U // 扩展帧的标识
#define CAN_RTR_FLAG 0x40000000U // 远程帧的标识
#define CAN_ERR_FLAG 0x20000000U // 错误帧的标识，用于错误检查
// extern void mcp_cs(int fd);
// extern void mcp_ncs(int fd);
int spi_write_port2(int fd, uint8_t *spi_txbuf, int len);
int spi_write_read_port2(int fd, uint8_t *spi_txbuf, uint8_t *spi_rxbuf, int len);
// void mcp_rd_can ( unsigned char buffer, unsigned char * ext, unsigned long* can_id,unsigned char * dlc, unsigned char * rtr, unsigned char * data );

// void mcp_wr_can_id ( unsigned char  mcp_addr,unsigned long);
// unsigned long  mcp_rd_can_id ( unsigned char  mcp_addr);
void mcp_wr_can_id(int fd, unsigned char, unsigned int);
unsigned int mcp_rd_can_id(int fd, unsigned char);
void mcp_reset(int fd);
unsigned char mcp_rd(int fd, unsigned char);
void mcp_rds(int fd, unsigned char, unsigned char *, unsigned char);
void mcp_wr(int fd, unsigned char, unsigned char);
void mcp_wrs(int fd, unsigned char, unsigned char *, unsigned char);
unsigned char mcp_rd(int fd, unsigned char);
void mcp_write_tbuf(int fd, unsigned char value);
// void mcp_write_can (unsigned char  buffer, unsigned char  ext, unsigned long can_id,
//                         unsigned char  dlc, unsigned char  rtr, const unsigned char * data );
void mcp_transmit(int fd, unsigned char buffer);
// void mcp_init(int fd);
void mcp_write(int fd, unsigned char MCPaddr, const unsigned char *writedata, unsigned char length);
void mcp_ld_txbuf(int fd, unsigned char buf, unsigned char *DAT, unsigned char length);


void rts(int fd, unsigned char);
void mcp_wr_bit(int fd, unsigned char add, unsigned char DAT, unsigned char mask);
unsigned char get_mcp_rxstatus(int fd);
void mcp_rd_rxbuf(int fd, unsigned char buf, unsigned char *DAT, unsigned char length);
// uint8_t spi_write(uint8_t *data, uint8_t length);
// uint8_t spi_read(uint8_t *data, uint8_t length);
// uint8_t spi_write_read(uint8_t *wdata, uint8_t wlen, uint8_t *rdata, uint8_t rlen);



typedef struct can_config
{
    uint32_t baud_rate;
    uint8_t rx_ctrl[2];
    uint16_t can_mode;
    uint32_t rx_mask[2];
    uint32_t rx_filter[6];
} can_config_t;

struct can_status
{
    uint8_t bus_status;
    uint8_t status;
    uint16_t rx_cnt;
    uint16_t tx_cnt;
};


int mcp_can_init(int ch, can_config_t *cfg);
int can_tx_pack(int ch, CanTxMsg *TxPack);
int can_read_rxbuf(int ch, CanRxMsg *RxMsg);
unsigned char get_mcp_status(int fd);
#endif