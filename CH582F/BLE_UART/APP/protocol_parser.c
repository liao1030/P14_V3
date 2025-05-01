/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_parser.c
 * Description        : 多功能生化儀藍牙通訊協議解析器實現
 *******************************************************************************/

#include "protocol_parser.h"
#include "CH58x_common.h"
#include <string.h>

// 解析器狀態機狀態
enum {
    STATE_WAIT_START,    // 等待起始標記
    STATE_WAIT_CMD,      // 等待指令ID
    STATE_WAIT_LENGTH,   // 等待長度
    STATE_WAIT_DATA,     // 等待資料
    STATE_WAIT_CHECKSUM, // 等待校驗和
    STATE_WAIT_END       // 等待結束標記
};

/**
 * @brief 初始化協議解析器
 * 
 * @param parser 協議解析器結構指針
 */
void protocol_parser_init(ProtocolParser_t *parser)
{
    memset(parser, 0, sizeof(ProtocolParser_t));
    parser->rxState = STATE_WAIT_START;
}

/**
 * @brief 重置協議解析器狀態
 * 
 * @param parser 協議解析器結構指針
 */
void protocol_parser_reset(ProtocolParser_t *parser)
{
    parser->rxIndex = 0;
    parser->isPacketComplete = 0;
    parser->rxState = STATE_WAIT_START;
    parser->calculatedChecksum = 0;
}

/**
 * @brief 計算校驗和
 * 
 * @param data 資料指針
 * @param length 資料長度
 * @return uint8_t 校驗和結果
 */
uint8_t calculate_checksum(uint8_t *data, uint16_t length)
{
    uint8_t sum = 0;
    for (uint16_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum;
}

/**
 * @brief 解析接收到的字節
 * 
 * @param parser 協議解析器結構指針
 * @param byte 接收到的字節
 * @return int 1: 封包完成, 0: 繼續解析, -1: 解析錯誤
 */
int protocol_parser_parse(ProtocolParser_t *parser, uint8_t byte)
{
    switch (parser->rxState) {
        case STATE_WAIT_START:
            if (byte == PACKET_START_MARKER) {
                parser->rxBuffer[0] = byte;
                parser->rxIndex = 1;
                parser->rxState = STATE_WAIT_CMD;
                parser->calculatedChecksum = byte; // 開始計算校驗和
            }
            break;
            
        case STATE_WAIT_CMD:
            parser->rxBuffer[parser->rxIndex++] = byte;
            parser->cmdId = byte;
            parser->calculatedChecksum += byte;
            parser->rxState = STATE_WAIT_LENGTH;
            break;
            
        case STATE_WAIT_LENGTH:
            parser->rxBuffer[parser->rxIndex++] = byte;
            parser->dataLength = byte;
            parser->calculatedChecksum += byte;
            
            if (parser->dataLength == 0) {
                // 如果資料長度為0，直接進入校驗和階段
                parser->rxState = STATE_WAIT_CHECKSUM;
            } else {
                parser->rxState = STATE_WAIT_DATA;
                parser->expectedLength = parser->dataLength;
            }
            break;
            
        case STATE_WAIT_DATA:
            parser->rxBuffer[parser->rxIndex++] = byte;
            parser->calculatedChecksum += byte;
            parser->expectedLength--;
            
            if (parser->expectedLength == 0) {
                parser->rxState = STATE_WAIT_CHECKSUM;
            }
            break;
            
        case STATE_WAIT_CHECKSUM:
            parser->rxBuffer[parser->rxIndex++] = byte;
            parser->checksum = byte;
            parser->rxState = STATE_WAIT_END;
            break;
            
        case STATE_WAIT_END:
            parser->rxBuffer[parser->rxIndex++] = byte;
            
            if (byte == PACKET_END_MARKER) {
                if (parser->checksum == parser->calculatedChecksum) {
                    parser->isPacketComplete = 1;
                    return 1; // 封包完成且校驗正確
                } else {
                    // 校驗錯誤
                    protocol_parser_reset(parser);
                    return -1;
                }
            } else {
                // 結束標記錯誤
                protocol_parser_reset(parser);
                return -1;
            }
            break;
            
        default:
            protocol_parser_reset(parser);
            return -1;
    }
    
    return 0; // 繼續解析
}

/**
 * @brief 構建協議封包
 * 
 * @param cmdId 指令ID
 * @param data 資料指針
 * @param length 資料長度
 * @param buffer 輸出緩衝區
 * @param bufferSize 緩衝區大小
 * @return int 封包總長度, -1: 錯誤
 */
int protocol_build_packet(uint8_t cmdId, uint8_t *data, uint16_t length, uint8_t *buffer, uint16_t bufferSize)
{
    if (bufferSize < (length + 5)) {
        return -1; // 緩衝區太小
    }
    
    // 構建封包頭
    buffer[0] = PACKET_START_MARKER;
    buffer[1] = cmdId;
    buffer[2] = length;
    
    // 計算校驗和
    uint8_t checksum = PACKET_START_MARKER + cmdId + length;
    
    // 複製資料
    if (length > 0 && data != NULL) {
        memcpy(&buffer[3], data, length);
        
        // 計算資料部分的校驗和
        for (uint16_t i = 0; i < length; i++) {
            checksum += data[i];
        }
    }
    
    // 添加校驗和和結束標記
    buffer[3 + length] = checksum;
    buffer[4 + length] = PACKET_END_MARKER;
    
    return length + 5; // 返回封包總長度
}

/**
 * @brief 構建錯誤回應封包
 * 
 * @param originalCmd 原始指令ID
 * @param errorCode 錯誤代碼
 * @param buffer 輸出緩衝區
 * @param bufferSize 緩衝區大小
 * @return int 封包總長度, -1: 錯誤
 */
int build_error_response(uint8_t originalCmd, uint8_t errorCode, uint8_t *buffer, uint16_t bufferSize)
{
    ErrorResponseData_t errorData;
    errorData.originalCmd = originalCmd;
    errorData.errorCode = errorCode;
    
    return protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errorData, sizeof(ErrorResponseData_t), buffer, bufferSize);
} 