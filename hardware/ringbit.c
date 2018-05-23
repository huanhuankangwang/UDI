
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
	�������ܣ���bit��������������(9λ)
	��������
		ringbit:	�������Ķ���
		data:		��������(8λ)
		bit:		�������ݷ���:1λ
				0:��ʾ���1��ʾ����
	ע�����bit���г���Ϊ32bit
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
	�������ܣ�����һ������ʵ����
	��������
		ringbit:	bit����
		
	����ֵ��0 Ϊ�գ����� ��Ϊ��
	ע�����
		����ʱÿ�ζ���Ҫ�����������ȫ������ȥ��
		����bit���г���ֻ��32λ
*/
void sendChar(char data,int type)
{
	int bit = 0;
//	int  len;
	PT_RingBit ringbit = &s_ringbit;

	//append ����
	appendChar( ringbit, data, bit);
	
	//try send �������
	while(getLen(ringbit) >= 8)
	{
		//senddata
		getChar(ringbit,&data);
		
		//����ʵ�ʷ��͵�ʵ����������spi����Ϊ���ӡ�
		//spi_write(spi,&data,1);
	}
}

/*
	�������ܣ��ж�bit�����Ƿ�Ϊ��
	��������
		ringbit:	bit����
	����ֵ��0 Ϊ�գ����� ��Ϊ��
*/

int isEmpty(PT_RingBit ringbit)
{
	if(ringbit->mLen == 0)
		return 0;
	else
		return 1;
}

/*
	�������ܣ���ȡbit���еĳ���
	��������
		ringbit:	bit����
	����ֵ��bit���еĳ���
	ע�����
		bit���г���Ϊ32bit
*/

int getLen(PT_RingBit ringbit)
{
	return ((ringbit->mLen ));
}