/*
 * @file       DPS310.h
 * @brief      DPS310ѹ��������ͷ�ļ�
 * @author     dong
 * @version    v0.1
 * @date       2016-11-03
 */

#define DPS310_DeviceAddress  0xEE      // ���һλ��0������д���������һλ��1�����������

#define DPS310_StartAddress    0x00      // �Ĵ�����ʼ��ַ
#define DPS310_RegisterCount   41        // �Ĵ�������

#define PRODUCT_ID            0x0D      // ��Ʒ�汾��

#define COEF                  0x10      // ����ϵ��
#define COEF_COUNT            18        // ����ϵ���ĸ���

#define FIFO_STS              0x0B      // FIFO״̬  
                                        /* FIFO_FULL  - 1 or 0(1��������0������),  
                                           FIFO_EMPTY - 1 or 0(1����գ�0������)*/

#define INT_SYS               0x0A      // �ж�״̬   
                                        /* INT_FIFO_FULL - 1 or 0(1�������ж�)��
                                           INT_TMP       - 1 or 0(1�������ж�),
                                           INT_PRS       - 1 or 0(1�������ж�) */

#define CFG_REG               0x09      // �жϺ�FIFO���� 
                                        /* INT_HL      SDO�ж�ʹ��
                                           INT_FIFO    FIFO���ж�
                                           INT_TMP     �¶Ȳ���׼�����ж�
                                           INT_PRS     ѹ������׼�����ж�
                                           T_SHIFT     �¶Ȳ�������Ƿ���λ
                                           P_SHIFT     ѹ����������Ƿ���λ
                                           FIFO_EN     ʹ��FIFO
                                           SPI_MODE    ����SPIģʽ */ 

#define MEAS_CFG               0x08      // ����ģʽ��״̬
                                         /* COEF_RDY   1bit ϵ���Ƿ����
                                            SENSOR_RDY 1bit ��������ʼ���Ƿ�ɹ�
                                            TMP_RDY    1bit �µ��¶�׼����
                                            PRS_RDY    1bit �µ�ѹ��׼����
                                            reserve    1bit Ԥ��
                                            MEAS_CTRL  3bit ���ù���ģʽ
                                                  standby mode
                                                    000 - ���� / ֹͣ
                                                  command mode
                                                    001 - ѹ������
                                                    010 - �¶Ȳ���
                                                    011 - none
                                                    100 - none
                                                  background mode
                                                    101 - ����ѹ������
                                                    110 - �����¶Ȳ���
                                                    111 - ����ѹ�����¶Ȳ��� */       

#define TMP_CFG                 0x07     // �¶Ȳ�������
                                         /* TMP_EXT  ����ʹ���ڲ������������ⲿ������
                                            TMP_RATE[2:0]  ����ÿ���������
                                                  000 - ÿ�����1��
                                                  001 - ÿ�����2��
                                                  010 - ÿ�����4��
                                                  011 - ÿ�����8��
                                                  100 - ÿ�����16��
                                                  101 - ÿ�����32��
                                                  110 - ÿ�����64��
                                                  111 - ÿ�����128��
                                            TMP_PRC[3:0]  ����ÿ�β����вɼ�������ƽ��ֵ
                                                  0000 - 1��
                                                  0001 - 2��
                                                  0010 - 4��
                                                  0011 - 8��
                                                  0100 - 16��
                                                  0101 - 32��
                                                  0110 - 64��
                                                  0111 - 128��
                                                  1������ - Ԥ�� */

#define PRS_CFG                 0x06     // ѹ����������
                                         /* Reserved  Ԥ��
                                            PM_RATE[2:0]  ����ÿ���������
                                                  000 - ÿ�����1��
                                                  001 - ÿ�����2��
                                                  010 - ÿ�����4��
                                                  011 - ÿ�����8��
                                                  100 - ÿ�����16��
                                                  101 - ÿ�����32��
                                                  110 - ÿ�����64��
                                                  111 - ÿ�����128��
                                            PM_PRC[3:0]  ����ÿ�β����вɼ�������ƽ��ֵ
                                                  0000 - 1��
                                                  0001 - 2��
                                                  0010 - 4��
                                                  0011 - 8��
                                                  0100 - 16��
                                                  0101 - 32��
                                                  0110 - 64��
                                                  0111 - 128��
                                                  1������ - Ԥ�� */
                                             
#define PRS_B0              0x02
#define PRS_B1              0x01
#define PRS_B2              0x00
#define TMP_B0              0x05
#define TMP_B1              0x04
#define TMP_B2              0x03

#define PRS_STARTADDRESS    0x00
#define PRS_COUNT           3
#define TMP_STARTADDRESS    0x03
#define TMP_COUNT           3

#define DPS310_CONFIG_1     1

typedef enum DPS310_cfg
{
    CONFIG_1,
    CONFIG_2,
    CONFIG_3,
    CONFIG_4,
} DPS310_config;
     
typedef struct 
{
  int C0;
  int C1;
  int C00;
  int C10;
  int C01;
  int C11;
  int C20;
  int C21;
  int C30;
} struct_DPS310_coef;

U8 DPS310_ReadRegister( U8 registerAddress, U8 *dataSave);
U8 DPS310_ReadRegisters(U8 startAddress, U8 dataCount, U8 * dataBuff);
U8 DPS310_WriteRegister(U8 registerAddress, U8 dataBuff);
void DPS310_Config(DPS310_config configNumber);
struct_DPS310_coef DPS310_Get_Coef();
void DPS310_Get_Pressure(struct_DPS310_coef coef, U8* dataSave );

