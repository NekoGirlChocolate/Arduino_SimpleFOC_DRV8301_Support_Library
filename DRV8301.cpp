#include "DRV8301.h"

// DRV8301 class constructor
// mosi  DRV8301 SPI data in pin
// miso  DRV8301 SPI data out pin
// sclk  DRV8301 SPI clock pin
// cs  DRV8301 SPI chip select pin
// en_gate   DRV8301 enable pin
// fault  DRV8301 fault pin (pull_up)
DRV8301::DRV8301(int mosi, int miso, int sclk, int cs, int en_gate, int fault)
{
    DRV8301::drv8301_mosi_pin = mosi;
    DRV8301::drv8301_miso_pin = miso;
    DRV8301::drv8301_sclk_pin = sclk;
    DRV8301::drv8301_cs_pin = cs;
    DRV8301::drv8301_en_gate_pin = en_gate;
    DRV8301::drv8301_fault_pin = fault;
}

#pragma GCC push_options
#pragma GCC optimize("O0")  //Don't use GCC optimize
/**
 * Use for SPI timing's delay function
 * It's only test on STM32F405/F407 168MHz
 */
void DRV8301::spi_delay(void)
{
    for (int i = 0; i < 22; i++);
}
#pragma GCC pop_options

// SPI transfer 16 bit value
uint16_t DRV8301::spi_transfer(uint16_t txdata)
{
    uint16_t rxdata = 0;

    for (int i = 0; i < 16; i++)
    {
        digitalWrite(DRV8301::drv8301_mosi_pin, bitRead(txdata, 15 - i));
        digitalWrite(DRV8301::drv8301_sclk_pin, HIGH);
        DRV8301::spi_delay();
        digitalWrite(DRV8301::drv8301_sclk_pin, LOW);
        bitWrite(rxdata, 15 - i, digitalRead(DRV8301::drv8301_miso_pin));
        DRV8301::spi_delay();
    }

    return rxdata;
}

// Read DRV8301's register
int DRV8301::drv8301_read_reg(uint16_t reg)
{
    uint16_t read_data;

    digitalWrite(DRV8301::drv8301_cs_pin, LOW);
    read_data = DRV8301::spi_transfer(0x8000 | ((reg & 0x000F) << 11));
    digitalWrite(DRV8301::drv8301_cs_pin, HIGH);

    digitalWrite(DRV8301::drv8301_cs_pin, LOW);
    read_data = DRV8301::spi_transfer(0xffff);
    digitalWrite(DRV8301::drv8301_cs_pin, HIGH);

    return read_data;
}

// Set DRV8301's register
void DRV8301::drv8301_write_reg(uint16_t reg, uint16_t data)
{
    digitalWrite(drv8301_cs_pin, LOW);
    DRV8301::spi_transfer(((reg & 0x000F) << 11) | (data & 0x07FF));
    digitalWrite(drv8301_cs_pin, HIGH);
}

// Initialize pin and reset DRV8301
void DRV8301::begin(void)
{
    /** Initialize pin */
    pinMode(DRV8301::drv8301_en_gate_pin, OUTPUT);
    digitalWrite(DRV8301::drv8301_en_gate_pin, LOW);
    pinMode(DRV8301::drv8301_fault_pin, INPUT_PULLUP);
    pinMode(DRV8301::drv8301_cs_pin, OUTPUT);
    digitalWrite(DRV8301::drv8301_cs_pin, HIGH);
    pinMode(DRV8301::drv8301_mosi_pin, OUTPUT);
    pinMode(DRV8301::drv8301_miso_pin, INPUT);
    pinMode(DRV8301::drv8301_sclk_pin, OUTPUT);
    digitalWrite(DRV8301::drv8301_sclk_pin, LOW);

    /** Power UP reset timing */
    delayMicroseconds(40);
    digitalWrite(DRV8301::drv8301_en_gate_pin, HIGH);
    delay(20);
    DRV8301::drv8301_read_reg(DRV8301_STATUS_REG1);
    DRV8301::drv8301_write_reg(DRV8301_CONTROL_REG1, OCP_MODE_DISABLE | OC_ADJ_SET(27)); //Disable OC
}

// Reset all fault bits
void DRV8301::reset_all_faults(void)
{
    uint16_t reg = DRV8301::drv8301_read_reg(DRV8301_CONTROL_REG1) & 0x07FF;
    reg |= GATE_RESET_RESET_MODE;
    DRV8301::drv8301_write_reg(DRV8301_CONTROL_REG1, reg);
}

// Set DRV8301 to usage 3 pwm inputs
void DRV8301::set_3pwm_input(void)
{
    uint16_t reg = DRV8301::drv8301_read_reg(DRV8301_CONTROL_REG1) & 0x07FF;
    reg &= ~PWM_MODE_MASK;
    reg |= PWM_MODE_3_PWM_INPUTS;
    DRV8301::drv8301_write_reg(DRV8301_CONTROL_REG1, reg);
}

// Set DRV8301 to usage 6 pwm inputs
void DRV8301::set_6pwm_input(void)
{
    uint16_t reg = DRV8301::drv8301_read_reg(DRV8301_CONTROL_REG1) & 0x07FF;
    reg &= ~PWM_MODE_MASK;
    reg |= PWM_MODE_6_PWM_INPUTS;
    DRV8301::drv8301_write_reg(DRV8301_CONTROL_REG1, reg);
}

// Detect if DRV8301 has fault occurred
// retval   0:no faults 1:has faults
int DRV8301::is_fault(void)
{
    return (int)!digitalRead(DRV8301::drv8301_fault_pin);
}

// Get DRV8301's chip id
// retval   chip id
int DRV8301::get_id(void)
{
    return DRV8301::drv8301_read_reg(DRV8301_STATUS_REG2) & 0x000F;
}
