# Pin Map

Mapping the [Adafruit ESP32 Feather V2](https://learn.adafruit.com/adafruit-esp32-feather-v2) pins to hardware modules

```
                         ┌──────────────┐                     
         SD Card SPI CS ─┤26            │    *Input Only                     
         MAX7219 SPI CS ─┤25            │                     
            Left button ─┤34*           │                     
          Middle button ─┤39*         13├─ LED_BUILTIN, lights
           Right button ─┤36*         12├─ I2S data           
        SSD1309 SPI RST ─┤04          27├─ I2S bck            
                SPI SCK ─┤05          33├─ I2S lrc            
               SPI MOSI ─┤19          15├─                    
               SPI MISO ─┤21          32├─                    
         SSD1309 SPI DC ─┤07          14├─                    
         SSD1309 SPI CS ─┤08          20├─ I2C SCL            
                        ─┤37*         22├─ I2C SDA            
                         └──────────────┘                     
```

![Adafruit ESP32 Feather V2](https://cdn-learn.adafruit.com/assets/assets/000/123/406/original/adafruit_products_Adafruit_ESP32_Feather_V2_Pinout.png)