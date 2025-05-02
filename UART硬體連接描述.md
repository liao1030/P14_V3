# P14多功能生化儀藍牙與UART硬體連接描述

## 硬體組成
P14多功能生化儀由以下兩個主要控制芯片組成：
1. **CH32V203G8R6** - 主控MCU，負責測試儀主要功能、數據處理和存儲
2. **CH582F** - 藍牙模組，負責與手機APP的藍牙通訊

## 接口連接說明

### CH32V203G8R6 (主控MCU)
- **UART2** 用於與藍牙模組通訊：
  - **PA2 (Pin 11)** - UART2_TX，連接至CH582F的UART3_RX
  - **PA3 (Pin 12)** - UART2_RX，連接至CH582F的UART3_TX
  - 通訊參數：115200 bps，8數據位，1停止位，無校驗位，無流控制

### CH582F (藍牙模組)
- **UART3** 用於與主控MCU通訊：
  - **PA14 (Pin 31)** - UART3_TX，連接至CH32V203G8R6的UART2_RX
  - **PA15 (Pin 32)** - UART3_RX，連接至CH32V203G8R6的UART2_TX
  - 通訊參數：115200 bps，8數據位，1停止位，無校驗位，無流控制

## 電氣特性
- 工作電壓：3.3V
- 最大通訊速率：115200 bps
- 信號電平：3.3V CMOS/TTL 電平
- 信號線上建議加入適當的上拉電阻和去耦電容，以確保信號質量

## 信號保護
為提高系統穩定性和抗干擾能力，在串口信號線路上添加以下保護措施：
1. 在每條信號線上串聯100Ω電阻，限制短路電流
2. 在每條信號線上並聯TVS二極管，防止靜電和過壓
3. 信號地線間共用屏蔽層，減少電磁干擾

## 板級布局建議
1. 保持通訊線路短而直，避免形成環路
2. 信號線遠離高頻干擾源，如開關電源、DC-DC轉換器等
3. UART信號線與其他信號線之間保持足夠的間距
4. 採用差分走線或加屏蔽的走線方式，提高抗干擾能力

## 調試接口
為方便調試，板子上預留以下測試點：
1. CH32V203G8R6 UART2_TX/RX 測試點
2. CH582F UART3_TX/RX 測試點
3. 電源和地測試點

## 注意事項
1. CH32V203G8R6和CH582F的供電要保持穩定，避免電壓波動
2. 在系統初始化時，確保兩個芯片的上電順序：先上電CH32V203G8R6，後上電CH582F
3. 在進行固件更新時，兩個芯片需要單獨進行更新操作
4. 板級測試時需要確認UART通訊功能正常，包括通訊協議的數據包完整性 