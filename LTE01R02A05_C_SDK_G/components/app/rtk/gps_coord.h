
#ifndef __GPS_COORD_H
#define __GPS_COORD_H

#define REG_GPS_MESSAGE (512 + 48)
#define  REG_GPS_WRITE_DATA (REG_GPS_MESSAGE + sizeof(gps_t))
#define WRITE_SAVE_MAGIC	0x55AA





extern struct Point target_point;
extern struct Point target_point_d;
extern struct Point target_res_point;
extern struct Point flat_res_point;
extern struct Point result_point;
void modbus_write_gps_data_cb(uint16_t *addr, int len);



#endif