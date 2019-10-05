//UART1.h
//Dung Nguyen
//Wally Guzman

void UART1_Init(uint32_t baud);

void UART1_OutChar(char data);

void UART1_OutString(char *pt);

void UART1_FIFOtoBuffer(void);
