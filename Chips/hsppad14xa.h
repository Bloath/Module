#ifndef _HSPPAD14XA_H_ 
#define _HSPPAD14XA_H_

/* Includes ------------------------------------------------------------------*/
#include "Module/Module_Conf.h"

/* typedef -------------------------------------------------------------------*/
struct Hsppad14xaFifoStruct
{
    int32_t pressure[16];
    int32_t temperature;
    uint16_t pCount;
};

struct Hsppad14xaStruct
{
    struct SoftI2cStruct *i2c;
    uint8_t devAddress;
    uint8_t coef;
    
    struct Hsppad14xaFifoStruct *fifo;
    
    void (*CallBack_ReadFifo)(struct Hsppad14xaStruct *);
};
/* define --------------------------------------------------------------------*/

#define HSPPAD14X_WIA      	0x00
#define HSPPAD14X_INFO      0x01
#define HSPPAD14X_FFST      0x02
#define HSPPAD14X_STAT      0x03
#define HSPPAD14X_POUTL     0x04
#define HSPPAD14X_POUTM     0x05
#define HSPPAD14X_POUTH     0x06
#define HSPPAD14X_TOUTL     0x09
#define HSPPAD14X_TOUTH     0x0A
#define HSPPAD14X_CTL1      0x0E
#define HSPPAD14X_CTL2      0x0F
#define HSPPAD14X_ACTL1     0x10
#define HSPPAD14X_ACTL2     0x11
#define HSPPAD14X_FCTL      0x12
#define HSPPAD14X_AVCL      0x13
#define HSPPAD14X_PNUM      0x1C
#define HSPPAD14X_PDET      0x20
#define HSPPAD14X_TDET      0x22
#define HSPPAD14X_SRST      0x26
#define HSPPAD14X_PTDET     0x29


#define HSPPAD14X_CONFIG_ODR_OFFSET		(8 + 2)
#define HSPPAD14X_CONFIG_ODR_1HZ		(uint32_t)(0 << HSPPAD14X_CONFIG_ODR_OFFSET)
#define HSPPAD14X_CONFIG_ODR_10HZ		(uint32_t)(1 << HSPPAD14X_CONFIG_ODR_OFFSET)
#define HSPPAD14X_CONFIG_ODR_100HZ		(uint32_t)(2 << HSPPAD14X_CONFIG_ODR_OFFSET)
#define HSPPAD14X_CONFIG_ODR_200HZ		(uint32_t)(3 << HSPPAD14X_CONFIG_ODR_OFFSET)

#define HSPPAD14X_CONFIG_PTAP_OFFSET	(16)
#define HSPPAD14X_CONFIG_PTAP_ULP		(uint32_t)(0 << HSPPAD14X_CONFIG_PTAP_OFFSET)
#define HSPPAD14X_CONFIG_PTAP_LP		(uint32_t)(1 << HSPPAD14X_CONFIG_PTAP_OFFSET)
#define HSPPAD14X_CONFIG_PTAP_HA		(uint32_t)(2 << HSPPAD14X_CONFIG_PTAP_OFFSET)
#define HSPPAD14X_CONFIG_PTAP_UA		(uint32_t)(3 << HSPPAD14X_CONFIG_PTAP_OFFSET)


#define HSPPAD14X_CONFIG_TFRQ_OFFSET	(0 + 3)
#define HSPPAD14X_CONFIG_TFRQ_OFT		(uint32_t)(0 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E64		(uint32_t)(1 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E32		(uint32_t)(2 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E16		(uint32_t)(3 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E8		(uint32_t)(4 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E4		(uint32_t)(5 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E2		(uint32_t)(6 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_TFRQ_E1		(uint32_t)(7 << HSPPAD14X_CONFIG_TFRQ_OFFSET)

#define HSPPAD14X_CONFIG_AVG_OFFSET		(0)
#define HSPPAD14X_CONFIG_AVG_NONE		(uint32_t)(0 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E2			(uint32_t)(1 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E4			(uint32_t)(2 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E8			(uint32_t)(3 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E16		(uint32_t)(4 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E32		(uint32_t)(5 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E64		(uint32_t)(6 << HSPPAD14X_CONFIG_TFRQ_OFFSET)
#define HSPPAD14X_CONFIG_AVG_E128		(uint32_t)(7 << HSPPAD14X_CONFIG_TFRQ_OFFSET)

#define HSPPAD14X_CONFIG_CTL2_OFFSET    8
#define HSPPAD14X_CONFIG_CTL2_CONTINUS  (uint32_t)(0xA1 << HSPPAD14X_CONFIG_CTL2_OFFSET)
#define HSPPAD14X_FIFO_SIZE		1024

/* macro ---------------------------------------------------------------------*/
/* variables -----------------------------------------------------------------*/
/* function prototypes -------------------------------------------------------*/
/* functions -----------------------------------------------------------------*/
int Hsppad14xa_Init(struct Hsppad14xaStruct *hsppad14xa, uint32_t config);
int Hsppad14xa_BackgroudRead(struct Hsppad14xaStruct *hsppad14xa);

#endif