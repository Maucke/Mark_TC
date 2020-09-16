#include "pt6311.h"
#include "rgb.h"
sbit VFDPW = P1 ^ 2;
sbit DATA = P1 ^ 3;
sbit CLK = P1 ^ 4;
sbit STB = P1 ^ 5;

/************VFD地址部分***************/
u8 code NumDigi[] = { 15, 21, 22, 18, 19, 16 };
u8 code MiDigi[] = { 12, 9, 6, 3, 0 };
u8 code TitleDigi[] = { 20, 23, 17, 32, 11, 14, 8, 5, 2 };

/************VFD驱动部分***************/
u8 code CYCLETab[] = { 0x00, 0x40, 0xC0, 0xC1, 0xC3, 0xC7, 0xCF, 0xDF, 0xFF };	  //转盘用
u8 code HRDRTab[] = { 0xA0, 0xA1, 0xA3, 0xA7, 0xAE, 0xAC, 0xA8, 0xA0 };	  //HDHR用
u8 code NumTab[] = { 0x77, 0x12, 0x6b, 0x5b, 0x1e, 0x5d, 0x7d, 0x13, 0x7f, 0x5f, 0x88, 0x22, 0x00, 0x2F ,0x6D ,0x1C};  //编码方式：Dp D E C G F B A 

u16 code NetBuf[]={0xFF7F,0xFE7E,0xFC7C,0xF878,0xF070,0xE060,0xC040};
u16 code DoxBuf[]={0x0010,0x0030,0x0070,0x00F0,0x00F8,0x80F8,0x8CF8,0xACF8,0xBCF8,0xBDF8};
u16 code MiTab[] = {
	0x3146, 0x1002, 0xd145, 0xd143, 0xf003, 0xe143, 0xe147, 0x1102,	//0,1,2,3,4,5,6,7
	0xf147, 0xf143, 0x0000, 0xc001, 0x8808, 0xf107, 0xd552, 0x2144,	//8,9, ,-,/,A,B,C
	0x9552, 0xe145, 0xe105, 0x6146, 0xf007, 0x8550, 0x1046, 0xa825,	//D,E,F,G,H,I,J,K
	0x2044, 0xba06, 0xb226, 0x3146, 0xf105, 0x3166, 0xf125, 0xe143,	//L,M,N,O,P,Q,R,S
	0x8510, 0x3046, 0xa80c, 0xb02e, 0x8a28, 0x8a10, 0x8948, 0xc411,	//T,U,V,W,X,Y,Z,+
	0x8000, 0x0040, 0x0220, 0xA101, 0xEA2B, 0x0808, 0xC001					//.,_,\,℃,%,:,
};

u16 MiNEW[6] = { 0, 0, 0, 0, 0 };	 //五个米字缓存
u16 MiRAM[6] = { 0, 0, 0, 0, 0 };	 //五个米字缓存
u16 NumNEW[6] = { 0, 0, 0, 0, 0, 0 };		 //六个数字缓存
u16 NumRAM[6] = { 0, 0, 0, 0, 0, 0 };		 //六个数字缓存
u16 DoxRAM = 0x5A5A ;

//将所有的动作放在一个数组中
u8 Step_Mt[34];

//高8位和低8位互换
u16 High2Low(u16 Input)
{
	return (Input<<8)|(Input>>8);
}

//向PT6311中写入数据
void PT6311_Wrt(u8 dat)
{
	u8 i;
	for (i = 0; i<8; i++)
	{
		if((dat&1) ==1)
			DATA = 1;
		else
			DATA = 0;
		dat = dat >> 1;
		CLK = High;
		CLK = Low;
	}
}

//工作方式
void PT6311_Func(void)
{
	STB = Low;
	STB = High;
	PT6311_Wrt(0x0a);//设置显示方式  12栅18段
}

//模式
void PT6311_Mode(void)
{								//				0  0	  写数据至显示存储
	STB = Low;
	STB = High;
	PT6311_Wrt(0x40);	//数据设定 写显存 自动增地址 正常模式
	//设定数据      0 1 - - b3 b2 b1 b0
	//				0  0	  写数据至显示存储
	//				0  1      写数据至LED端
	//				1  0	  读键数据
	//				1  1	  读SW数据	
	//		    0  0	  正常操作 数据写入后增量地址

}

//开始
void PT6311_Start(void)
{
	STB = Low;
	STB = High;
	PT6311_Wrt(0xc0);//设定地址从00开始
}

//背光调整
void PT6311_Dim(u8 Light) //亮度 136~143	   
{
	STB = Low;
	STB = High;
	if(Light!=0)
		PT6311_Wrt(135 + Light);//138 143
}


//局部刷新
void PT6311_ReFush(u8 Addr, u8 dat)
{
	STB = Low;
	STB = High;
	PT6311_Wrt(0xc0 + Addr);
	PT6311_Wrt(dat);
	STB = Low;
}

//局部刷新一个INT16
void PT6311_DobReFush(u8 Addr, u16 dat)
{
	STB = Low;
	STB = High;
	PT6311_Wrt(0xc0 + Addr);
	PT6311_Wrt(dat >> 8);
	PT6311_Wrt(dat);
	STB = Low;
}

void PT6311_Init(void)
{
	u8 i;
	VFDPW = High;
	PT6311_Func();//设置显示方式 
	PT6311_Mode(); //数据设定 写显存 自动增地址 正常模式
	PT6311_Start();
	for (i = 0; i < 34; i++)
	{
		PT6311_Wrt(0x00);
		Step_Mt[i] = 0;
	}
	
}

void PT6311_State(u8 State)
{
	VFDPW = State & 0x1;
}

void PT6311_NumDis(u8 *Sp)
{
	u8 i;
	for (i = 0; i<6; i++)
		NumNEW[i] = Sp[i];
}

void PT6311_NumRAM(u8 Flash)
{
	u8 i;
	if(Flash)
		for (i = 0; i<6; i++)
			PT6311_ReFush(NumDigi[i], NumRAM[i]|0x80);
	else
		for (i = 0; i<6; i++)
			PT6311_ReFush(NumDigi[i], NumRAM[i]&0x7F);
}

u8 ArrayContrast(u16 *Ay1,u16 *Ay2,u8 Count)
{
	u8 i;
	for(i=0;i<Count;i++)
		if(Ay1[i]!=Ay2[i])
			return False;
	return True;
}

void PT6311_MiDis(u16 *Sp)
{
	u8 i;
	for (i = 0; i<5; i++)
		PT6311_DobReFush(MiDigi[i], Sp[i]);
}

void PT6311_MiRAM(void)
{
	u8 i;
	for (i = 0; i<5; i++)
	{
		PT6311_DobReFush(MiDigi[i], MiRAM[i]);
	}
}

void PT6311_PtTime(u8 Hou, Min, Sec)
{
	if (Hou >> 4)
		NumNEW[0] = NumTab[Hou >> 4];
	else
		NumNEW[0] = 0x80;
	NumNEW[1] = NumTab[Hou & 0xF];
	NumNEW[2] = NumTab[Min >> 4];
	NumNEW[3] = NumTab[Min & 0xF];
	NumNEW[4] = NumTab[Sec >> 4];
	NumNEW[5] = NumTab[Sec & 0xF];
}

void PT6311_Bool(u8 Bl)
{
	u8 i;
	for(i=0;i<6;i++)
		NumNEW[i] = NumTab[0xA];
	if(Bl == 0)
	{
		NumNEW[2] = NumTab[0];
		NumNEW[3] = 0x2D;//F
		NumNEW[4] = 0x2D;//F
	}
	else if(Bl == 1)
	{
		NumNEW[2] = NumTab[0];
		NumNEW[3] = 0x38;//n
	}
	else if(Bl == 2)
	{
		NumNEW[1] = 0x3F;//A
		NumNEW[2] = 0x70;//u
		NumNEW[3] = 0x6C;//t
		NumNEW[4] = 0x78;//o
	}
	else if(Bl == 3)
	{
		NumNEW[1] = 0x65;//C
		NumNEW[2] = 0x78;//o
		NumNEW[3] = NumTab[1];//l
		NumNEW[4] = 0x78;//o
		NumNEW[5] = 0x28;//r
		
	}
		
}

u8 code NumMtBuf[] = {0x01,0x03,0x13,0x53,0x57,0x77,0x7F};

u8 MtTemp[5];
void PT6311_TimeMt(u8 Style,u8 Flash)
{
	u8 i;
	if(!ArrayContrast(NumRAM,NumNEW,6))
	{
		Step_Mt[0]++;
		for(i=0;i<6;i++)
			if(NumRAM[i]!=NumNEW[i])
				switch(Style)
				{
					case 1:NumRAM[i] = (NumRAM[i]<<1)|((NumNEW[i]>>(8-Step_Mt[0]))&1);break;
					case 2:NumRAM[i] = NumNEW[i]&(NumMtBuf[Step_Mt[0]-1]);break;
					case 3:NumRAM[i] = NumNEW[i]&((1<<Step_Mt[0])-1);break;
					case 4:if(Step_Mt[0]==1) NumRAM[i] &= NumNEW[i]; if(Step_Mt[0]==4) NumRAM[i] = NumNEW[i];break;
					case 5:if(Step_Mt[0]==1) {MtTemp[i] = NumRAM[i];NumRAM[i] |= NumNEW[i];} else if(Step_Mt[0]==2) NumRAM[i] = NumNEW[i]&MtTemp[i]; else if(Step_Mt[0]==4) NumRAM[i]=NumNEW[i];break;
				}
		if(Step_Mt[0]==8)
		{
			for(i=0;i<6;i++)
				 NumRAM[i]=NumNEW[i];
			Step_Mt[0]=0;
		}
	}
	else
	{
		Step_Mt[0]=0;
	}
	PT6311_NumRAM(Flash);
}
	

void PT6311_MiStr(u8 *ch)
{
	u8 i = 0;
	for (i = 0; i<5; i++)
	{
		if (ch[i] >= 'A'&&ch[i] <= 'Z')
			MiNEW[i] = MiTab[ch[i] - 52];
		else if (ch[i] >= '0'&&ch[i] <= '9')
			MiNEW[i] = MiTab[ch[i] - 48];
		else if (ch[i] == '_')
			MiNEW[i] = MiTab[41];
		else if (ch[i] == '-')
			MiNEW[i] = MiTab[46];
		else
			MiNEW[i] = 0;
	}
}

u16 code RunnBuf[][5] = 
{0xDD5D,0xD80D,0xC001,0x0000,0x0000,
0x0000,0x0000,0x2222,0x2772,0xBF7E};

u16 Temporary = 0;

void PT6311_MiMt(u8 Style)
{
	u8 i;
	u8 Keye;
	if(Step_Mt[7]>Step_Mt[8])
		Keye = 0x5A;
	else if(Step_Mt[7]<Step_Mt[8])
		Keye = 0xA5;
	else
		Keye = Style;
	if(!ArrayContrast(MiRAM,MiNEW,5))
	{
		Step_Mt[1]++;
		switch(Keye)
		{
			case 0xA5:for(i=0;i<5;i++) MiRAM[5-i] = MiRAM[(6+5-i-1)%6]; MiRAM[0] = MiNEW[6-Step_Mt[1]];break;
			case 0x5A:for(i=0;i<5;i++) MiRAM[i] = MiRAM[(i+1)%6]; MiRAM[5] = MiNEW[Step_Mt[1]-1];break;
			case 1:for(i=0;i<5;i++) MiRAM[i] = MiNEW[i]<<(6-Step_Mt[1]); break;
			case 2:for(i=0;i<5;i++) MiRAM[i] = (MiRAM[i]&RunnBuf[0][Step_Mt[1]-1])|(MiNEW[i]&RunnBuf[1][Step_Mt[1]-1]);break;
		}
		if(Step_Mt[1]==6)
		{
			MiNEW[6] = 0;
			for(i=0;i<6;i++)
				MiRAM[i]=MiNEW[i];
			Step_Mt[1]=0;
			Step_Mt[8]=Step_Mt[7];
			Temporary = 0;
		}
	}
	else
	{
		Step_Mt[1]=0;
	}
	PT6311_MiRAM();
}
	

void PT6311_NETMt(u8 Style)
{
	if(!Step_Mt[10])
		switch(Style)
		{
			case 0:
			{
				PT6311_ReFush(VNETWORK, (0x01 << Step_Mt[2]) | 0x80);
				PT6311_ReFush(VMUSIC, (0x01 << Step_Mt[2]) & 0x7F);
				if (Step_Mt[2]++ >= 6)
					Step_Mt[2] = 0;
			}break;
			case 1:
			{
				if(Step_Mt[2]<7)
				{
					PT6311_ReFush(VNETWORK, (0x01 << Step_Mt[2]) | 0x80);
					PT6311_ReFush(VMUSIC, 0x00);
				}
				else
				{
					PT6311_ReFush(VNETWORK, 0x80);
					PT6311_ReFush(VMUSIC, (0x01 << (Step_Mt[2]-7)) & 0x7F);
				}
				if (Step_Mt[2]++ >= 13)
					Step_Mt[2] = 0;
			}break;
		}
}


u16 RandomData;
#define CycleControl 8
void PT6311_DoEvent(void)
{
	if(!Step_Mt[10])
		switch(Step_Mt[3])
		{
			case 0:PT6311_ReFush(VHRDR, HRDRTab[7]);break;
			case 0+CycleControl:PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)>>12)&0x000F)|((High2Low(DoxRAM)<<4)&0xFFF0)));     break;
			case 1+CycleControl:PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)>>8)&0x00FF)|((High2Low(DoxRAM)<<8)&0xFF00)));     break;
			case 2+CycleControl:PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)>>4)&0x0FFF)|((High2Low(DoxRAM)<<12)&0xF000)));DoxRAM = RandomData;     break;
			case 3+CycleControl:PT6311_DobReFush(VBOXDN,DoxRAM);        break;
			case 4+CycleControl:RandomData = rand();PT6311_DobReFush(VBOXDN,((RandomData<<3)&0x8888)|((DoxRAM>>1)&0x7777));     break;
			case 5+CycleControl:PT6311_DobReFush(VBOXDN,((RandomData<<2)&0xCCCC)|((DoxRAM>>2)&0x3333));PT6311_ReFush(VHRDR, HRDRTab[0]);     break;
			case 6+CycleControl:PT6311_DobReFush(VBOXDN,((RandomData<<1)&0xEEEE)|((DoxRAM>>3)&0x1111));DoxRAM = RandomData;PT6311_ReFush(VHRDR, HRDRTab[1]);     break;
			case 7+CycleControl:PT6311_DobReFush(VBOXDN,DoxRAM);   PT6311_ReFush(VHRDR, HRDRTab[2]);   break;	
			case 8+CycleControl:PT6311_ReFush(VHRDR, HRDRTab[3]);       break;		
			case 9+CycleControl:PT6311_ReFush(VHRDR, HRDRTab[4]);       break;		
			case 10+CycleControl:PT6311_ReFush(VHRDR, HRDRTab[5]);      break;		
			case 11+CycleControl:PT6311_ReFush(VHRDR, HRDRTab[6]);      break;		
			case 12+CycleControl:PT6311_ReFush(VHRDR, HRDRTab[7]);RandomData = rand();            break;		
		}
		Step_Mt[3]++;
		if(Step_Mt[3]>(12+CycleControl))
		{
			Step_Mt[4]++;
			if(Step_Mt[4]>8)
			{
				Step_Mt[4]=0;
				Step_Mt[5]=1;
			}
			Step_Mt[3]=0;
		}
}

void PT6311_DoSet(void)
{
	if(!Step_Mt[10])
		switch(Step_Mt[3])
		{
			case 0:PT6311_ReFush(VHRDR, 0x51);PT6311_ReFush(VROUND, 0x33);PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)<<12)&0xF000)|((High2Low(DoxRAM)>>4)&0x0FFF)));     break;
			case 1:PT6311_ReFush(VHRDR, 0x53);PT6311_ReFush(VROUND, 0x66);PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)<<8)&0xFF00)|((High2Low(DoxRAM)>>8)&0x00FF)));     break;
			case 2:PT6311_ReFush(VHRDR, 0x57);PT6311_ReFush(VROUND, 0xCC);PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)<<4)&0xFFF0)|((High2Low(DoxRAM)>>12)&0x000F)));DoxRAM = RandomData;     break;
			case 3:PT6311_ReFush(VHRDR, 0x5F);PT6311_ReFush(VROUND, 0x99);PT6311_DobReFush(VBOXDN,DoxRAM);RandomData = rand();        break;
			case 4:PT6311_ReFush(VHRDR, 0x57);PT6311_ReFush(VROUND, 0x33);PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)<<12)&0xF000)|((High2Low(DoxRAM)>>4)&0x0FFF)));     break;
			case 5:PT6311_ReFush(VHRDR, 0x53);PT6311_ReFush(VROUND, 0x66);PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)<<8)&0xFF00)|((High2Low(DoxRAM)>>8)&0x00FF)));     break;
			case 6:PT6311_ReFush(VHRDR, 0x51);PT6311_ReFush(VROUND, 0xCC);PT6311_DobReFush(VBOXDN,High2Low(((High2Low(RandomData)<<4)&0xFFF0)|((High2Low(DoxRAM)>>12)&0x000F)));DoxRAM = RandomData;     break;
			case 7:PT6311_ReFush(VHRDR, 0x51);PT6311_ReFush(VROUND, 0x99);PT6311_DobReFush(VBOXDN,DoxRAM);RandomData = rand();        break;
		}
		Step_Mt[3]++;
		if(Step_Mt[3]>7)
		{
//			Step_Mt[4]++;
//			if(Step_Mt[4]>8)
//			{
//				Step_Mt[4]=0;
//				Step_Mt[5]=1;
//			}
			Step_Mt[3]=0;
		}
		
}

void PT6311_CYCLEMt(void)
{
	if(Step_Mt[4]==Step_Mt[9]&&!Step_Mt[5])
		PT6311_ReFush(VROUND, CYCLETab[Step_Mt[9]]);
	else if(Step_Mt[4]>Step_Mt[9])
	{
		Step_Mt[9]++;
		PT6311_ReFush(VROUND, CYCLETab[Step_Mt[9]]);
	}
	else if(Step_Mt[4]<Step_Mt[9])
	{
		Step_Mt[9]--;
		PT6311_ReFush(VROUND, CYCLETab[Step_Mt[9]]);
	}
	else if(Step_Mt[5])
	{
		Step_Mt[5]=0;
		Step_Mt[9]=0;		//防止执行下面代码导致转一圈
		Step_Mt[7]++;   //进入下一个信息
		if(Step_Mt[7]>4)
			Step_Mt[7]=0;
	}
}


void PT6311_InitEvent(void)
{
	if(Step_Mt[3]<10)
	{
		PT6311_DobReFush(VBOXDN,DoxBuf[Step_Mt[3]]);
		Step_Mt[3]++;
	}
	else
	{
		DoxRAM = DoxBuf[9];
	}
}


void PT6311_InitNet(void)
{
	if(Step_Mt[2]<7)
	{
		PT6311_DobReFush(VNETWORK,NetBuf[Step_Mt[2]]);Step_Mt[2]++;
	}
	else
	{
		Step_Mt[10] = False;
	}
}

void PT6311_Tilte(u8 Type, u8 Error)
{
	u8 i;
	if (Error)
		for (i = 0; i<8; i++)
			PT6311_ReFush(TitleDigi[i], 0xFF);
	else if (Type<8)
		for (i = 0; i<8; i++)
			if (i == Type)
				PT6311_ReFush(TitleDigi[i], 0xFF);
			else
				PT6311_ReFush(TitleDigi[i], 0x00);
	else
		for (i = 0; i<8; i++)
			PT6311_ReFush(TitleDigi[i], 0x00);
}


void PT6311_PutYear(u8 Year)
{
	MiNEW[0] = MiTab[2];
	MiNEW[1] = MiTab[0];
	MiNEW[2] = MiTab[Year >> 4];
	MiNEW[3] = MiTab[Year & 0xF];
	MiNEW[4] = MiTab[10];
}

void PT6311_PutMonDay(u8 Month, u8 Day)
{
	if(Day)
	{
		switch (Month)
		{
		case 0x1:PT6311_MiStr("JAN  "); break;
		case 0x2:PT6311_MiStr("FEB  "); break;
		case 0x3:PT6311_MiStr("MAR  "); break;
		case 0x4:PT6311_MiStr("APR  "); break;
		case 0x5:PT6311_MiStr("MAY  "); break;
		case 0x6:PT6311_MiStr("JUN  "); break;
		case 0x7:PT6311_MiStr("JUL  "); break;
		case 0x8:PT6311_MiStr("AUG  "); break;
		case 0x9:PT6311_MiStr("SET  "); break;
		case 0x10:PT6311_MiStr("OCT  "); break;
		case 0x11:PT6311_MiStr("NOV  "); break;
		case 0x12:PT6311_MiStr("DEC  "); break;
		default:PT6311_MiStr("ERR  "); break;
		}
		MiNEW[3] = MiTab[Day >> 4];
		MiNEW[4] = MiTab[Day & 0xF];
	}
	else PT6311_MiStr("ERROR");
		
}

void PT6311_PutWeek(u8 Week)
{
	switch (Week)
	{
	case 1:PT6311_MiStr("MON  "); break;
	case 2:PT6311_MiStr("TUES "); break;
	case 3:PT6311_MiStr("WED  "); break;
	case 4:PT6311_MiStr("THUR "); break;
	case 5:PT6311_MiStr("FRI  "); break;
	case 6:PT6311_MiStr("SAT  "); break;
	case 7:PT6311_MiStr("SUN  "); break;
	default:PT6311_MiStr("ERROR"); break;
	}
}

void PT6311_PutTemp(u8 Temp)
{
	if(Temp<99)
	{
		MiNEW[0] = MiTab[32];
		MiNEW[1] = MiTab[42];
		MiNEW[2] = MiTab[Temp / 10];
		MiNEW[3] = MiTab[Temp % 10];
		MiNEW[4] = MiTab[43];
	}
	else
		PT6311_MiStr("ERROR");
}

void PT6311_PutHumi(u8 Humi)
{
	if(Humi<99)
	{
		MiNEW[0] = MiTab[20];
		MiNEW[1] = MiTab[42];
		MiNEW[2] = MiTab[Humi / 10];
		MiNEW[3] = MiTab[Humi % 10];
		MiNEW[4] = MiTab[44];
	}
	else
		PT6311_MiStr("ERROR");
}

void PT6311_PutDim(u8 Dim)
{
	PT6311_MiStr("DIM A");
	MiNEW[3] = MiTab[42];
	if(Dim<9)
		MiNEW[4] = MiTab[Dim];
}

void PT6311_PutMiMt(u8 MiMt)
{
	PT6311_MiStr("MHME ");
	MiNEW[4] = MiTab[MiMt];
}

void PT6311_PutTmMt(u8 TmMt)
{
	PT6311_MiStr("THME ");
	MiNEW[4] = MiTab[TmMt];
}

void PT6311_Check(u16 Speed)
{
	u8 i,j;
	for(i=0;i<34;i++)
	{
		STB = Low;
		STB = High;
		PT6311_Wrt(0xc0);
		for(j=0;j<34;j++)
			PT6311_Wrt(rand());
		STB = Low;
		delay_ms(Speed);
	}
	for(i=0;i<34;i++)
	{
		STB = Low;
		STB = High;
		PT6311_Wrt(0xc0);
		for(j=i+1;j<34;j++)
			PT6311_Wrt(rand());
		for(j=0;j<i+1;j++)
			PT6311_Wrt(0xFF);
		STB = Low;
		delay_ms(Speed);
	}
	delay_ms(Speed*4);
	PT6311_ReFush(VUSB, 0x00);
}

void PT6311_Blink(u8 Val)
{
	if(Val)
		PT6311_MiStr("BK_ON");
	else
		PT6311_MiStr("BK_OF");
}

