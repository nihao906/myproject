#include <string.h>
#include <stdint.h>
#include "ModbusM.h"
#include "ModbusS.h"
// #include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "uart.h"
#include "driver/uart.h"
#include "esp_log.h"
static const char *TAG = "UART1";

#define ZB_CHANNEL 1
static char hex[1514 * 3 + 2];

static char *bin2hex(uint8_t *bin, int lenght)
{

    if (lenght > sizeof(hex) / 3)
    {
        lenght = sizeof(hex) / 3;
    }
    int i;
    for (i = 0; i < lenght; i++)
    {
        sprintf(hex + i * 3, "%02X ", bin[i]);
    }
    hex[i * 3] = '\0';
    return hex;
}
extern uint16_t gWordVar[];
//uint16_t g_tmp[100];
device_t zb_device[ZB_CHANNEL];

send_t zb_send[16];

#define GetTicks xTaskGetTickCount
#define UART1_TXBUF_SIZE  256

int zbapi_send_write(device_t *device, send_t *sender);
int zbapi_send_req(device_t *device);
int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}

//static float pow_user(float x, float y)
//{
//    int i;
//    float tmp = 1;
//    for (i = 0; i < y; i++)
//    {
//        tmp = tmp * x;
//    }
//    return tmp;
//}
uint16_t bin2bcd(uint16_t bin)
{
    uint16_t bcd;
    uint8_t buf[4];
    if (bin > 9999)
    {
        bin = 9999;
    }
    buf[0] = bin % 10;
    bin /= 10;
    buf[1] = bin % 10;
    bin /= 10;
    buf[2] = bin % 10;
    buf[3] = bin / 10;
    bcd = buf[0] | (buf[1] << 4) | (buf[2] << 8) | (buf[3] << 12);
    return bcd;
}
int min(int x, int y)
{
    if (x < y)
        return x;
    else
        return y;
}
// ��������sn ֵ
uint8_t get_new_sn(int device_id)
{
    int i;
    int sn = 0;
    for (i = 0; i < 16; i++)
    {
        if (zb_send[i].device_id == device_id)
        {
            sn = max(zb_send[i].sn, sn);
        }
    }
    return sn + 1;
}

// �����豸��Сsn��Ӧ���±�
int device_get_send(int device_id)
{
    int i;
    int sn = 255;
    int min_index = -1;
    for (i = 0; i < 16; i++)
    {
        if (zb_send[i].device_id == device_id)
        {
            if (zb_send[i].sn < sn)
            {
                sn = zb_send[i].sn;
                min_index = i;
            }
        }
    }
    return min_index;
}
void on_in_pack_error(device_t *device)
{
    if (device->in_err_cnt[device->in_index] < device->in->max_err_cnt)
    {
        if (++device->in_err_cnt[device->in_index] >= device->in->max_err_cnt)
        {
            const packet_t *in_pack = device->in->packets + device->in_index;
            int i;
            for (i = 0; i < in_pack->length; i++)
            {
                uint32_t reg_addr = in_pack->local_addr + i;
                if (reg_addr < gWORD_SIZE)
                {
                    gWordVar[reg_addr] = 0;
                }
            }
            if (device->protocol == 255)
            {
                if (in_pack->slave > 0 && in_pack->slave <= 10)
                {
                    int i;
                    for (i = 0; i < 10; i++)
                    {
                        gWordVar[in_pack->local_addr] = 0;
                    }
                }
                if (in_pack->slave == 0)
                {
                    gWordVar[64 + 10] = 0;
                }
            }
            // on_data_error(device);
        }
    }
}
extern int page;
void modbus_master_poll(int n)
{
    uint32_t now = GetTicks();
    // printf("status == %d\n",zb_device[n].status);
    if (zb_device[n].status == 0) // 空闲状态
    {
        if (zb_device[n].out_pending > 0) ////如果有发送请求先处理发送
        {
            // printf("out_pending > 0\n");
            int send_index = device_get_send(n);
            if (send_index >= 0)
            {
                if (now >= zb_send[send_index].next_try_time)
                {
                    zb_device[n].out_index = send_index;
                    zb_device[n].send_req_time = now;
                    zbapi_send_write(&zb_device[n], &zb_send[send_index]);
                    zb_device[n].status = 0x10;
                    return;
                }
            }
            else
            {
                zb_device[n].out_pending = 0;
            }
        }
        if (now >= zb_device[n].next_scan_time)
        {
            // printf("now >= zb_device[%d]\n",n);
            zb_device[n].send_req_time = now;
            zbapi_send_req(&zb_device[n]);
            zb_device[n].status = 2;
        }
    }
    else if (zb_device[n].status == 1 || zb_device[n].status == 2) //
    {

        if ((now - zb_device[n].send_req_time) > zb_device[n].in->time_out * 100)
        {
            zb_device[n].status = 0; //��ʱ
            zb_device[n].ErrCode = 0x01;
            zb_device[n].ErrCount++;
            on_in_pack_error(&zb_device[n]);
            if ((++zb_device[n].try_times >= zb_device[n].in->max_try_times) ||
                (zb_device[n].in_err_cnt[zb_device[n].in_index] >= zb_device[n].in->max_err_cnt))
            {
                if (++zb_device[n].in_index >= zb_device[n].in->packet_cnt)
                {
                    zb_device[n].in_index = 0;
                    zb_device[n].group_start_time = now;
                }
                //zb_device[n].next_scan_time = zb_device[n].group_start_time + zb_device[n].in->scan_rate*10;
                zb_device[n].try_times = 0;
            }
            zb_device[n].next_scan_time = zb_device[n].group_start_time + zb_device[n].in->scan_rate * 10;
            zb_device[n].status = 0;
        }
    }
    else if (zb_device[n].status == 3) // ���յ��ظ�
    {
        if (++zb_device[n].in_index >= zb_device[n].in->packet_cnt)
        {
            zb_device[n].in_index = 0;
            zb_device[n].group_start_time = now; //+ zb_device[n].in->scan_rate*10;
        }
        zb_device[n].next_scan_time = zb_device[n].group_start_time + zb_device[n].in->scan_rate * 10;
        zb_device[n].status = 255;
        zb_device[n].try_times = 0;
    }
    else if (zb_device[n].status == 0x10) // �������������ѷ����ȴ���Ӧ���߳�ʱ
    {
        int index = zb_device[n].out_index;
        if ((now - zb_device[n].send_req_time) > zb_device[n].in->time_out * 100)
        {
            zb_device[n].status = 0; //��ʱ
            zb_device[n].ErrCode = 0x01;
            zb_device[n].ErrCount++;
            if (++zb_send[index].try_times >= zb_device[n].out->max_try_times) //����������Դ���������ʧ��
            {
                zb_send[index].device_id = 0xff;
            }
            else
            {
                zb_send[index].next_try_time = now + zb_device[n].out->try_space * 100;
            }
            zb_device[n].status = 0;
            zb_device[n].try_times = 0;
        }
    }
    else if (zb_device[n].status == 0x11) // ���յ���������ȷ��
    {
        zb_send[zb_device[n].out_index].device_id = 0xff;
        zb_device[n].status = 255;
        zb_device[n].try_times = 0;
    }
    else
    {
        if ((now - zb_device[n].revice_resp_time) >= (zb_device[n].in->scan_space))
        {
            zb_device[n].status = 0;
            zb_device[n].try_times = 0;
        }
    }
}
// �����ཻ
int intersection(int x0, int x1, int y0, int y1, int *s, int *e)
{
    int ret = 0;
    if ((y0 >= x0) && (y0 < x1))
    {
        *s = y0;
        *e = min(x1, y1);
        ret = 1;
    }
    else if ((y1 > x0) && (y1 <= x1))
    {
        *s = y1 - 1;
        *e = min(x1, y1);
        ret = 1;
    }
    else if ((x0 >= y0) && (x0 < y1))
    {
        *s = x0;
        *e = min(y1, x1);
        ret = 1;
    }
    else if ((x1 > y0) && (x1 <= y1))
    {
        *s = x1 - 1;
        *e = min(y1, x1);
        ret = 1;
    }
    return ret;
}
int zb_ModBusWordWriteHook(uint16_t addr, uint16_t length)
{
    int i, j;
    int ret = 0;
    for (i = 0; i < ZB_CHANNEL; i++)
    {
        if (zb_device[i].out == NULL)
        {
            continue;
        }
        for (j = 0; j < zb_device[i].out->packet_cnt; j++)
        {
            int s1, e1;
            int s2, e2;
            int s, e;
            int n;
            s1 = zb_device[i].out->packets[j].local_addr;
            e1 = zb_device[i].out->packets[j].local_addr + zb_device[i].out->packets[j].length;
            s2 = addr;
            e2 = addr + length;
            if (intersection(s1, e1, s2, e2, &s, &e))
            //if((s2 >= s1 && s2 < e1) || (e2 >= s1 && s2 < e1))
            {
                for (n = 0; n < 16; n++)
                {
                    if (zb_send[n].device_id == 0xff)
                    {
                        zb_send[n].sn = get_new_sn(i);
                        zb_send[n].device_id = i;
                        zb_send[n].packet_index = j;
                        zb_send[n].address = s;
                        zb_send[n].length = e - s;
                        zb_send[n].next_try_time = 0;
                        zb_send[n].try_times = 0;
                        zb_device[i].out_pending++;
                        ret = 1;
                        break;
                    }
                }
            }
        }
    }
    return ret;
}

//extern uint8_t Uart3TxBuf[];
//#define txbuf   Uart3TxBuf

int zbapi_send_req(device_t *device)
{
    if (device->protocol == 255)
    {
        device->txbuf[0] = device->in->packets[device->in_index].slave;
        device->txbuf[1] = 0x5a;
        device->txbuf[2] = 0xaa;
        device->write(device->txbuf, 3);
    }
    else
    {
        uint16_t crc;
        device->txbuf[0] = device->in->packets[device->in_index].slave;
        device->txbuf[1] = device->in->packets[device->in_index].funcode;
        device->txbuf[2] = device->in->packets[device->in_index].address >> 8;
        device->txbuf[3] = device->in->packets[device->in_index].address & 0xff;
        device->txbuf[4] = device->in->packets[device->in_index].length >> 8;
        device->txbuf[5] = device->in->packets[device->in_index].length & 0xff;
        crc = crc16(device->txbuf, 6);
        device->txbuf[6] = crc >> 8;
        device->txbuf[7] = crc & 0xff;
        device->write(device->txbuf, 8);
    }
    return 0;
}
int zbapi_send_write(device_t *device, send_t *sender)
{
    {
        int n = 0;
        uint16_t crc;
        const packet_t *out_pack = &device->out->packets[sender->packet_index];
        if (sender->length == 0)
        {
            return 0;
        }
        device->txbuf[0] = device->out->packets[sender->packet_index].slave;
        if (sender->length > 1)
        {
            int addr = out_pack->address + (sender->address - out_pack->local_addr);
            int i;
            int len;
            device->txbuf[1] = 0x10;
            device->txbuf[2] = addr >> 8;
            device->txbuf[3] = addr & 0xff;
            device->txbuf[4] = sender->length >> 8;
            device->txbuf[5] = sender->length & 0xff;
            device->txbuf[6] = (sender->length * 2) & 0xff;
            len = 7;
            for (i = 0; i < sender->length; i++)
            {
                uint16_t reg16 = gWordVar[sender->address + i];
                if (out_pack->device_type == 5)
                {
                    reg16 = bin2bcd(reg16);
                }
                device->txbuf[len++] = reg16 >> 8;
                device->txbuf[len++] = reg16 & 0xff;
            }
            crc = crc16(device->txbuf, len);
            device->txbuf[len++] = crc >> 8;
            device->txbuf[len++] = crc & 0xff;
            n += len;
        }
        else
        {
            int addr = out_pack->address + (sender->address - out_pack->local_addr);
            uint16_t reg16 = gWordVar[sender->address];
            device->txbuf[1] = 0x06;
            device->txbuf[2] = addr >> 8;
            device->txbuf[3] = addr & 0xff;
            if (out_pack->device_type == 5)
            {
                reg16 = bin2bcd(reg16);
            }
            if ((device->protocol == 5) || (device->protocol == 6))
            {
                reg16 *= 10;
            }
            device->txbuf[4] = reg16 >> 8;
            device->txbuf[5] = reg16 & 0xff;
            crc = crc16(device->txbuf, 6);
            device->txbuf[6] = crc >> 8;
            device->txbuf[7] = crc & 0xff;
            n += 8;
        }
        if (device->write != NULL)
        {
            device->write(device->txbuf, n);
        }

    }
    return 1;
}

extern uint16_t Is_SetFromScreen;

int modbus_master_on_revice(int ch, uint8_t *rxbuf, int length)
{
    int i;
//    uint8_t src_addr[8];
    if (length < 4)
    {
        return -1;
    }
    i = ch;
    //for(i=0; i<ZB_CHANNEL; i++)//1
    {
        if (zb_device[i].status == 2)
        {
            zb_device[i].revice_resp_time = GetTicks();
            {
                int index = zb_device[i].in_index;
                if ((zb_device[i].in->packets[index].slave == rxbuf[0]) &&
                    zb_device[i].in->packets[index].funcode == rxbuf[1])
                {
                    uint16_t crcChk = crc16(rxbuf, length - 2);
                    uint16_t crcData = (rxbuf[length - 2] << 8) | rxbuf[length - 1];
                    if (crcData != crcChk)
                    {
                        zb_device[i].ErrCode = 2; //CRC error
                        zb_device[i].ErrCount++;
                    }
                    else if (rxbuf[0] != zb_device[i].in->packets[index].slave)
                    {
                        zb_device[i].ErrCode = 3; //ADD error
                        zb_device[i].ErrCount++;
                    }
                    else if (rxbuf[1] != zb_device[i].in->packets[index].funcode)
                    {
                        zb_device[i].ErrCode = 4; //Fun err;
                        zb_device[i].ErrCount++;
                    }
                    else if (rxbuf[2] != (zb_device[i].in->packets[index].length * 2))
                    {
                        zb_device[i].ErrCode = 1; //byte miss
                        zb_device[i].ErrCount++;
                    }
                    else
                    {
                        int n = 0;
                       // uint32_t reg32;
                        uint16_t addr = zb_device[i].in->packets[index].local_addr;
                        uint8_t len = rxbuf[2] / 2;
                        if (addr >= gWORD_SIZE)
                        {
                            zb_device[i].ErrCode = 5; //regaddr err;
                            goto done;
                        }
                        zb_device[i].ErrCode = 0;
                        switch (zb_device[i].in->packets[index].device_type)
                        {
                        default:

                            for (n = 0; n < len; n++)
                            {
                                if (addr + n < gWORD_SIZE)
                                {
                                    gWordVar[addr + n] = (rxbuf[3 + n * 2] << 8) | (rxbuf[3 + n * 2 + 1]);
                                }
                                else
                                {
                                    break;
                                }
                            }
                            break;
                        }
                        zb_device[i].in_err_cnt[zb_device[i].in_index] = 0;
                    }
                done:
                    zb_device[i].status = 3;
                }
            }
        }
        else if (zb_device[i].status == 0x10)
        {
            zb_device[i].revice_resp_time = GetTicks();
            //if(memcmp(src_addr,((zb_dev_t*)zb_device[i].ext)->zb_addr,8) == 0)
            {
                zb_device[i].status = 0x11;
            }
        }
    }
    return 0;
}

//int meter_count = 4;

//#define meter_count meter_cfg.count
//const int reg_offset[] = {0, 2, 3, 4, 1};

//static char input_buf[1][414];
//static char output_buf[1][212];
static char in_err_cnt_buf[1][25];
struct input{
	input_t input;
	packet_t packet[5];
};
struct output{
	output_t output;
	packet_t packet[5];
};
struct input input1;
struct output output1;

extern int uart1_tx_start(uint8_t *data, int length);
 uint8_t uart1_txbuf[UART1_TXBUF_SIZE];
//static uint8_t           uart_txbuf[ 256 ];
//static uint8_t           uart_rxbuf[ 256 ];

static int uart_send(uint8_t *buf, int len)
{
    // uart1_tx_start(buf, len);
    uart_write_bytes(1, buf, len);
    //HAL_UART_Transmit_DMA(&huart1,buf,len);
    return len;
}
int ModbusM_init(void)
{
    int i = 0;
//    int meter_count[4] = {0, 0, 0, 0};
//    int size;
//    char *p;
 //   int n;
//    uint8_t *in_err_cnt;
     
    zb_device[0].txbuf = uart1_txbuf;
    zb_device[0].txbuf_size = UART1_TXBUF_SIZE;
    zb_device[0].write = uart_send;
    

    {
        input_t *input;
        output_t *output;
        uint8_t *in_err_cnt;


        //  memset(input_buf[i], 0, sizeof(input_buf[i]));
        // memset(output_buf[i], 0, sizeof(output_buf[i]));
        //  memset(in_err_cnt_buf[i], 0, sizeof(in_err_cnt_buf[i]));

        output = &output1.output;
        input = &input1.input;
        in_err_cnt = (uint8_t *)in_err_cnt_buf[i];

        zb_device[i].in_err_cnt = in_err_cnt;

        input->scan_rate = 10; //*10ms
        input->time_out = 10;   //*100ms
        input->max_try_times = 2;
        input->scan_space = 50; //*10ms
        input->max_err_cnt = 30;

        output->max_try_times = 3;
        output->try_space = 40;

        zb_device[i].channl = i;
        zb_device[i].next_scan_time = 0;
        zb_device[i].status = 0;
        zb_device[i].try_times = 0;
        zb_device[i].in_index = 0;
        zb_device[i].out_pending = 0;
        zb_device[i].in_err_cnt = in_err_cnt;
        zb_device[i].in = input;
        zb_device[i].out = output;
        // zb_device[i].ext = &serial_port_cfg[i];
    }
    {
        device_t *dev = (device_t *)&zb_device;
        dev->in->packets[dev->in->packet_cnt].slave = 2;
        dev->in->packets[dev->in->packet_cnt].funcode = 3;
        dev->in->packets[dev->in->packet_cnt].address = 0;
        dev->in->packets[dev->in->packet_cnt].length = 3;
        dev->in->packets[dev->in->packet_cnt].device_type = 0;
        dev->in->packets[dev->in->packet_cnt].local_addr = TILT_SENSER_ADDR;
        dev->in->packets[dev->in->packet_cnt].ch = 0;
        dev->in->packet_cnt++;

        // dev->in->packets[dev->in->packet_cnt].slave = 2;
        // dev->in->packets[dev->in->packet_cnt].funcode = 3;
        // dev->in->packets[dev->in->packet_cnt].address = 5;
        // dev->in->packets[dev->in->packet_cnt].length = 1;
        // dev->in->packets[dev->in->packet_cnt].device_type = 0;
        // dev->in->packets[dev->in->packet_cnt].local_addr = 82;
        // dev->in->packets[dev->in->packet_cnt].ch = 0;
        // dev->in->packet_cnt++;

        dev->out->packets[dev->out->packet_cnt].slave = 2;
        dev->out->packets[dev->out->packet_cnt].funcode = 6;
        dev->out->packets[dev->out->packet_cnt].local_addr = 85;
        dev->out->packets[dev->out->packet_cnt].length = 12;
        dev->out->packets[dev->out->packet_cnt].address = 20;
        dev->out->packets[dev->out->packet_cnt].device_type = 0;
        dev->in->packets[dev->in->packet_cnt].ch = 0;
        dev->out->packet_cnt++;

        dev->out->packets[dev->out->packet_cnt].slave = 2;
        dev->out->packets[dev->out->packet_cnt].funcode = 6;
        dev->out->packets[dev->out->packet_cnt].local_addr = 81;
        dev->out->packets[dev->out->packet_cnt].length = 4;
        dev->out->packets[dev->out->packet_cnt].address = 5;
        dev->out->packets[dev->out->packet_cnt].device_type = 0;
        dev->in->packets[dev->in->packet_cnt].ch = 0;
        dev->out->packet_cnt++;
    }
    return 0;
}
extern int meter_type;

void ModbusM_reinit(void)
{
    int i;
    for (i = 64; i < 64 + 128; i++)
    {
        gWordVar[i] = 0;
    }
    ModbusM_init();
}

int fill_data(uint8_t *pData, uint16_t local_addr, uint8_t remote_offset, uint8_t length)
{
    uint16_t *pWordVar;
    int i;
    *pData++ = remote_offset;
    *pData++ = length;
    pWordVar = &gWordVar[local_addr];
    for (i = 0; i < length; i++)
    {
        *pData++ = *pWordVar >> 8;
        *pData++ = *pWordVar & 0xff;
        pWordVar++;
    }
    return i * 2 + 2;
}
int build_data(int dest_addr, const int *src_addr_map, int length)
{
    int i;
    int change_cnt = 0;
    for (i = 0; i < length; i++)
    {
        int src_addr = src_addr_map[i];
        if (dest_addr < gWORD_SIZE && src_addr >= 0 && src_addr < gWORD_SIZE)
        {
            if (gWordVar[dest_addr] != gWordVar[src_addr])
            {
                gWordVar[dest_addr] = gWordVar[src_addr];
                change_cnt++;
            }
        }
        else
        {
            return -1;
        }
        dest_addr++;
    }
    return change_cnt;
}
int copy_data(int dest_addr, int src_addr, int length)
{
    int i;
    int change_cnt = 0;
    for (i = 0; i < length; i++)
    {
        //int src_addr = src_addr_map[i];
        if (dest_addr < gWORD_SIZE && src_addr >= 0 && src_addr < gWORD_SIZE)
        {
            if (gWordVar[dest_addr] != gWordVar[src_addr])
            {
                gWordVar[dest_addr] = gWordVar[src_addr];
                change_cnt++;
            }
        }
        else
        {
            return -1;
        }
        dest_addr++;
        src_addr++;
    }
    return change_cnt;
}
