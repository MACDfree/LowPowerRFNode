#ifndef setting_h
#define setting_h


// �������������ʶ
/*
�ڵ㹤��ģʽ��Ϊ�㼯�ڵ㣨MODE_HJ���Ͳɼ��ڵ㣨MODE_CJ��
MODE_TESTΪ���Ա�־
���������Ա�־����MODE_OLED��MODE_UARTҲ��Ҫ���壨���߿�ͬʱ���壩
*/
// *��Ҫ�޸�*
//#define MODE_TEST // ����ģʽ
//#define MODE_HJ // �㼯�ڵ�ģʽ
#define MODE_CJ // �ɼ��ڵ�ģʽ
//#define MODE_OLED // ʹ��OLED
//#define MODE_UART // ʹ�ô���



// ����ڵ������Ϣ
// *��Ҫ�޸�*
#define ADDR_CJ 13 // �ɼ��ڵ��ַ�����÷�Χ11~14

#define ADDR_HJ 10 // �㼯�ڵ��ַΪ10
#define ADDR_CJ1 11 // �ɼ��ڵ�1��ַ

#define ADDR_B 0 // �㲥��ַΪ0
#define KEY_H 111 // ��Կ��8λ
#define KEY_L 111 // ��Կ��8λ

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