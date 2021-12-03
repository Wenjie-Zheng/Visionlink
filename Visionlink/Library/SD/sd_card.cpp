#include "sd_card.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
	uint8_t CRC7(const uint8_t* data, uint32_t length);
	uint16_t CRC16(const uint8_t* data, uint32_t length);
}
#endif // __cplusplus


typedef enum
{
	sdStep_idle =0,
	sdStep_ifcond,
	sdStep_type1,
	sdStep_type2,
	sdStep_suced,
	sdStep_fail
}sd_init_step_t;

uint8_t SD_CARD::begin(void)
{
	uint8_t curr_staus = sdStep_idle;
	bool flag_onoff = true;
	uint16_t retry = 0;
	uint8_t token;
	uint32_t resp;
	
	this->init();
	setSpeedLow();
	for (uint8_t count = 0; count < 20; count++){
		readWriteByte(0xff);
	}
	
	while (flag_onoff)
	{
		switch (curr_staus)  
		{
		case sdStep_idle:	
			retry = 0x14;
			do
			{
				token = sdTransaction(GO_IDLE_STATE, 0, NULL);
			} while (token != 0x01 && retry--);
			
			if (token != 0x01){
				curr_staus = sdStep_fail;
				break;
			}
			
//			token = sdTransaction(CRC_ON_OFF, 1, NULL);
//			if (token == 0x5) {
//				//old card maybe
//				supports_crc = false;
//			}
//			else if (token != 1) {
//				//log_w("CRC_ON_OFF failed: %u", token);
//				curr_staus = sdStep_fail;
//				break;
//			}
			curr_staus = sdStep_ifcond;
			break;
			
			
		case sdStep_ifcond:
			if (sdTransaction(SEND_IF_COND, 0x1AA, &resp) == 1) {
				curr_staus = sdStep_type1;
			}
			else {
				curr_staus = sdStep_type2;
			}
			break;
			
		case sdStep_type1:
			//resp = readWriteWord(0xffffffff);
			if ((resp & 0xFFF) != 0x1AA){
				//log_w("SEND_IF_COND failed: %03X", resp & 0xFFF);
				curr_staus = sdStep_fail;
				break;
			}
			
			if (sdTransaction(READ_OCR, 0, &resp) != 1 || !(resp & (1 << 20)) ) {
				curr_staus = sdStep_fail;
				break;
			}
			
			retry = 0xFFFE;
			do
			{
				sdTransaction(APP_CMD, 0, NULL);
				token = sdTransaction(APP_OP_COND, 0x40000000, NULL);	//收到回复0x00
			} while (token && retry--);
			
			if (token){
				curr_staus = sdStep_fail;
				break;
			}
//			if (token && sdTransaction(READ_OCR, 0, NULL) != 0) {
//				curr_staus = sdStep_fail;
//				break;
//			}
			
//			resp = readWriteWord(0xffffffff);
//			if (resp & (0x01 << 30)){
//				sdType = CARD_SDV2HC;
//			}else{
//				sdType = CARD_SDV2;
//			}
			
			if ( !sdTransaction(READ_OCR, 0, &resp) ) {
				if (resp & (1 << 30)) 
					sdType = CARD_SDV2HC;
				else	
					sdType = CARD_SDV2;
			}
			else{
				curr_staus = sdStep_fail;
				break;
			}
				
			curr_staus = sdStep_suced;
			break;
			
		case sdStep_type2:
			sdTransaction(APP_CMD, 0, NULL);
			token = sdTransaction(APP_OP_COND, 0, NULL);
			
			retry = 0x800;
			if (token <= 1){
				sdType = CARD_SDV1;
				do //等待退出IDLE模式
				{
					sdTransaction(APP_CMD, 0, NULL);
					token = sdTransaction(APP_OP_COND, 0, NULL);
				}while (token && retry--);
			}
			else{
				sdType = CARD_MMC; //MMC V3
				do //等待退出IDLE模式
				{	//MMC卡不支持CMD55+CMD41识别
					token = sdTransaction(SEND_OP_COND, 0, NULL);
				}while (token && retry--);
			}
			
			if(!retry || sdTransaction(SET_BLOCKLEN, 512, NULL) != 0){
				sdType = CARD_NONE;
				curr_staus = sdStep_fail;
				break;
			}
			
			curr_staus = sdStep_suced;
			break;
			
		case sdStep_suced:
//			if (sdType != CARD_MMC)
//			{
//				if (sdTransaction(APP_CLR_CARD_DETECT, 0, NULL)){
//					//log_w("APP_CLR_CARD_DETECT failed");
//					curr_staus = sdStep_fail;
//					break;
//				}
//			}
			
			sdDeselectCard();
			sdSectors = sdGetSectorsCount();	//获取扇区数
			setSpeedHigh();
			flag_onoff = false;
			break;
			
		case sdStep_fail:
			sdType = CARD_NONE;
			flag_onoff = false;
			break;
			
		default:
			curr_staus = sdStep_fail; 
			break;
		}
	}
	
	if(sdType) {
		sdStatus &= ~STA_NOINIT;
	}
	else
		sdStatus |= STA_NOINIT;
	
	
	return sdStatus;
}





uint8_t SD_CARD::sdTransaction(uint8_t cmd, uint32_t arg, uint32_t* resp)
{
	sdDeselectCard();
	if (!sdSelectCard()){
		return 0xff;
	}
	
	uint8_t token = sdCommand(cmd, arg);
	if (resp != NULL)
	{
		*resp = 0;
		
		if (cmd == SEND_STATUS) {
			*resp = readWriteByte(0xff);
		}
		else if (cmd == SEND_IF_COND || cmd == READ_OCR) {
			*resp = readWriteWord(0xffffffff);
		}
	}
	//sdDeselectCard();
	return token;
}


bool SD_CARD::sdSelectCard(uint8_t pdrv /* = 0 */)
{
	pinSet(CS.GPIO, CS.Pin, Bit_RESET);
	if (!sdWait(500)) {
		//LV_LOG_ERROR("Select Failed");
		sdDeselectCard();
		return false;
	}
	return true;
}


void SD_CARD::sdDeselectCard(uint8_t pdrv /* = 0 */)
{
	pinSet(CS.GPIO, CS.Pin, Bit_SET);
	//readWriteByte(0xff);
}


bool SD_CARD::sdWait(uint16_t timeOut, uint8_t pdrv /* = 0 */)
{
	uint8_t resp;
	
	do
	{
		resp = readWriteByte(0xff);
	} while (resp != 0xff && timeOut--);
	
	return (resp == 0xff);
}


uint8_t SD_CARD::sdCommand(uint8_t cmd, uint32_t arg)
{
	uint16_t retry = 0;
	uint8_t cmdPacket[7];
	uint8_t token;
	
	cmdPacket[0] = cmd | 0x40;
	cmdPacket[1] = (uint8_t)(arg >> 24);
	cmdPacket[2] = (uint8_t)(arg >> 16);
	cmdPacket[3] = (uint8_t)(arg >> 8);
	cmdPacket[4] = (uint8_t)(arg);
	
	if (supports_crc || cmd == GO_IDLE_STATE || cmd == SEND_IF_COND){
		cmdPacket[5] = (CRC7(cmdPacket, 5) << 1) | 0x01;
	}
	else{
		cmdPacket[5] = 0x01;
	}
	
	cmdPacket[6] = 0xFF;
	
	writeData(cmdPacket, (cmd == STOP_TRANSMISSION) ? 7 : 6);
	
	retry = 0XFF;
	do
	{
		token = readWriteByte(0xFF);
	} while ((token & 0X80) && retry--);	
	
		
//	if (token == 0xFF){
//		//log_w("no token received");
//		sdDeselectCard();
//		delay_ms(100);
//		sdSelectCard();
//		continue;
//	}else if (token & 0x08){
//		//log_w("crc error");
//		sdDeselectCard();
//		delay_ms(100);
//		sdSelectCard();
//		continue;
//	}else if (token > 1){
//		//log_w("token error [%u] 0x%x", cmd, token);
//		break;
//	}

	return token;
}


uint8_t SD_CARD::sdSetSectorsSize(uint16_t size)
{
	return sdTransaction(SET_BLOCKLEN, size, NULL);
}

//获取SD卡的总扇区数（扇区数）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(扇区数/512字节)
//每扇区的字节数必为512，因为如果不是512，则初始化不能通过.
uint32_t SD_CARD::sdGetSectorsCount(void)
{
	uint8_t csd[16];	
	uint32_t Capacity;
	uint8_t	n;
	uint16_t csize;
		
	if (!sdGetCSD(csd)) 
		return 0;	//读取失败

	 //如果为SDHC卡，按照下面方式计算
	if ((csd[0] & 0xC0) == 0x40)	 //V2.00的卡
	{	
		csize = csd[9] + ((u16)csd[8] << 8) + 1;
		Capacity = (u32)csize << 10; //得到扇区数	 		   
	}else//V1.XX的卡
	{	
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
		Capacity = (u32)csize << (n - 9); //得到扇区数   
	}
	
	return Capacity;
}
	
	


uint8_t SD_CARD::sdGetCSD(uint8_t *csd)
{
	uint8_t token = 0;	 
	
	if (sdTransaction(SEND_CSD, 0, NULL) == 0)//发CMD9命令，读CSD
	{
		token = sdReadData(csd, 16); //接收16个字节的数据 
	}
	else{
		sdDeselectCard();
		return 0;
	}
	
	sdDeselectCard();
	
	if (token)
		return 1;	
	
	return 0;
}


bool SD_CARD::sdReadData(uint8_t *buffer, uint32_t len)
{
	uint8_t token;
	uint8_t index = 0;
	
	uint16_t retry = 0xffff;
	do
	{
		token = readWriteByte(0xff);
	} while (token != 0xfe && retry--);
	
	if (retry == 0) {
		return 0;	//失败
	}
	
	while (len--)
	{
		*buffer = readWriteByte(0xff);
		buffer++;
	}
	
	uint16_t crc = readWriteHalfWord(0xffff);
	
	if (supports_crc == false || crc == CRC16(buffer, len)){
		return 1;	//读取并校验成功
	}
	
	
	return 0;		
}


DRESULT SD_CARD::sdReadDisk(uint8_t *read_buf, uint32_t sector, uint32_t cout)
{
	DRESULT res = RES_ERROR;
	
//	if (this->status & STA_NOINIT){
//		return RES_NOTRDY;
//	}
	
	if (sdType != CARD_SDV2HC)sector <<= 9; //转换为字节地址
	
	if (cout == 1){
		res = sdReadSector(read_buf, sector) ? RES_OK : RES_ERROR;
	}
	else{
		res = sdReadSectors(read_buf, sector, cout)? RES_OK : RES_ERROR;
	}
	
	return res;
}

bool SD_CARD::sdReadSectors(uint8_t* buffer, uint32_t sector, uint32_t count)
{
	uint8_t resp;
	uint32_t read_len = count;
	
	resp = sdTransaction(READ_BLOCK_MULTIPLE, sector, NULL); //命令发送成功
	do
	{
		resp = sdReadData(buffer, 512); //接收512个字节	 
		buffer += 512;  
	} while ((resp != 0) && --count); //读取成功并且未读取完成.
	sdTransaction(STOP_TRANSMISSION, 0, NULL);
	
	
	if (resp != 0 && count == 0) {
		return true;
	}
	
	return false;
}

bool SD_CARD::sdReadSector(uint8_t* buffer, uint32_t sector)
{
	uint8_t resp;
	
	if(sdTransaction(READ_BLOCK_SINGLE, sector, NULL) == 0)	//命令发送成功
	{
		resp = sdReadData(buffer, 512);
	}
	sdDeselectCard();
	
	if (resp)
		return true;
	else
		return false;
}




DRESULT SD_CARD::sdWriteDisk(uint8_t *buffer, uint32_t sector, uint32_t cout)
{
	DRESULT res = RES_ERROR;
	
//	if (this->status & STA_NOINIT) {
//		return RES_NOTRDY;
//	}
//	if (this->status & STA_PROTECT) {
//		return RES_PARERR;
//	}
	
	if (sdType != CARD_SDV2HC)sector <<= 9; //转换为字节地址
	
	if (cout == 1){
		res = sdWriteSector(buffer, sector) ? RES_OK : RES_ERROR;
	}
	else{
		res = sdWriteSectors(buffer, sector, cout) ? RES_OK : RES_ERROR;
	}
	
	return res;
}

uint8_t SD_CARD::sdWriteBlock(uint8_t* buffer, uint8_t cmd)
{
	uint16_t crc = (supports_crc) ? CRC16(buffer, 512) : 0xFFFF;
	uint8_t resp;
	
	if (!sdWait(500)){
		sdDeselectCard();
		return 0;	//等待准备失效
	}
	readWriteByte(cmd);
	if (cmd != 0xfd)	//不是结束指令
	{
		writeData(buffer, 512);
		readWriteHalfWord(crc);
		resp = readWriteByte(0xff);
		if ((resp & 0x1F) != 0x05) 
			return 0;
	}
	
	return 1;
}

bool SD_CARD::sdWriteSector(uint8_t* buffer, uint32_t sector)
{
	uint32_t resp;
	
	if (sdTransaction(WRITE_BLOCK_SINGLE, sector, NULL) == 0)//指令发送成功
	{
		resp = sdWriteBlock(buffer, 0xfe);//写512个字节	   
	}
	
	sdDeselectCard();
	if (resp) return true;
	
	return false;
}

bool SD_CARD::sdWriteSectors(uint8_t* buffer, uint32_t sector, uint32_t count)
{
	uint8_t resp;

	if (sdType != CARD_MMC)
	{
		sdTransaction(APP_CMD, 0, NULL);
		sdTransaction(SET_WR_BLK_ERASE_COUNT, count, NULL);
	}
		
	if (sdTransaction(WRITE_BLOCK_MULTIPLE, count, NULL) == 0)//连续读命令
	{
		do
		{
			resp = sdWriteBlock(buffer, 0xFC);
			buffer += 512;
		} while (resp != 0 && --count);
		resp = sdWriteBlock(0, 0xFD); //接收512个字节
			
		if (count == 0 && resp) {
			return true;
		}
	}
	
	return false;
}




