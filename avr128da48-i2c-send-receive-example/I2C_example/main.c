/* 
    (c) 2019 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/
#define F_CPU                                           4000000     // 4MHz default clock

#include <avr/io.h>
#include <util/delay.h>

#define CLK_PER                                         4000000     // 4MHz default clock
#define TWI0_BAUD(F_SCL, T_RISE)                        ((((((float)CLK_PER / (float)F_SCL)) - 10 - ((float)CLK_PER * T_RISE))) / 2)

#define I2C_SCL_FREQ                                    100000
#define I2C_SLAVE_ADDRESS                               0x4D

#define I2C_DIRECTION_BIT_WRITE                         0
#define I2C_DIRECTION_BIT_READ                          1
#define I2C_SET_ADDR_POSITION(address)                  (address << 1)           
#define I2C_SLAVE_RESPONSE_ACKED                        (!(TWI_RXACK_bm & TWI0.MSTATUS))
#define I2C_DATA_RECEIVED                               (TWI_RIF_bm & TWI0.MSTATUS)

#define CREATE_16BIT_VARIABLE(HIGH_BYTE, LOW_BYTE)      ((HIGH_BYTE << 8) | (LOW_BYTE & 0xFF))

static void I2C_0_init(void);
static void I2C_0_transmittingAddrPacket(uint8_t slaveAddres, uint8_t directionBit);
static uint8_t I2C_0_receivingDataPacket(void);
static void I2C_0_sendMasterCommand(uint8_t newCommand);
static void I2C_0_setACKAction(void);
static void I2C_0_setNACKAction(void);

int main(void)
{
    volatile uint16_t potentiometer12BitValue = 0;
    volatile uint8_t potentiometerUpperDataByte = 0;
    volatile uint8_t potentiometerLowerDataByte = 0;
    
    I2C_0_init();
    
    while (1) 
    {
        I2C_0_transmittingAddrPacket(I2C_SLAVE_ADDRESS, I2C_DIRECTION_BIT_READ);
      
        potentiometerUpperDataByte = I2C_0_receivingDataPacket();
        I2C_0_setACKAction();
        I2C_0_sendMasterCommand(TWI_MCMD_RECVTRANS_gc);         
            
        potentiometerLowerDataByte = I2C_0_receivingDataPacket();
        I2C_0_setNACKAction();
        I2C_0_sendMasterCommand(TWI_MCMD_STOP_gc);
        
    
        potentiometer12BitValue =  CREATE_16BIT_VARIABLE(potentiometerUpperDataByte, potentiometerLowerDataByte);
        
        _delay_ms(500);
    }
}

static void I2C_0_init(void)
{
    TWI0.MBAUD = (uint8_t)TWI0_BAUD(I2C_SCL_FREQ, 0);

    TWI0.MCTRLA = TWI_ENABLE_bm;
    
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

static void I2C_0_transmittingAddrPacket(uint8_t slaveAddres, uint8_t directionBit)
{
    TWI0.MADDR = I2C_SET_ADDR_POSITION(slaveAddres) + directionBit;
    while (!I2C_SLAVE_RESPONSE_ACKED)
    {
        ;
    }    
}

static uint8_t I2C_0_receivingDataPacket(void)
{
    while (!I2C_DATA_RECEIVED)
    {
        ;
    }

    return TWI0.MDATA;    
}

static void I2C_0_sendMasterCommand(uint8_t newCommand)
{
    TWI0.MCTRLB |=  newCommand;
}

static void I2C_0_setACKAction(void)
{
    TWI0.MCTRLB &= !TWI_ACKACT_bm;
}

static void I2C_0_setNACKAction(void)
{
    TWI0.MCTRLB |= TWI_ACKACT_bm;
}
