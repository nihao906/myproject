

#include <stdint.h>
#include "mcp2515.h"
#include "ql_api_spi.h"
#include "ql_api_osi.h"
// #include "hw.h"
#include "ql_log.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "mcp2515", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "mcp2515", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "mcp2515", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "mcp2515", msg, ##__VA_ARGS__)

extern struct can_status *can_status[2];

#define mcp_cs(fd)  // ql_spi_cs_low(fd)
#define mcp_ncs(fd) // ql_spi_cs_high(fd)

static uint8_t spi_txbuf[32] __attribute__((aligned(32)));
static uint8_t spi_rxbuf[32] __attribute__((aligned(32)));
unsigned char spi_put(unsigned char);

void mcp_wr_can_eid(int fd, unsigned char add, unsigned long can_id)
{
    unsigned char txbuf[4];
    uint16_t canid;
    canid = (uint16_t)can_id;
    txbuf[EID0] = canid & 0xff;
    txbuf[EID8] = (canid >> 8) & 0xff;
    canid = (uint16_t)(can_id >> 16);

    txbuf[SIDL] = canid & 0x03;
    canid = canid << 3;
    txbuf[SIDL] |= (canid & 0xe0);
    txbuf[SIDL] |= 0x8;
    txbuf[SIDH] = canid >> 8;
    mcp_wrs(fd, add, txbuf, 4);
}

unsigned long mcp_rd_can_eid(int fd, unsigned char add)
{
    unsigned char rcbuf[4];
    uint32_t can_id;
    uint16_t canid;
    mcp_rds(fd, add, rcbuf, 4);
    can_id = rcbuf[EID0] + (rcbuf[EID8] << 8);
    canid = rcbuf[SIDL] + (rcbuf[SIDH] << 8);
    canid = canid >> 3;
    canid |= (rcbuf[SIDL] & 0x03);
    can_id |= (canid << 16);
    return can_id;
}

void mcp_wr_can_id(int fd, unsigned char add, unsigned int CanId)
{
    unsigned char txbuf[4];
    uint16_t sid;
    uint32_t eid;
    if (CanId & CAN_EFF_FLAG) // eid
    {
        sid = (CanId >> 18) & 0x7ff;
        eid = CanId & 0x3ffff;
        txbuf[SIDH] = (uint8_t)(sid >> 3);
        txbuf[SIDL] = (sid << 5) & 0xe0;
        txbuf[SIDL] |= (1 << 3);
        txbuf[SIDL] |= (eid >> 16);
        txbuf[EID8] = (eid >> 8) & 0xff;
        txbuf[EID0] = eid & 0xff;
    }
    else
    {
        sid = CanId & 0x7ff;
        txbuf[SIDH] = (uint8_t)(sid >> 3);
        txbuf[SIDL] = (sid << 5) & 0xe0;
        txbuf[EID8] = 0;
        txbuf[EID0] = 0;
    }
    mcp_wrs(fd, add, txbuf, 4);
}
unsigned int mcp_rd_can_id(int fd, unsigned char add)
{
    unsigned int CanId;
    unsigned char rcbuf[4];
    mcp_rds(fd, add, rcbuf, 4);
    CanId = (rcbuf[SIDL] + (rcbuf[SIDH] << 8));
    CanId >>= 5;
    CanId &= 0x7ff;
    if (rcbuf[SIDL] & (1 << 3))
    {
        unsigned int eid;
        eid = rcbuf[EID0] | (rcbuf[EID8] << 8);
        eid |= (rcbuf[SIDL] & 0x03) << 16;
        CanId <<= 18;
        CanId |= eid;
        CanId |= CAN_EFF_FLAG;
    }
    return CanId;
}
void mcp_reset(int fd)
{
    mcp_cs(fd);
    spi_txbuf[0] = RESET;
    spi_write_port2(fd, spi_txbuf, 1);
    mcp_ncs(fd);
}
// 单字节写
void mcp_wr(int fd, unsigned char add, unsigned char DAT)
{
    spi_txbuf[0] = WRITE;
    spi_txbuf[1] = add;
    spi_txbuf[2] = DAT;
    mcp_cs(fd);
    spi_write_port2(fd, spi_txbuf, 3);
    mcp_ncs(fd);
}
// 多字节写
void mcp_wrs(int fd, unsigned char add, unsigned char *wr_dat, unsigned char length)
{
    spi_txbuf[0] = WRITE;
    spi_txbuf[1] = add;
    mcp_cs(fd);
    for (int i = 0; i < length; i++)
    {
        spi_txbuf[i + 2] = wr_dat[i];
    }
    spi_write_port2(fd, spi_txbuf, length + 2);
    mcp_ncs(fd);
}
// 单字节读
unsigned char mcp_rd(int fd, unsigned char add)
{
    spi_txbuf[0] = READ;
    spi_txbuf[1] = add;
    spi_txbuf[2] = 0;
    mcp_cs(fd);
    spi_write_read_port2(fd, spi_txbuf, spi_rxbuf, 3);
    mcp_ncs(fd);
    return (spi_rxbuf[2]);
}
// 多字节读
void mcp_rds(int fd, unsigned char add, unsigned char *DAT, unsigned char length)
{
    spi_txbuf[0] = READ;
    spi_txbuf[1] = add;
    for (int i = 0; i < length; i++)
    {
        spi_txbuf[2 + i] = 0;
    }
    mcp_cs(fd);
    spi_write_read_port2(fd, spi_txbuf, spi_rxbuf, length + 2);
    for (int i = 0; i < length; i++)
    {
        *DAT++ = spi_rxbuf[2 + i];
    }
    mcp_ncs(fd);
}
// 发送请求
void rts(int fd, unsigned char ch)
{
    ch &= 0x7;
    ch |= 0x80;
    spi_txbuf[0] = ch;
    mcp_cs(fd);
    spi_write_port2(fd, spi_txbuf, 1);
    mcp_ncs(fd);
}
// 状态读
unsigned char get_mcp_status(int fd)
{
    spi_txbuf[0] = STATUS;
    spi_txbuf[1] = 0;
    mcp_cs(fd);
    spi_write_read_port2(fd, spi_txbuf, spi_rxbuf, 2);
    mcp_ncs(fd);
    return (spi_rxbuf[1]);
}
void mcp_wr_bit(int fd, unsigned char add, unsigned char DAT, unsigned char mask)
{
    spi_txbuf[0] = 0x05;
    spi_txbuf[1] = add;
    spi_txbuf[2] = mask;
    spi_txbuf[3] = DAT;
    mcp_cs(fd);
    spi_write_port2(fd, spi_txbuf, 4);
    mcp_ncs(fd);
}
void mcp_rd_rxbuf(int fd, unsigned char buf, unsigned char *DAT, unsigned char length)
{
    buf <<= 1;
    buf &= 0x06;
    buf |= 0x90;
    spi_txbuf[0] = buf;
    for (int i = 0; i < length; i++)
    {
        spi_txbuf[i + 1] = 0;
    }
    mcp_cs(fd);
    spi_write_read_port2(fd, spi_txbuf, spi_rxbuf, length + 1);
    for (int i = 0; i < length; i++)
    {
        *DAT++ = spi_rxbuf[1 + i];
    }
    mcp_ncs(fd);
}

void mcp_ld_txbuf(int fd, unsigned char buf, unsigned char *DAT, unsigned char length)
{
    mcp_cs(fd);
    buf &= 0x07;
    buf |= 0x40;
    spi_txbuf[0] = buf;
    for (int i = 0; i < length; i++)
    {
        // spi_txbuf[i + 1] = *DAT++;
        spi_txbuf[i + 1] = DAT[i];
        // spi_txbuf[i + 1] = 1;
    }
    spi_write_read_port2(fd, spi_txbuf, spi_rxbuf, length + 1);
    for (int i = 0; i < length; i++)
    {
        // *DAT++ = spi_rxbuf[i + 1];
        spi_rxbuf[i + 1] = 0;
    }
    mcp_ncs(fd);
}
unsigned char get_mcp_rxstatus(int fd)
{
    spi_txbuf[0] = 0xb0;
    spi_txbuf[1] = 0;
    mcp_cs(fd);
    spi_write_read_port2(fd, spi_txbuf, spi_rxbuf, 2);
    mcp_ncs(fd);
    return spi_rxbuf[1];
}

int mcp_can_init(int ch, can_config_t *cfg)
{
    // uint8_t value;
    int can_clock = 24000000;
    uint8_t brp;
    int bit_time;
    mcp_reset(ch);
    ql_rtos_task_sleep_ms(20);
    mcp_wr(ch, CANCTRL, MODE_CONFIG);
    if (mcp_rd(ch, CANCTRL) != MODE_CONFIG)
    {
        return -1;
    }
    bit_time = can_clock / 2 / cfg->baud_rate;
    if (bit_time % 12 == 0) // ±ê×¼²¨ÌØÂÊ 125,250,500,10000
    {
        mcp_wr(ch, CNF2, BTLMODE_CNF3 + SEG3 * 8 + SEG5);
        mcp_wr(ch, CNF3, SEG3);
        brp = bit_time / 12 - 1;
        if (brp > 7)
        {
            return -2;
        }
        mcp_wr(ch, CNF1, SJW1 + brp);
    }
    else if (bit_time % 15 == 0) // 100,200,400,800
    {
        mcp_wr(ch, CNF2, BTLMODE_CNF3 + SEG4 * 8 + SEG6);
        mcp_wr(ch, CNF3, SEG4);
        brp = bit_time / 15 - 1;
        if (brp > 7)
        {
            return -2;
        }
        mcp_wr(ch, CNF1, SJW1 + brp);
    }
    else if (bit_time % 16 == 0) // 750,325
    {
        mcp_wr(ch, CNF2, BTLMODE_CNF3 + SEG5 * 8 + SEG6);
        mcp_wr(ch, CNF3, SEG4);
        brp = bit_time / 16 - 1;
        if (brp > 7)
        {
            return -2;
        }
        mcp_wr(ch, CNF1, SJW1 + brp);
    }
    else // ²»Ö§³ÖµÄ²¨ÌØÂÊ
    {
        return -2;
    }

    mcp_wr(ch, RXB0CTRL, cfg->rx_ctrl[0]); // ÂË²¨Æ÷0
    mcp_wr_can_id(ch, RXM0SIDH, cfg->rx_mask[0]);
    mcp_wr_can_id(ch, RXF0SIDH, cfg->rx_filter[0]);

    mcp_wr(ch, RXB1CTRL, cfg->rx_ctrl[1]); // ÂË²¨Æ÷1
    mcp_wr_can_id(ch, RXF1SIDH, cfg->rx_filter[1]);
    mcp_wr_can_id(ch, RXM1SIDH, cfg->rx_mask[1]);
    mcp_wr_can_id(ch, RXF2SIDH, cfg->rx_filter[2]);
    mcp_wr_can_id(ch, RXF3SIDH, cfg->rx_filter[3]);
    mcp_wr_can_id(ch, RXF4SIDH, cfg->rx_filter[4]);
    mcp_wr_can_id(ch, RXF5SIDH, cfg->rx_filter[5]);

    mcp_wr(ch, CANINTE, 0x03);
    mcp_wr(ch, CANINTF, 0x00);

    mcp_wr_bit(ch, TXRTSCTRL, 0x0, 0xff);
    mcp_wr_bit(ch, BFPCTRL, 0x3c, 0xff);
    mcp_wr(ch, CANCTRL, (cfg->can_mode & 0xf0));
    LED0_ON(ch);
    LED1_ON(ch);
    LOGD("[%s] can%d init ok", __FUNCTION__, ch);
    return 0;
}

int can_read_rxbuf(int ch, CanRxMsg *RxMsg)
{
    unsigned char status;
    uint32_t can_id;
    status = get_mcp_rxstatus(ch);
    if (status & 0x40)
    {
        RxMsg->Id = mcp_rd_can_id(ch, 0x61);
        if (RxMsg->Id & (1 << 31))
        {
            RxMsg->Id &= 0x1fffffff;
            RxMsg->IDE = CAN_Id_Extended;
        }
        else
        {
            RxMsg->Id &= 0x7ff;
            RxMsg->IDE = CAN_Id_Standard;
        }
        RxMsg->DLC = mcp_rd(ch, 0x65);
        if (RxMsg->DLC & 0x40)
        {
            RxMsg->DLC = 0;
            RxMsg->RTR = 1;
            goto ret1; // ËÍµ½rtr
        }
        RxMsg->RTR = 0;
        if (RxMsg->DLC > 8)
        {
            RxMsg->DLC = 8;
        }
        mcp_rd_rxbuf(ch, 0x1, RxMsg->Data, RxMsg->DLC);
    ret1:
        mcp_wr_bit(ch, CANINTF, 0x00, 0x01);
        return 1;
    }
    if (status & 0x80)
    {
        RxMsg->Id = mcp_rd_can_id(ch, 0x71);
        if (can_id & (1 << 31))
        {
            RxMsg->Id &= 0x1fffffff;
            RxMsg->IDE = CAN_Id_Extended;
        }
        else
        {
            RxMsg->Id &= 0x7ff;
            RxMsg->IDE = CAN_Id_Standard;
        }
        RxMsg->DLC = mcp_rd(ch, 0x75);
        if (RxMsg->DLC & 0x40)
        {
            RxMsg->DLC = 0;
            RxMsg->RTR = 1;
            goto ret2; // ÊÕµ½rtr
        }
        RxMsg->RTR = 0;
        if (RxMsg->DLC > 8)
        {
            RxMsg->DLC = 8;
        }
        mcp_rd_rxbuf(ch, 0x3, RxMsg->Data, RxMsg->DLC);
    ret2:
        mcp_wr_bit(ch, CANINTF, 0x00, 0x02);
        return 2;
    }

    return 0;
}

int can_tx_pack(int ch, CanTxMsg *TxPack)
{
    // #ifdef _CAN_DEBUG_
    //     char msg[256];
    //     int i;
    //     int n = sprintf(msg, "CAN%d Tx: Id=0x%x,IDE=%x,DLC=%d,Data=[", ch, TxPack->Id, TxPack->IDE, TxPack->DLC);
    //     for (i = 0; i < TxPack->DLC; i++)
    //     {
    //         n += sprintf(&msg[n], "%02x ", TxPack->Data[i]);
    //     }
    //     msg[n - 1] = ']';
    //     eat_trace(msg);
    // #endif
    can_status[0]->tx_cnt++;
    if (!(mcp_rd(ch, 0x30) & 0x08))
    { // ·¢ËÍ»º³åÇø0¿Õ£¿
        LED1_ON(ch);
        if (TxPack->IDE == CAN_Id_Extended)
        {
            TxPack->Id |= (1 << 31);
        }
        else
        {
            TxPack->Id &= 0x7ff;
        }
        mcp_wr_can_id(ch, 0x31, TxPack->Id);
        if (TxPack->RTR)
        {
            mcp_wr(ch, 0x35, 0x40); // ·¢ËÍRTR
        }
        else
        {
            mcp_wr(ch, 0x35, TxPack->DLC);
            mcp_ld_txbuf(ch, 0x1, (uint8_t *)&TxPack->Data[0], TxPack->DLC);
        }
        rts(ch, 1);
        LED1_OFF(ch);

        return 0;
    }
    if (!(mcp_rd(ch, 0x40) & 0x08))
    { // ·¢ËÍ»º³åÇø1¿Õ£¿
        LED1_ON(ch);
        if (TxPack->IDE == CAN_Id_Extended)
        {
            TxPack->Id |= (1 << 31);
        }
        else
        {
            TxPack->Id &= 0x7ff;
        }
        mcp_wr_can_id(ch, 0x41, TxPack->Id);
        if (TxPack->RTR)
        {
            mcp_wr(ch, 0x45, 0x40); // ·¢ËÍRTR
        }
        else
        {
            mcp_wr(ch, 0x45, TxPack->DLC);
            mcp_ld_txbuf(ch, 0x3, (uint8_t *)&TxPack->Data[0], TxPack->DLC);
        }
        rts(ch, 2);
        LED1_OFF(ch);
        return 0;
    }
    if (!(mcp_rd(ch, 0x50) & 0x08))
    { // ·¢ËÍ»º³åÇø2¿Õ£¿
        LED1_ON(ch);
        if (TxPack->IDE == CAN_Id_Extended)
        {
            TxPack->Id |= (1 << 31);
        }
        else
        {
            TxPack->Id &= 0x7ff;
        }
        mcp_wr_can_id(ch, 0x51, TxPack->Id);
        if (TxPack->RTR)
        {
            mcp_wr(ch, 0x55, 0x40); // ·¢ËÍRTR
        }
        else
        {
            mcp_wr(ch, 0x55, TxPack->DLC);
            mcp_ld_txbuf(ch, 0x5, (uint8_t *)&TxPack->Data[0], TxPack->DLC);
        }
        rts(ch, 4);
        LED1_OFF(ch);
        return 0;
    }
    if (mcp_rd(ch, EFLG) & (1 << 5))
    {
        mcp_wr(ch, TEC, 0);
    }
    return 1; // ·¢ËÍ»º³åÇøÂú,·¢ËÍÊ§°Ü
}