/********************************** (C) COPYRIGHT *******************************
 * File Name          : protocol_parser.c
 * Description        : �๦�������x�{��ͨӍ�f�h���������F
 *******************************************************************************/

#include "protocol_parser.h"
#include "CH58x_common.h"
#include <string.h>

// ��������B�C��B
enum {
    STATE_WAIT_START,    // �ȴ���ʼ��ӛ
    STATE_WAIT_CMD,      // �ȴ�ָ��ID
    STATE_WAIT_LENGTH,   // �ȴ��L��
    STATE_WAIT_DATA,     // �ȴ��Y��
    STATE_WAIT_CHECKSUM, // �ȴ�У��
    STATE_WAIT_END       // �ȴ��Y����ӛ
};

/**
 * @brief ��ʼ���f�h������
 * 
 * @param parser �f�h�������Y��ָ�
 */
void protocol_parser_init(ProtocolParser_t *parser)
{
    memset(parser, 0, sizeof(ProtocolParser_t));
    parser->rxState = STATE_WAIT_START;
}

/**
 * @brief ���Åf�h��������B
 * 
 * @param parser �f�h�������Y��ָ�
 */
void protocol_parser_reset(ProtocolParser_t *parser)
{
    parser->rxIndex = 0;
    parser->isPacketComplete = 0;
    parser->rxState = STATE_WAIT_START;
    parser->calculatedChecksum = 0;
}

/**
 * @brief Ӌ��У��
 * 
 * @param data �Y��ָ�
 * @param length �Y���L��
 * @return uint8_t У�ͽY��
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
 * @brief �������յ����ֹ�
 * 
 * @param parser �f�h�������Y��ָ�
 * @param byte ���յ����ֹ�
 * @return int 1: ������, 0: �^�m����, -1: �����e�`
 */
int protocol_parser_parse(ProtocolParser_t *parser, uint8_t byte)
{
    switch (parser->rxState) {
        case STATE_WAIT_START:
            if (byte == PACKET_START_MARKER) {
                parser->rxBuffer[0] = byte;
                parser->rxIndex = 1;
                parser->rxState = STATE_WAIT_CMD;
                parser->calculatedChecksum = byte; // �_ʼӋ��У��
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
                // ����Y���L�Ȟ�0��ֱ���M��У���A��
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
                    return 1; // ��������У����_
                } else {
                    // У��e�`
                    protocol_parser_reset(parser);
                    return -1;
                }
            } else {
                // �Y����ӛ�e�`
                protocol_parser_reset(parser);
                return -1;
            }
            break;
            
        default:
            protocol_parser_reset(parser);
            return -1;
    }
    
    return 0; // �^�m����
}

/**
 * @brief �����f�h���
 * 
 * @param cmdId ָ��ID
 * @param data �Y��ָ�
 * @param length �Y���L��
 * @param buffer ݔ�����n�^
 * @param bufferSize ���n�^��С
 * @return int ������L��, -1: �e�`
 */
int protocol_build_packet(uint8_t cmdId, uint8_t *data, uint16_t length, uint8_t *buffer, uint16_t bufferSize)
{
    if (bufferSize < (length + 5)) {
        return -1; // ���n�^̫С
    }
    
    // ��������^
    buffer[0] = PACKET_START_MARKER;
    buffer[1] = cmdId;
    buffer[2] = length;
    
    // Ӌ��У��
    uint8_t checksum = PACKET_START_MARKER + cmdId + length;
    
    // �}�u�Y��
    if (length > 0 && data != NULL) {
        memcpy(&buffer[3], data, length);
        
        // Ӌ���Y�ϲ��ֵ�У��
        for (uint16_t i = 0; i < length; i++) {
            checksum += data[i];
        }
    }
    
    // ���У�ͺͽY����ӛ
    buffer[3 + length] = checksum;
    buffer[4 + length] = PACKET_END_MARKER;
    
    return length + 5; // ���ط�����L��
}

/**
 * @brief �����e�`�ؑ����
 * 
 * @param originalCmd ԭʼָ��ID
 * @param errorCode �e�`���a
 * @param buffer ݔ�����n�^
 * @param bufferSize ���n�^��С
 * @return int ������L��, -1: �e�`
 */
int build_error_response(uint8_t originalCmd, uint8_t errorCode, uint8_t *buffer, uint16_t bufferSize)
{
    ErrorResponseData_t errorData;
    errorData.originalCmd = originalCmd;
    errorData.errorCode = errorCode;
    
    return protocol_build_packet(CMD_ERROR_RESPONSE, (uint8_t *)&errorData, sizeof(ErrorResponseData_t), buffer, bufferSize);
} 