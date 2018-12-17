//orginal code by bitluni (send him a high five if you like the code)

// Dirty modded for bench testing Shift registers by Agge

#include <soc/rtc.h>
#include "driver/i2s.h"

static const i2s_port_t i2s_num = (i2s_port_t)I2S_NUM_0; // i2s port number

//static i2s_config_t i2s_config;
static const i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX ),
     .sample_rate = 1000000,  //not really used
     .bits_per_sample = (i2s_bits_per_sample_t)I2S_BITS_PER_SAMPLE_16BIT, 
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
     .dma_buf_count = 2,
     .dma_buf_len = 64  //Small buffer 
};

static const i2s_pin_config_t pin_config = {
	.bck_io_num = 26, //this is BCK pin
	.ws_io_num = 25, // this is LRCK pin
	.data_out_num = 22, // this is DATA output pin
	.data_in_num = -1   //Not used
};
void setup() 
{
  Serial.begin(115200);
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);              //highest cpu frequency
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);    //start i2s driver
  i2s_set_pin(i2s_num, &pin_config);                           //use external shift register
  i2s_set_sample_rates(i2s_num, 1000000);               //dummy sample rate, since the function fails at high values
														// the API states that this value is an int!!!	Agge

  //this is the hack that enables the highest sampling rate possible ~13MHz, have fun

  // Gives BCK @ ~25Mhz , WCK @ ~833KHz and Step Rate @ ~416KHz, Can we go higher ?
  SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_A_V, 1, I2S_CLKM_DIV_A_S);
  SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_B_V, 1, I2S_CLKM_DIV_B_S);
  SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_NUM_V, 2, I2S_CLKM_DIV_NUM_S); 
  SET_PERI_REG_BITS(I2S_SAMPLE_RATE_CONF_REG(0), I2S_TX_BCK_DIV_NUM_V, 2, I2S_TX_BCK_DIV_NUM_S);
  
}

//buffer to store two 16 bit words 
uint16_t buff[] = { 0xAAAA , 0x5555 };


void loop() 
{
 	
  //write the buffer (waits until a buffer is ready to be filled, that's timing for free)
  i2s_write_bytes(i2s_num, (char*)buff, sizeof(buff), portMAX_DELAY);

  // dirty toggle the bits for all outputs at max rate
  if (buff[0] == 0xAAAA) {
		buff[0] = 0x5555 ;
		buff[1] = 0xAAAA ;
  }
  else {
	  buff[0] = 0xAAAA;
	  buff[1] = 0x5555;
  }
}
