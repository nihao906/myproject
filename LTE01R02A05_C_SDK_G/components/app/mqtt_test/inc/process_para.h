#ifndef PROCESS_PARA_H
#define PROCESS_PARA_H

int process_0parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_1parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_2parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_3parameter_instruction(const unsigned char *payload, unsigned short payload_len);
int process_4parameter_instruction(const unsigned char *payload, unsigned short payload_len);

#endif