# CH32V203G8R6和CH582F之間的UART通訊

CH32V203G8R6和CH582F之間的通訊是透過以下UART介面進行的：
* CH32V203G8R6晶片端使用UART2進行通訊
* CH582F晶片端使用UART1進行通訊

## 硬體連接

具體的硬體連接如下：

1. CH32V203G8R6(MCU)的腳位連接：
   * PA2/ADC2/OP2O0 (Pin 11) 作為MCU_TX，連接到UART 2 Tx
   * PA3/ADC3/OP1O0 (Pin 12) 作為MCU_RX，連接到UART 2 Rx

2. CH582F(BLE)的腳位連接：
   * PA8/RXD1/AIN12 (Pin 6) 作為BLE_RX，連接到UART 1 Rx
   * PA9/TMR0/TXD1/AIN13 (Pin 7) 作為BLE_TX，連接到UART 1 Tx

所以CH32V203G8R6的UART2與CH582F的UART1進行跨晶片通訊，使它們能夠在多功能生化測試儀中相互傳遞資料，實現從生化測量到藍牙傳輸的完整功能。