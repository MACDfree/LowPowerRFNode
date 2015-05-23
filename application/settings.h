#ifndef setting_h
#define setting_h


// 定义条件编译标识
/*
节点工作模式分为汇集节点（MODE_HJ）和采集节点（MODE_CJ）
MODE_TEST为测试标志
如果定义测试标志，则MODE_OLED或MODE_UART也需要定义（两者可同时定义）
*/
// *需要修改*
//#define MODE_TEST // 测试模式
//#define MODE_HJ // 汇集节点模式
#define MODE_CJ // 采集节点模式
//#define MODE_OLED // 使用OLED
//#define MODE_UART // 使用串口



// 定义节点基本信息
// *需要修改*
#define ADDR_CJ 13 // 采集节点地址，设置范围11~14

#define ADDR_HJ 10 // 汇集节点地址为10
#define ADDR_CJ1 11 // 采集节点1地址

#define ADDR_B 0 // 广播地址为0
#define KEY_H 111 // 密钥高8位
#define KEY_L 111 // 密钥低8位

#ifdef MODE_HJ
#define LEN 7
#define ADDR ADDR_HJ
#define TIMEOUT 2000
#endif

#ifdef MODE_CJ
#define LEN 4
#define ADDR ADDR_CJ
#define TIMEOUT (2000*4)
#endif

#endif