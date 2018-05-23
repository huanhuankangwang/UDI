
//#include <linux/spi/spi.h>
typedef unsigned short   u16;

typedef struct _RingBit{
	int  mBuffer;
	int  mWrite;
	int  mLen;
}T_RingBit,*PT_RingBit;

int getLen(PT_RingBit ringbit);
void appendChar(PT_RingBit ringbit,char data,int bit);
int getChar(PT_RingBit ringbit ,unsigned char *ch);

#define   RINGBIT_SIZE    		(sizeof(int) *8)

#define  MOVE_BIT_OFFSET(pos)    ( RINGBIT_SIZE -1 - pos)
#define  SET_BIT(value,pos)		value |= ( 1 << MOVE_BIT_OFFSET(pos))
#define  CLEAR_BIT(value,pos)		value &= (~( 1 << MOVE_BIT_OFFSET(pos)))

static T_RingBit s_ringbit = {
	.mBuffer = 0,
	.mWrite = 0,
	.mLen = 0,
}; 

/*
	函数功能：向bit队列中新增数据(9位)
	参数解释
		ringbit:	待新增的队列
		data:		新添数据(8位)
		bit:		新添数据方向:1位
				0:表示命令，1表示数据
	注意事项：bit队列长度为32bit
*/

void appendChar(PT_RingBit ringbit,char data,int bit)
{
	int c_data;

	//append first bit
	if(bit)
		SET_BIT(ringbit->mBuffer,ringbit->mWrite);
	else
		CLEAR_BIT(ringbit->mBuffer,ringbit->mWrite);

	ringbit->mWrite += 1;
	//fill 8 data because buffer is enough
	c_data = data << (MOVE_BIT_OFFSET(ringbit->mWrite) + 8);

	ringbit->mBuffer = ringbit->mBuffer | (c_data);
	ringbit->mWrite += 8;
	ringbit->mLen += 9;
}

int getChar(PT_RingBit ringbit ,unsigned char *ch)
{
	int data;

	data = ringbit->mBuffer;
	*ch = (unsigned char )( 0xff &(data >> 24));

	ringbit->mWrite -= 8;
	ringbit->mLen -= 8;
	
	return 0;
}

/*
	函数功能：这是一个发送实例，
	参数解释
		ringbit:	bit队列
		
	返回值：0 为空，非零 不为空
	注意事项：
		发送时每次都需要把里面的数据全都发出去。
		由于bit队列长度只有32位
*/
void sendChar(char data,int type)
{
	int bit = 0;
//	int  len;
	PT_RingBit ringbit = &s_ringbit;

	//append 数据
	appendChar( ringbit, data, bit);
	
	//try send 发送完成
	while(getLen(ringbit) >= 8)
	{
		//senddata
		getChar(ringbit,&data);
		
		//调用实际发送的实例，这里以spi发送为例子。
		//spi_write(spi,&data,1);
	}
}

/*
	函数功能：判断bit队列是否为空
	参数解释
		ringbit:	bit队列
	返回值：0 为空，非零 不为空
*/

int isEmpty(PT_RingBit ringbit)
{
	if(ringbit->mLen == 0)
		return 0;
	else
		return 1;
}

/*
	函数功能：获取bit队列的长度
	参数解释
		ringbit:	bit队列
	返回值：bit队列的长度
	注意事项：
		bit队列长度为32bit
*/

int getLen(PT_RingBit ringbit)
{
	return ((ringbit->mLen ));
}