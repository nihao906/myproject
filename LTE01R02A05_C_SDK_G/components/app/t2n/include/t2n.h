#ifndef _T2N_H_
#define _T2N_H_
#ifndef ontainer_of
#define container_of(ptr, type, member)                \
  ({                                                   \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); \
  })
#endif
#define t2n_trace printf
// #define HTONS(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))
extern void tun_poll(void);
extern int t2n_input(struct pbuf *pkt_buf);
int t2n_send_heartbeat(uint8_t flags);
void *t2n_buf_alloc(int size);
uint32_t t2n_clock(void);

extern uint32_t readUint32LE(uint8_t *buf, int offset);
extern uint32_t readUint32BE(uint8_t *buf, int offset);
extern int32_t readInt32LE(uint8_t *buf, int offset);
extern int32_t readInt32BE(uint8_t *buf, int offset);
extern uint16_t readUInt16LE(uint8_t *buf, int offset);
extern uint16_t readUInt16BE(uint8_t *buf, int offset);
extern int16_t readInt16LE(uint8_t *buf, int offset);
extern int16_t readInt16BE(uint8_t *buf, int offset);
extern void writeUInt16BE(uint8_t *buf, int offset, uint16_t data);
extern void writeUInt16LE(uint8_t *buf, int offset, uint16_t data);
extern void writeUInt32BE(uint8_t *buf, int offset, uint32_t data);
extern void writeUInt32LE(uint8_t *buf, int offset, uint32_t data);
extern uint8_t readUInt8(uint8_t *buf, int offset);
extern int8_t readInt8(uint8_t *buf, int offset);
extern void writeUInt8(uint8_t *buf, int offset, uint8_t data);

#endif // _T2N_H_
