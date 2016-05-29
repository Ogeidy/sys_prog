﻿#define DL_ASSTEXT 17
#define DL_OBJTEXT 50                             /*длина об'ектн. текста   */
#define NSYM 10                                   /*размер табл.символов    */
#define NPOP 6                                    /*размер табл.псевдоопер. */
#define NOP  10                                    /*размер табл.операций    */
#include <string.h>                               /*вкл.строковые подпрогр. */
#include <stdlib.h>                               /*вкл.подпрогр.преобр.данн*/
#include <stdio.h>                                /*вкл.подпр.станд.вв/выв  */
#include <ctype.h>                                /*вкл.подпр.классиф.симв. */

/*
******* Б Л О К  об'явлений статических рабочих переменных
*/

char NFIL [30] = "\x0";

unsigned char PRNMET = 'N';                       /*индикатор обнаруж.метки */
int I3;                                           /*счетчик цикла           */


int FDC();                                        /*подпр.обр.пс.опер.DC    */
int FDS();                                        /*подпр.обр.пс.опер.DS    */
int FEND();                                       /*подпр.обр.пс.опер.END   */
int FEQU();                                       /*подпр.обр.пс.опер.EQU   */
int FSTART();                                     /*подпр.обр.пс.опер.START */
int FUSING();                                     /*подпр.обр.пс.опер.USING */
int FRR();                                        /*подпр.обр.опер.RR-форм. */
int FRX();                                        /*подпр.обр.опер.RX-форм. */
//Our awesome input
int FRS();
int FSS();                                      /*подпр.обр.опер.SS-форм. */

int SDC();                                        /*подпр.обр.пс.опер.DC    */
int SDS();                                        /*подпр.обр.пс.опер.DS    */
int SEND();                                       /*подпр.обр.пс.опер.END   */
int SEQU();                                       /*подпр.обр.пс.опер.EQU   */
int SSTART();                                     /*подпр.обр.пс.опер.START */
int SUSING();                                     /*подпр.обр.пс.опер.USING */
int SRR();                                        /*подпр.обр.опер.RR-форм. */
int SRX();                                        /*подпр.обр.опер.RX-форм. */
//Our awesome input
int SRS();
int SSS();                                        /*подпр.обр.опер.SS-форм. */

int BitToInt( char* value )
{
	int i;
	int result = 0;
	int size = strlen( value );
	int multiplier = 1;
	for ( i = 1; i <= size; i++ )
	{
		if( value[size-i] == '1' )
		{
			result += multiplier;
		}
		multiplier *= 2;
	}
}
/*
******* ОБ'ЯВЛЕНИЕ структуры строки (карты) исходного текста
*/

struct ASSKARTA                                  /*структ.карты АССЕМБЛЕРА */
{
	unsigned  char METKA    [ 8];                  /*поле метки              */
	unsigned  char PROBEL1  [ 1];                  /*пробел-разделитель      */
	unsigned  char OPERAC   [ 5];                  /*поле операции           */
	unsigned  char PROBEL2  [ 1];                  /*пробел-разделитель      */
	unsigned  char OPERAND  [12];                  /*поле операнда           */
	unsigned  char PROBEL3  [ 1];                  /*пробел разделитель      */
	unsigned  char COMM     [52];                  /*поле комментария        */
};

/*
******* НАЛОЖЕНИЕ структуры карты исх. текста на входной буфер
*/

union                                            /*определить об'единение  */
{
	unsigned char BUFCARD [80];                    /*буфер карты.исх.текста  */
	struct ASSKARTA STRUCT_BUFCARD;                /*наложение шабл.на буфер */
} TEK_ISX_KARTA;

/*
***** СЧЕТЧИК относительного адреса (смещешия относительно базы )
*/

int CHADR;                                       /*счетчик                 */
char PL8_BUFFER[8];
char BL_BUFFER[1];
/*
***** ТАБЛИЦА символов
*/

int ITSYM = -1;                                  /*инд.своб.стр. табл.симв.*/
struct TSYM                                      /*структ.строки табл.симв.*/
{
	unsigned char IMSYM [8];                       /*имя символа             */
	int           ZNSYM;                           /*значение символа        */
	int           DLSYM;                           /*длина символа           */
	char          PRPER;                           /*признак перемещения     */
};

struct TSYM T_SYM [NSYM];                        /*определение табл.симв.  */

struct TMOP                                      /*структ.стр.табл.маш.опер*/
{
	unsigned char MNCOP [5];                       /*мнемокод операции       */
	unsigned char CODOP;                       /*машинный код операции   */
	unsigned char DLOP;                       /*длина операции в байтах */
	int (*BXPROG)();
} T_MOP [NOP]  =                                /*об'явление табл.маш.опер*/
{
	{ {'B','A','L','R',' '} , '\x05' , 2 , FRR }, /*инициализация           */
	{ {'B','C','R',' ',' '} , '\x07' , 2 , FRR }, /*строк                   */
	{ {'S','T',' ',' ',' '} , '\x50' , 4 , FRX }, /*таблицы                 */
	{ {'L',' ',' ',' ',' '} , '\x58' , 4 , FRX }, /*машинных                */
	{ {'A',' ',' ',' ',' '} , '\x5A' , 4 , FRX }, /*операций                */
	{ {'S',' ',' ',' ',' '} , '\x5B' , 4 , FRX }, /*                        */
	//Our awesome input
	{ {'S','R','L',' ',' '} , '\x88' , 4 , FRS }, /*                        */
	{ {'C','V','D',' ',' '} , '\x4E' , 4 , FRX }, /*                        */
	{ {'Z','A','P',' ',' '} , '\xF8' , 6 , FSS }, /*                        */
	{ {'I','C',' ',' ',' '} , '\x43' , 4 , FRX }, /*                        */

};

/*
***** ТАБЛИЦА псевдоопераций
*/

struct TPOP                                      /*структ.стр.табл.пс.опeр.*/
{
	unsigned char MNCPOP[5];                       /*мнемокод псевдооперации */
	int (*BXPROG) ()       ;                       /*указатель на подпр.обраб*/
} T_POP [NPOP] =                                /*об'явление табл.псевдооп*/
{
	{ { 'D','C',' ',' ',' ' } , FDC    },            /*инициализация           */
	{ { 'D','S',' ',' ',' ' } , FDS    },            /*строк                   */
	{ { 'E','N','D',' ',' ' } , FEND   },            /*таблицы                 */
	{ { 'E','Q','U',' ',' ' } , FEQU   },            /*псевдоопераций          */
	{ { 'S','T','A','R','T' } , FSTART },            /*                        */
	{ { 'U','S','I','N','G' } , FUSING }             /*                        */
};

/*
***** ТАБЛИЦА базовых регистров
*/

struct TBASR                                     /*структ.стр.табл.баз.рег.*/
{
	int SMESH;                                     /*                        */
	char PRDOST;                                   /*                        */
} T_BASR[15] =                                  /*                        */
{
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'},
	{0x00,'N'}
};


//Б Л О К   об'явления массива с об'ектным текстом

unsigned char OBJTEXT [DL_OBJTEXT][80];         /*массив об'ектных карт   */
int ITCARD = 0;                                 /*указатель текущ.карты   */
struct OPRR                                     /*структ.буф.опер.форм.RR */
{
	unsigned char OP;                             /*код операции            */
	unsigned char R1R2;                           /*R1 - первый операнд     */
	/*R2 - второй операнд     */
};

union                                           /*определить об'единение  */
{
	unsigned char BUF_OP_RR [2];                  /*оределить буфер         */
	struct OPRR OP_RR;                            /*структурировать его     */
} RR;

struct OPRX                                     /*структ.буф.опер.форм.RX */
{
	unsigned char OP;                             /*код операции            */
	unsigned char R1X2;                           /*R1 - первый операнд     */
	short B2D2;                                     /*X2 - второй операнд     */
};

union                                           /*определить об'единение  */
{
	unsigned char BUF_OP_RX [4];                  /*оределить буфер         */
	struct OPRX OP_RX;                            /*структурировать его     */
} RX;
//Our awesome input
struct OPSS                                  /*структ.буф.опер.форм.SS */
{
	unsigned char OP;                             /*код операции            */
	unsigned char X1X2;
	short B1D1;
	short B2D2;                                     
};
union                                           /*определить об'единение  */
{
	unsigned char BUF_OP_SS [6];                  /*оределить буфер         */
	struct OPSS OP_SS;                            /*структурировать его     */
} SS;
struct STR_BUF_ESD                              /*структ.буфера карты ESD */
{
	unsigned char POLE1      ;                    /*место для кода 0x02     */
	unsigned char POLE2  [ 3];                    /*поле типа об'ектн.карты */
	unsigned char POLE3  [ 6];                    /*пробелы                 */
	unsigned char POLE31 [ 2];                    /*длина данных на карте   */
	unsigned char POLE32 [ 2];                    /*пробелы                 */
	unsigned char POLE4  [ 2];                    /*внутр.ид-р имени прогр. */
	unsigned char IMPR   [ 8];                    /*имя программы           */
	unsigned char POLE6      ;                    /*код типа ESD-имени      */
	unsigned char ADPRG  [ 3];                    /*относит.адрес программы */
	unsigned char POLE8      ;                    /*пробелы                 */
	unsigned char DLPRG  [ 3];                    /*длина программы         */
	unsigned char POLE10 [40];                    /*пробелы                 */
	unsigned char POLE11 [ 8];                    /*идентификационное поле  */
};

struct STR_BUF_TXT                               /*структ.буфера карты TXT */
{
	unsigned char POLE1      ;                    /*место для кода 0x02     */
	unsigned char POLE2  [ 3];                    /*поле типа об'ектн.карты */
	unsigned char POLE3      ;                    /*пробел                  */
	unsigned char ADOP   [ 3];                    /*относит.адрес опреации  */
	unsigned char POLE5  [ 2];                    /*пробелы                 */
	unsigned char DLNOP  [ 2];                    /*длина операции          */
	unsigned char POLE7  [ 2];                    /*пробелы                 */
	unsigned char POLE71 [ 2];                    /*внутренний идент.прогр. */
	unsigned char OPER   [56];                    /*тело операции           */
	unsigned char POLE9  [ 8];                    /*идентификационное поле  */
};

struct STR_BUF_END                                /*структ.буфера карты END */
{
	unsigned char POLE1      ;                    /*место для кода 0x02     */
	unsigned char POLE2  [ 3];                    /*поле типа об'ектн.карты */
	unsigned char POLE3  [68];                    /*пробелы                 */
	unsigned char POLE9  [ 8];                    /*идентификационное поле  */
};

union                                           /*определить об'единение  */
{
	struct STR_BUF_ESD STR_ESD;                   /*структура буфера        */
	unsigned char BUF_ESD [80];                   /*буфер карты ESD         */
} ESD;


union                                           /*определить об'единение  */
{
	struct STR_BUF_TXT STR_TXT;                   /*структура буфера        */
	unsigned char BUF_TXT [80];                   /*буфер карты TXT         */
} TXT;

union                                           /*определить об'единение  */
{
	struct STR_BUF_END STR_END;                   /*структура буфера        */
	unsigned char BUF_END [80];                   /*буфер карты ESD         */
} END;


//Б Л О К  об'явлений подпрограмм, используемых при 1-ом просмотре

int FDC()                                         /*подпр.обр.пс.опер.DC    */
{
	int size = 4;
	if ( PRNMET == 'Y' )                            /*если псевдооп.DC помеч.,*/
	{
		if( TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='F' )
		{
			T_SYM[ITSYM].DLSYM = 4;                     /*  уст.длину симв. =  4, */
			T_SYM[ITSYM].PRPER = 'R';                   /*  а,призн.перемест.='R' */
			if ( CHADR % 4 )                            /*  и, если CHADR не указ.*/
			{                                          /*  на границу слова, то: */
				CHADR = (CHADR /4 + 1) * 4;               /*   уст.CHADR на гр.сл. и*/
				T_SYM[ITSYM].ZNSYM = CHADR;               /*   запомн. в табл.симв. */
			}
			PRNMET = 'N';                               /*  занулить PRNMET зн.'N'*/
		}
		//Out awesome input
		else if( TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='B' )
		{
			size = atoi( &TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[2] );
			size = 1;
			T_SYM[ITSYM].DLSYM = size;//( size - 1 ) / 8 + 1;
			T_SYM[ITSYM].PRPER = 'R';                   /*  а,призн.перемест.='R' */
			T_SYM[ITSYM].ZNSYM = CHADR;               /*   запомн. в табл.симв. */
			PRNMET = 'N';
		}
		else if( TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='P' )
		{
			size = atoi( &TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[2] );
			T_SYM[ITSYM].DLSYM = size;
			T_SYM[ITSYM].PRPER = 'R';                   /*  а,призн.перемест.='R' */
			T_SYM[ITSYM].ZNSYM = CHADR;
			PRNMET = 'N';                               /*  занулить PRNMET зн.'N'*/
		}
		else
			return 1;                                  /* иначе выход по ошибке  */
	}
	else
		if ( CHADR % 4 )  
		{                             /*и CHADR не кратен 4,то: */
			CHADR = (CHADR /4 + 1) * 4; 
		}                  /* установ.CHADR на гр.сл.*/

		CHADR = CHADR + size;                              /*увелич.CHADR на 4 и     */
		return (0);                                     /*успешно завершить подпр.*/
}
/*..........................................................................*/
int FDS()                                         /*подпр.обр.пс.опер.DS    */
{
	if ( PRNMET == 'Y' )                            /*если псевдооп.DC помеч.,*/
	{                                              /*то:                     */
		if( TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='F' )
		{
			T_SYM[ITSYM].DLSYM = 4;                     /*  уст.длину симв. =  4, */
			T_SYM[ITSYM].PRPER = 'R';                   /*  а,призн.перемест.='R' */
			if ( CHADR % 4 )                            /*  и, если CHADR не указ.*/
			{                                          /*  на границу слова, то: */
				CHADR = (CHADR /4 + 1) * 4;               /*   уст.CHADR на гр.сл. и*/
				T_SYM[ITSYM].ZNSYM = CHADR;               /*   запомн. в табл.симв. */
			}
			PRNMET = 'N';                               /*  занулить PRNMET зн.'N'*/
		}
		else
			return (1);                                  /* иначе выход по ошибке  */
	}
	else                                            /*если же псевдооп.непомеч*/
	{
		if ( CHADR % 4 ) 
		{                              /*и CHADR не кратен 4,то: */
			CHADR = (CHADR /4 + 1) * 4; 
			return(0);

		}                  /* установ.CHADR на гр.сл.*/
	}
	CHADR = CHADR + 4;                              /*увелич.CHADR на 4 и     */
	return (0);                                     /*успешно завершить подпр.*/
}

int FEND()
{
	return (100);
}

int FEQU()                                        /*подпр.обр.пс.опер.EQU   */
{
	if( TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0] =='*' )
	{
		T_SYM[ITSYM].ZNSYM = CHADR;                  /*  CHADR в поле ZNSYM,   */
		T_SYM[ITSYM].DLSYM = 1;                      /*  1 в поле DLSYM,       */
		T_SYM[ITSYM].PRPER = 'R';                    /*  'R' в пооле PRPER     */
	}
	else                                            /*иначе запомн.в табл.симв*/
	{                                             /* значение оп-нда пс.оп. */
		T_SYM[ITSYM].ZNSYM = atoi ( (char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND );
		T_SYM[ITSYM].DLSYM = 1;
		T_SYM[ITSYM].PRPER = 'A';
	}
	PRNMET = 'N';                                   /*занул.PRNMET значен.'N' */
	return (0);                                     /*успешное заверш.подпр.  */
}

int FSTART()                                      /*подпр.обр.пс.опер.START */
{                                                /*CHADR установить равным */
	CHADR = atoi( (char*)TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND );
	if ( CHADR % 8 )                                /*если это значение не    */
	{                                              /*кратно 8, то сделать его*/
		CHADR = ( CHADR + ( 8 - CHADR % 8 ) );        /*кратным                 */
	}                                              /*запомнить в табл.симв.: */
	T_SYM[ITSYM].ZNSYM = CHADR;                     /* CHADR в поле ZNSYM,    */
	T_SYM[ITSYM].DLSYM = 1;                         /* 1 в поле DLSYM,        */
	T_SYM[ITSYM].PRPER = 'R';                       /* 'R' в поле PRPER       */
	PRNMET = 'N';                                   /*занул.PRNMET значен.'N' */
	return (0);                                     /*успешное заверш.подпрогр*/
}
/*..........................................................................*/
int FUSING()                                      /*подпр.обр.пс.опер.USING */
{
	return (0);                                     /*успешное заверш.подпрогр*/
}
/*..........................................................................*/
int FRR()                                         /*подпр.обр.опер.RR-форм. */
{
	CHADR = CHADR + 2;                              /*увеличить сч.адр. на 2  */
	if ( PRNMET == 'Y' )                            /*если ранее обнар.метка, */
	{                                              /*то в табл. символов:    */
		T_SYM[ITSYM].DLSYM = 2;                       /*запомнить длину маш.опер*/
		T_SYM[ITSYM].PRPER = 'R';                     /*и установить призн.перем*/
	}
	return(0);                                      /*выйти из подпрограммы   */
}
/*..........................................................................*/
int FRX()                                         /*подпр.обр.опер.RX-форм. */
{
	CHADR = CHADR + 4;                              /*увеличить сч.адр. на 4  */
	if ( PRNMET == 'Y' )                            /*если ранее обнар.метка, */
	{                                              /*то в табл. символов:    */
		T_SYM[ITSYM].DLSYM = 4;                       /*запомнить длину маш.опер*/
		T_SYM[ITSYM].PRPER = 'R';                     /*и установить призн.перем*/
	}

	return(0);                                      /*выйти из подпрограммы   */
}
//Our awesome input
int FRS()                                         /*подпр.обр.опер.RX-форм. */
{
	CHADR = CHADR + 4;                              /*увеличить сч.адр. на 4  */
	if ( PRNMET == 'Y' )                            /*если ранее обнар.метка, */
	{                                              /*то в табл. символов:    */
		T_SYM[ITSYM].DLSYM = 4;                       /*запомнить длину маш.опер*/
		T_SYM[ITSYM].PRPER = 'R';                     /*и установить призн.перем*/
	}
	return(0);                                      /*выйти из подпрограммы   */
}
//Our awesome input
int FSS()                                         /*подпр.обр.опер.RX-форм. */
{
	CHADR = CHADR + 6;                              /*увеличить сч.адр. на 4  */
	if ( PRNMET == 'Y' )                            /*если ранее обнар.метка, */
	{                                              /*то в табл. символов:    */
		T_SYM[ITSYM].DLSYM = 6;                       /*запомнить длину маш.опер*/
		T_SYM[ITSYM].PRPER = 'R';                     /*и установить призн.перем*/
	}
	return(0);                                      /*выйти из подпрограммы   */
}
/*..........................................................................*/

/*
******* Б Л О К  об'явлений подпрограмм, используемых при 2-ом просмотре
*/

void STXT( int ARG )                              /*подпр.формир.TXT-карты  */
{
	char *PTR;                                      /*рабоч.переменная-указат.*/
	PTR = (char *)&CHADR;                           /*формирование поля ADOP  */
	TXT.STR_TXT.ADOP[2]  = *PTR;                    /*TXT-карты в формате     */
	TXT.STR_TXT.ADOP[1]  = *(PTR+1);                /*двоичного целого        */
	TXT.STR_TXT.ADOP[0]  = '\x00';                  /*в соглашениях ЕС ЭВМ    */
	//If arg
	if ( ARG == 1 )                                 /*формирование поля OPER  */
	{
		memset ( TXT.STR_TXT.OPER , 64 , 8 );
		memcpy ( TXT.STR_TXT.OPER,BL_BUFFER , 1 ); /* для RR-формата         */
		TXT.STR_TXT.DLNOP [1] = 1;
	}
	else if ( ARG == 2 )                                 /*формирование поля OPER  */
	{
		memset ( TXT.STR_TXT.OPER , 64 , 8 );
		memcpy ( TXT.STR_TXT.OPER,RR.BUF_OP_RR , 2 ); /* для RR-формата         */
		TXT.STR_TXT.DLNOP [1] = 2;
	}
	else if (ARG==4)
	{
		memset ( TXT.STR_TXT.OPER , 64 , 8 );
		memcpy ( TXT.STR_TXT.OPER , RX.BUF_OP_RX , 4);/* для RX/RS-формата         */
		TXT.STR_TXT.DLNOP [1] = 4;
	}
	//Our awesome input
	else if ( ARG == 6 )
	{
		memset ( TXT.STR_TXT.OPER , 64 , 8 );
		memcpy ( TXT.STR_TXT.OPER , SS.BUF_OP_SS , 6);/* для SS         */
		TXT.STR_TXT.DLNOP [1] = 6;
	}
	else if (ARG ==8)
	{
		memset ( TXT.STR_TXT.OPER , 64 , 8 );
		memcpy ( TXT.STR_TXT.OPER , PL8_BUFFER , 8);     /* для PL8                */
		TXT.STR_TXT.DLNOP [1] = 8;
	}
	else
	{
		memset ( TXT.STR_TXT.OPER , 64 , 8 );
		memcpy ( TXT.STR_TXT.OPER , RX.BUF_OP_RX , ARG);/* для PL         */
		TXT.STR_TXT.DLNOP [1] = ARG;
	}

	memcpy (TXT.STR_TXT.POLE9,ESD.STR_ESD.POLE11,8);/*формиров.идентифик.поля */

	memcpy ( OBJTEXT[ITCARD] , TXT.BUF_TXT , 80 );  /*запись об'ектной карты  */
	ITCARD += 1;                                    /*коррекц.инд-са своб.к-ты*/
	CHADR = CHADR + ARG;                            /*коррекц.счетчика адреса */
	return;
}

int SDC()                                         /*подпр.обр.пс.опер.DC    */
{

	char *RAB;                                      /*рабочая переменная      */

	RX.OP_RX.OP   = 0;                              /*занулим два старших     */
	RX.OP_RX.R1X2 = 0;                              /*байта RX.OP_RX          */
	if( !memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, "F'", 2) )
	{
		RAB=strtok( (char*)TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND+2, "'" );
		RX.OP_RX.B2D2 = atoi ( RAB );                 /*перевод ASCII-> int     */
		RAB = (char *) &RX.OP_RX.B2D2;                /*приведение к соглашениям*/
		swab ( RAB , RAB , 2 ); 
		STXT (4);                                       /*формирование TXT-карты  */
		return (0); 
	}
	//Our awesome input
	else if ( !memcmp( TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, "BL", 2 ) )
	{
		RAB=strtok( (char*)TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND + 4, "'" );
		//Our awesome input
		int size = strlen(RAB);
		int value = strtol( RAB, NULL, 2 );
		char buffer[1];
		buffer[0] = value<<(8-size);

		//RX.OP_RX.B2D2 =  value<<(8-size);  
		//RAB = (char *) &RX.OP_RX.B2D2;                /*приведение к соглашениям*/
		memcpy(BL_BUFFER, buffer, 1);
		STXT (1);                                       /*формирование TXT-карты  */
		return (0); 
		/*перевод ASCII-> int     */	                
	}
	else if ( !memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, "PL", 2) )
	{
		RAB=strtok( (char*)TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND+4, "'" );
		int size = TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[2]-'0';		

		char buffer[8];
		memset ( buffer , 64 , 8 );
		buffer[size-1] = 0xC;
		if (size <= 4)
		{
			memset ( buffer , 0 , size-1 );
			memcpy(RX.BUF_OP_RX, buffer, 4);
			STXT (size);                                   /*формирование TXT-карты  */
		}
		else
		{
			memset ( buffer , 0 , 7 );
			memcpy(PL8_BUFFER, buffer, 8);
			STXT (size);
		}                                       /*формирование TXT-карты  */
		return (0);            /*перевод ASCII-> int     */
	}
	else
	{
		return (1);
	}                                    /*сообщение об ошибке     */                                    /*успешн.завершение подпр.*/
}
/*..........................................................................*/
int SDS()                                         /*подпр.обр.пс.опер.DS    */
{

	RX.OP_RX.OP   = 0;                              /*занулим два старших     */
	RX.OP_RX.R1X2 = 0;                              /*байта RX.OP_RX          */
	if
		(                                             /* если операнд начинается*/
		TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='F' /* с комбинации F'        */
		)                                             /* то:                    */
		RX.OP_RX.B2D2 = 0; 
	//Our awesome input
	else 
		if (TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='0')
		{
			if ( CHADR % 4 )                            /*  и, если CHADR не указ.*/
			{                                          /*  на границу слова, то: */
				CHADR = (CHADR /4 + 1) * 4;               /*   уст.CHADR на гр.сл. и*/
			}
			return (0);
		}/*занулим RX.OP_RX.B2D2   */
		else                                            /*иначе                   */
			return (1);                                    /*сообщение об ошибке     */

	STXT (4);                                       /*формирование TXT-карты  */

	return (0);                                     /*успешно завершить подпр.*/
}
/*..........................................................................*/
int SEND()                                        /*подпр.обр.пс.опер.END   */
{
	/*формирование            */
	memcpy (                                        /*идентификационнго поля  */
		END.STR_END.POLE9 ,                     /* END - карты            */
		ESD.STR_ESD.POLE11,                     /*                        */
		8                                       /*                        */
		);                                       /*                        */
	memcpy (                                        /*запись об'ектной карты  */
		OBJTEXT[ITCARD],                        /* в                      */
		END.BUF_END,                            /* массив                 */
		80                                      /* об'ектных              */
		);                                       /* карт                   */
	ITCARD += 1;                                    /*коррекц.инд-са своб.к-ты*/
	return (100);                                   /*выход с призн.конца 2-го*/
	/*просмотра               */
}
/*..........................................................................*/
int SEQU()                                        /*подпр.обр.пс.опер.EQU   */
{
	return (0);                                     /*успешное заверш.подпр.  */
}
/*..........................................................................*/
int SSTART()                                      /*подпр.обр.пс.опер.START */
{
	char *PTR;                                      /*набор                   */
	char *METKA;                                    /*рабочих                 */
	char *METKA1;                                   /*переменных              */
	int J;                                          /*подпрограммы            */
	int RAB;                                        /*                        */

	METKA1 = strtok                                 /*в перем. c указат.METKA1*/
		(                                      /*выбираем первую лексему */
		(char*) TEK_ISX_KARTA.STRUCT_BUFCARD.METKA,   /*операнда текущей карты  */
		" "                                   /*исх.текста АССЕМБЛЕРА   */
		);
	for ( J=0; J<=ITSYM; J++ )                      /* все метки исх.текста в */
	{                                              /* табл. T_SYM сравниваем */
		/* со знач.перем. *METKA1 */
		METKA = strtok (
			(char*) T_SYM[J].IMSYM , " "
			);
		if( !strcmp ( METKA , METKA1 ) )              /* и при совпадении:      */
		{                                            /*  берем разность сч.адр.*/
			RAB = CHADR - T_SYM[J].ZNSYM;               /*  знач.этой метки, обра-*/
			PTR = (char *)&RAB;                         /*  зуя длину программы в */
			swab ( PTR , PTR , 2 );                     /*  соглашениях ЕС ЭБМ, и */
			ESD.STR_ESD.DLPRG [0] = 0;                  /*  записыв.ее в ESD-карту*/
			ESD.STR_ESD.DLPRG [1] = *PTR;               /*  побайтно              */
			ESD.STR_ESD.DLPRG [2] = *(PTR + 1);         /*                        */
			CHADR = T_SYM[J].ZNSYM;                     /*устанавл.CHADR, равным  */
			/*операнду операт.START   */
			/*исходного текста        */
			PTR = (char *)&CHADR;                       /*формирование поля ADPRG */
			ESD.STR_ESD.ADPRG[2]  = *PTR;               /*ESD-карты в формате     */
			ESD.STR_ESD.ADPRG[1]  = *(PTR+1);           /*двоичного целого        */
			ESD.STR_ESD.ADPRG[0]  = '\x00';             /*в соглашениях ЕС ЭВМ    */
			memcpy (                                    /*формирование            */
				ESD.STR_ESD.IMPR ,                 /* имени программы        */
				METKA ,                            /*  и                     */
				strlen ( METKA )                   /*                        */
				);                                   /*                        */
			memcpy (                                    /*идентификационнго поля  */
				ESD.STR_ESD.POLE11 ,               /* ESD - карты            */
				METKA ,                            /*                        */
				strlen ( METKA )                   /*                        */
				);                                   /*                        */
			memcpy (                                    /*запись об'ектной карты  */
				OBJTEXT[ITCARD] ,                  /* в                      */
				ESD.BUF_ESD ,                      /* массив                 */
				80                                 /* об'ектных              */
				);                                   /* карт                   */
			ITCARD += 1;                                /*коррекц.инд-са своб.к-ты*/
			return (0);                                 /*успешное заверш.подпрогр*/
		}
	}
	return (2);                                     /*ошибочное заверш.прогр. */
}
/*..........................................................................*/
int SUSING()                                      /*подпр.обр.пс.опер.USING */
{
	/*набор                   */
	char *METKA;                                    /*рабочих                 */
	char *METKA1;                                   /*переменных              */
	char *METKA2;                                   /*                        */
	int J;                                          /*                        */
	int NBASRG;                                     /*                        */
	METKA1 = strtok                                 /*в перем. c указат.METKA1*/
		(                                      /*выбираем первую лексему */
		(char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, /*операнда текущей карты  */
		","                                   /*исх.текста АССЕМБЛЕРА   */
		);
	METKA2 = strtok                                 /*в перем. c указат.METKA2*/
		(                                      /*выбираем вторую лексему */
		NULL,                                 /*операнда текущей карты  */
		" "                                   /*исх.текста АССЕМБЛЕРА   */
		);
	if ( isalpha ( (int) *METKA2 ) )                /*если лексема начинается */
	{                                              /*с буквы, то:            */

		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *METKA2 */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA2 ) )            /* и при совпадении:      */
			{                                          /*  запоминаем значение   */
				if ( (NBASRG = T_SYM[J].ZNSYM) <= 0x0f ) /*  метки в NBASRG и в сл.*/
					goto SUSING1;                           /*  NBASRG <= 0x0f идем на*/
				/*  устан.регистра базы   */
				else                                     /* иначе:                 */
					return (6);                             /*  сообщение об ошибке   */

			}
		}
		return (2);                                   /*заверш.подпр.по ошибке  */
	}
	else                                            /*иначе, если второй опер.*/
	{                                              /*начинается с цифры, то: */
		NBASRG = atoi ( METKA2 );                     /* запомним его в NBASRG  */
		if ( (NBASRG = T_SYM[J].ZNSYM) <= 0x0f )      /* и,если он <= 0x0f,то:  */
			goto SUSING1;                                /* идем на устан.рег.базы */
		else                                          /*иначе:                  */
			return (6);                                  /* сообщение об ошибке    */
	}

SUSING1:                                         /*установить базовый рег. */

	T_BASR [NBASRG - 1].PRDOST = 'Y';               /* взвести призн.активн.  */
	if ( *METKA1 == '*' )                           /* если перв.оп-нд == '*',*/
	{
		T_BASR[NBASRG-1].SMESH = CHADR;               /* выбир.знач.базы ==CHADR*/
	}
	else                                            /*иначе:                  */
	{
		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *METKA1 */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA1 ) )            /* и при совпадении:      */
			{                                          /*  берем значение этой   */
				T_BASR[NBASRG-1].SMESH = T_SYM[J].ZNSYM;  /*  этой метки как базу   */
			}
		}
		return (2);                                   /*завершение прогр.по ошиб*/
	}
	return (0);                                     /*успешное заверш.подпрогр*/
}
/*..........................................................................*/
int SRR()                                         /*подпр.обр.опер.RR-форм. */
{

	char *METKA;                                    /*набор                   */
	char *METKA1;                                   /*рабочих                 */
	char *METKA2;                                   /*переменных              */
	unsigned char R1R2;                             /*                        */
	int J;                                          /*                        */
	RR.OP_RR.OP = T_MOP[I3].CODOP;                  /*формирование кода операц*/

	METKA1 = strtok                                 /*в перем. c указат.METKA1*/
		(                                      /*выбираем первую лексему */
		(char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, /*операнда текущей карты  */
		","                                   /*исх.текста АССЕМБЛЕРА   */
		);

	METKA2 = strtok                                 /*в перем. c указат.METKA2*/
		(                                      /*выбираем вторую лексему */
		NULL,                                 /*операнда текущей карты  */
		" "                                   /*исх.текста АССЕМБЛЕРА   */
		);

	if ( isalpha ( (int) *METKA1 ) )                /*если лексема начинается */
	{                                              /*с буквы, то:            */
		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *METKA1 */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA1 ) )            /* и при совпадении:      */
			{                                          /*  берем значение этой   */
				R1R2 = T_SYM[J].ZNSYM << 4;              /*  метки в качестве перв.*/
				goto SRR1;
			}                                          /*  опреранда машинной ком*/
		}
		return(2);                                    /*сообщ."необ'явл.идентиф"*/
	}
	else                                            /*иначе, берем в качестве */
	{                                              /*перв.операнда машинн.ком*/
		R1R2 = atoi ( METKA1 ) << 4;                 /*значен.выбр.   лексемы  */
	}


SRR1:


	if ( isalpha ( (int) *METKA2 ) )                /*если лексема начинается */
	{                                              /*с буквы, то:            */
		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *МЕТКА2 */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA2 ) )            /* и при совпадении:      */
			{                                          /*  берем значение этой   */
				R1R2 = R1R2 + T_SYM[J].ZNSYM;            /*  метки в качестве втор.*/
				goto SRR2;                               /*                        */
			}                                          /*  опреранда машинной ком*/
		}                                            /*                        */
		return(2);                                    /*сообщ."необ'явл.идентиф"*/
	}
	else                                            /*иначе, берем в качестве */
	{                                              /*втор.операнда машинн.ком*/
		R1R2 = R1R2 + atoi ( METKA2 );               /*значен.выбр.   лексемы  */
	}

SRR2:

	RR.OP_RR.R1R2 = R1R2;                           /*формируем опер-ды маш-ой*/
	/*команды                 */

	STXT(2);
	return(0);                                      /*выйти из подпрограммы   */
}

/*..........................................................................*/
int SRX()                                         /*подпр.обр.опер.RX-форм. */
{
	char *METKA;                                    /*набор                   */
	char *METKA1;                                   /*рабочих                 */
	char *METKA2;                                   /*переменных              */
	char *PTR;                                      /*                        */
	int  DELTA;                                     /*                        */
	int  ZNSYM;                                     /*                        */
	int  NBASRG;                                    /*                        */
	int J;                                          /*                        */
	int I;                                          /*                        */
	unsigned char R1X2;                             /*                        */
	int B2D2;                                       /*                        */
	RX.OP_RX.OP = T_MOP[I3].CODOP;                  /*формирование кода операц*/
	METKA1 = strtok                                 /*в перем. c указат.METKA1*/
		(                                      /*выбираем первую лексему */
		(char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, /*операнда текущей карты  */
		","                                   /*исх.текста АССЕМБЛЕРА   */
		);


	METKA2 = strtok                                 /*в перем. c указат.METKA2*/
		(                                      /*выбираем вторую лексему */
		NULL,                                 /*операнда текущей карты  */
		" "                                   /*исх.текста АССЕМБЛЕРА   */
		);

	if ( isalpha ( (int) *METKA1 ) )                /*если лексема начинается */
	{                                              /*с буквы, то:            */
		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *METKA  */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA1 ) )            /* и при совпадении:      */

			{                                          /*  берем значение этой   */
				R1X2 = T_SYM[J].ZNSYM << 4;              /*  метки в качестве перв.*/
				goto SRX1;
			}                                          /*  опреранда машинной ком*/
		}
		return(2);                                    /*сообщ."необ'явл.идентиф"*/
	}
	else                                            /*иначе, берем в качестве */
	{                                              /*перв.операнда машинн.ком*/
		R1X2 = atoi ( METKA1 ) << 4;                 /*значен.выбр.   лексемы  */
	}


SRX1:


	if ( isalpha ( (int) *METKA2 ) )                /*если лексема начинается */
	{                                              /*с буквы, то:            */
		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *МЕТКА  */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA2 ) )            /* и при совпадении:      */
			{      
				/*  установить нач.знач.: */
				NBASRG = 0;                               /*   номера базов.регистра*/
				DELTA  = 0xfff - 1;                       /*   и его значен.,а также*/
				ZNSYM  = T_SYM[J].ZNSYM;                  /*   смещен.втор.операнда */
				for ( I=0; I<15; I++ )                    /*далее в цикле из всех   */
				{                                        /*рег-ров выберем базовым */
					if (                                    /*тот, который имеет:     */
						T_BASR[I].PRDOST == 'Y'            /* призн.активности,      */
						&&                                  /*  и                     */
						ZNSYM - T_BASR[I].SMESH >= 0       /* значенение, меньшее по */
						&&                                  /* величине,но наиболее   */
						ZNSYM - T_BASR[I].SMESH < DELTA    /* близкое к смещению вто-*/
						)                                    /* рого операнда          */
					{
						NBASRG = I + 1;
						DELTA  = ZNSYM - T_BASR[I].SMESH;
					}
				}
				if ( NBASRG == 0 || DELTA > 0xfff )       /*если баз.рег.не выбр.,то*/
					return(5);                               /* заверш.подпр.по ошибке */
				else                                      /*иначе                   */
				{                                        /* сформировыать машинное */
					B2D2 = NBASRG << 12;                    /* представление второго  */
					B2D2 = B2D2 + DELTA;                    /* операнда в виде B2D2   */
					PTR = (char *)&B2D2;                    /* и в соглашениях ЕС ЭВМ */
					swab ( PTR , PTR , 2 );                 /* с записью в тело ком-ды*/
					RX.OP_RX.B2D2 = B2D2;
				}
				goto SRX2;                                /*перех.на форм.первого   */
			}                                          /*  опреранда машинной ком*/
		}
		return(2);                                    /*сообщ."необ'явл.идентиф"*/
	}
	else                                            /*иначе, берем в качестве */
	{                                              /*втор.операнда машинн.ком*/
		return(4);                                    /*значен.выбр.   лексемы  */
	}

SRX2:

	RX.OP_RX.R1X2 = R1X2;                           /*дозапись перв.операнда  */

	STXT(4);                                        /*формирование TXT-карты  */
	return(0);                                      /*выйти из подпрограммы   */
}
//Our awesome intput
/*..........................................................................*/ 
int SSS()                                         /*подпр.обр.опер.SS-форм. */
{
	char *METKA;                                    /*набор                   */
	char *METKA1;                                   /*рабочих                 */	
	char *METKA2;                                   /*переменных              */
	char *METKA3;                                   /*                        */
	char *METKA4;                                   /*                        */
	char *PTR;                                      /*                        */
	int  DELTA;                                     /*                        */
	int  ZNSYM;                                     /*                        */
	int  NBASRG;                                    /*                        */
	int J;                                          /*                        */
	int I;                                          /*                        */
	unsigned char X1X2;                             /*                        */
	int B1D1;                                       /*                        */
	int B2D2;                                       /*                        */
	SS.OP_SS.OP = T_MOP[I3].CODOP;                  /*формирование кода операц*/
	if (isdigit(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]))
	{
		METKA1 = strtok                               /*в перем. c указат.METKA1*/
			(                                      /*выбираем первую лексему */
			(char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, /*операнда текущей карты  */
			"("                                   /*исх.текста АССЕМБЛЕРА   */
			);

		METKA2 = strtok                               /*в перем. c указат.METKA2*/
			(                                      /*выбираем длину первого  */
			NULL,                                 /*операнда текущей карты  */
			","                                   /*исх.текста АССЕМБЛЕРА   */
			);

		METKA3 = strtok                               /*в перем. c указат.METKA3*/
			(                                      /*выбираем базовый регистр*/
			NULL,                                 /*операнда текущей карты  */
			")"                                   /*исх.текста АССЕМБЛЕРА   */
			);

		METKA4 = strtok                             /*в перем. c указат.METKA2*/
			(                                       /*выбираем вторую лексему */
			NULL,                                   /*операнда текущей карты  */
			" "                                     /*исх.текста АССЕМБЛЕРА   */
			);
		METKA4++;

		X1X2 = atoi ( METKA2 ) << 4;

		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *METKA3 */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA3 ) )            /* и при совпадении:      */

			{                                          /*  берем значение этой   */
				B1D1 = T_SYM[J].ZNSYM << 12;               /*  метки в качестве перв.*/
				B1D1 = B1D1 + atoi ( METKA1 );
				PTR = (char *)&B1D1;                       /* и в соглашениях ЕС ЭВМ */
				swab ( PTR , PTR , 2 );                    /* с записью в тело ком-ды*/
				SS.OP_SS.B1D1 = B1D1;
				goto SSS1;
			}
		}
		return(2);                                    /*сообщ."необ'явл.идентиф"*/
	}
	else
	{
		METKA1 = strtok                               /*в перем. c указат.METKA1*/
			(                                      /*выбираем первую лексему */
			(char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, /*операнда текущей карты  */
			","                                   /*исх.текста АССЕМБЛЕРА   */
			);

		METKA4 = strtok                             /*в перем. c указат.METKA2*/
			(                                       /*выбираем вторую лексему */
			NULL,                                   /*операнда текущей карты  */
			" "                                     /*исх.текста АССЕМБЛЕРА   */
			);

		for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
		{                                            /* табл. T_SYM сравниваем */
			/* со знач.перем. *МЕТКА  */
			METKA = strtok (
				(char*) T_SYM[J].IMSYM , " "
				);
			if( !strcmp ( METKA , METKA1 ) )            /* и при совпадении:      */
			{                                          /*  установить нач.знач.: */
				NBASRG = 0;                                 /*   номера базов.регистра*/
				DELTA  = 0xfff - 1;                         /*   и его значен.,а также*/
				ZNSYM  = T_SYM[J].ZNSYM;                    /*   смещен.втор.операнда */
				X1X2 = (T_SYM[J].DLSYM -1)<< 4;

				for ( I=0; I<15; I++ )                    /*далее в цикле из всех   */
				{                                        /*рег-ров выберем базовым */
					if (                                    /*тот, который имеет:     */
						T_BASR[I].PRDOST == 'Y'            /* призн.активности,      */
						&&                                  /*  и                     */
						ZNSYM - T_BASR[I].SMESH >= 0       /* значенение, меньшее по */
						&&                                  /* величине,но наиболее   */
						ZNSYM - T_BASR[I].SMESH < DELTA    /* близкое к смещению вто-*/
						)                                    /* рого операнда          */
					{
						NBASRG = I + 1;
						DELTA  = ZNSYM - T_BASR[I].SMESH;
					}
				}
				if ( NBASRG == 0 || DELTA > 0xfff )       /*если баз.рег.не выбр.,то*/
					return(5);                               /* заверш.подпр.по ошибке */
				else                                      /*иначе                   */
				{                                        /* сформировыать машинное */
					B1D1 = NBASRG << 12;                    /* представление второго  */
					B1D1 = B1D1 + DELTA;                    /* операнда в виде B1D1   */
					PTR = (char *)&B1D1;                    /* и в соглашениях ЕС ЭВМ */
					swab ( PTR , PTR , 2 );                 /* с записью в тело ком-ды*/
					SS.OP_SS.B1D1 = B1D1;
				}
				goto SSS1;                                /*перех.на форм.первого   */
			}                                        /*  опреранда машинной ком*/
		}
		return(2);
	}
SSS1:
	for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
	{                                          /* табл. T_SYM сравниваем */ 						             /* со знач.перем. *МЕТКА4 */
		METKA = strtok ((char*) T_SYM[J].IMSYM , " ");
		if( !strcmp ( METKA , METKA4 ) )          /* и при совпадении:      */
		{                                        /*  установить нач.знач.: */
			NBASRG = 0;                               /*   номера базов.регистра*/
			DELTA  = 0xfff - 1;                       /*   и его значен.,а также*/
			ZNSYM  = T_SYM[J].ZNSYM;                  /*   смещен.втор.операнда */
			X1X2 = X1X2 + T_SYM[J].DLSYM-1;

			for ( I=0; I<15; I++ )                    /*далее в цикле из всех   */
			{                                        /*рег-ров выберем базовым */
				if (                                    /*тот, который имеет:     */
					T_BASR[I].PRDOST == 'Y'            /* призн.активности,      */
					&&                                  /*  и                     */
					ZNSYM - T_BASR[I].SMESH >= 0       /* значенение, меньшее по */
					&&                                  /* величине,но наиболее   */
					ZNSYM - T_BASR[I].SMESH < DELTA    /* близкое к смещению вто-*/
					)                                    /* рого операнда          */
				{
					NBASRG = I + 1;
					DELTA  = ZNSYM - T_BASR[I].SMESH;
				}
			}
			if ( NBASRG == 0 || DELTA > 0xfff )       /*если баз.рег.не выбр.,то*/
				return(5);                               /* заверш.подпр.по ошибке */
			else                                      /*иначе                   */
			{                                        /* сформировыать машинное */
				B2D2 = NBASRG << 12;                    /* представление второго  */
				B2D2 = B2D2 + DELTA;                    /* операнда в виде B2D2   */
				PTR = (char *)&B2D2;                    /* и в соглашениях ЕС ЭВМ */
				swab ( PTR , PTR , 2 );                 /* с записью в тело ком-ды*/
				SS.OP_SS.B2D2 = B2D2;
			}
			goto SSS2;                                /*перех.на форм.первого   */
		}                                        /*  опреранда машинной ком*/
	}
	return(2);                                  /*сообщ."необ'явл.идентиф"*/
SSS2:
	SS.OP_SS.X1X2 = X1X2;
	STXT(6);                                      /*формирование TXT-карты  */
	return(0);                                    /*выйти из подпрограммы   */
}

int SRS()
{
	char *METKA;                                    /*набор                   */
	char *METKA1;                                   /*рабочих                 */
	char *METKA2;                                   /*переменных              */
	char *PTR;                                      /*                        */
	int  DELTA;                                     /*                        */
	int  ZNSYM;                                     /*                        */
	int  NBASRG;                                    /*                        */
	int J;                                          /*                        */
	int I;                                          /*                        */
	unsigned char R1X2;                             /*                        */
	int B2D2;  
	int DLSYM;                                     /*                        */
	RX.OP_RX.OP = T_MOP[I3].CODOP;                  /*формирование кода операц*/
	METKA1 = strtok((char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, ",");
	METKA2 = strtok(NULL, " ");

	if ( isalpha ( (int) *METKA1 ) )              
	{                                             
		for ( J=0; J<=ITSYM; J++ )                
		{                                    
			METKA = strtok((char*) T_SYM[J].IMSYM , " ");
			if(!strcmp (METKA, METKA1))            
			{                                          /*  берем значение этой   */
				NBASRG = 0;                               /*   номера базов.регистра*/
				DELTA  = 0xfff - 1;                       /*   и его значен.,а также*/
				ZNSYM  = T_SYM[J].ZNSYM;
				DLSYM =  T_SYM[J].DLSYM;   /*   смещен.втор.операнда */
				R1X2 = T_SYM[J].ZNSYM << 4;              /*  метки в качестве перв.*/
				//Our awesome input
				DELTA  = atoi(METKA2);                       /*   и его значен.,а также*/
				B2D2 = NBASRG << 12;                    /* представление второго  */
				B2D2 = B2D2 + DELTA;                    /* операнда в виде B2D2   */ 
				PTR = (char *)&B2D2;                    /* и в соглашениях ЕС ЭВМ */
				swab ( PTR , PTR , 2 );                 /* с записью в тело ком-ды*/
				RX.OP_RX.B2D2 = B2D2;
				RX.OP_RX.R1X2 = R1X2;                           /*дозапись перв.операнда  */
				STXT(4);                                        /*формирование TXT-карты  */	
				return(0);   
			}                                       
		}
		return(2);                                    /*сообщ."необ'явл.идентиф"*/
	}
	else                                            /*иначе, берем в качестве */
	{                                              /*перв.операнда машинн.ком*/
		R1X2 = atoi ( METKA1 ) << 4;                 /*значен.выбр.   лексемы  */
	}
}

/*..........................................................................*/
int SOBJFILE()                                    /*подпрогр.формир.об'екн. */
{                                                /*файла                   */
	FILE *fp;                                       /*набор рабочих           */
	int RAB2;                                       /*переменных              */
	/*формирование пути и име-*/
	strcat ( NFIL , "tex" );                        /*ни об'ектного файла     */
	if ( (fp = fopen ( NFIL , "wb" )) == NULL )     /*при неудачн.открыт.ф-ла */
		return (-7);                                   /* сообщение об ошибке    */
	else                                            /*иначе:                  */
		RAB2 =fwrite (OBJTEXT, 80 , ITCARD , fp);      /* формируем тело об.файла*/
	fclose ( fp );                                  /*закрываем об'ектный файл*/
	return ( RAB2 );                                /*завершаем  подпрограмму */

}
/*..........................................................................*/
void INITUNION ()
{

	/*
	***** и н и ц и а л и з а ц и я   полей буфера формирования записей ESD-типа
	*****                             для выходного объектного файла
	*/

	ESD.STR_ESD.POLE1      =  0x02;
	memcpy ( ESD.STR_ESD.POLE2, "ESD", 3 );
	memset ( ESD.STR_ESD.POLE3,  0x40, 6 );
	ESD.STR_ESD.POLE31 [0] = 0x00;
	ESD.STR_ESD.POLE31 [1] = 0x10;
	memset ( ESD.STR_ESD.POLE32, 0x40, 2 );
	ESD.STR_ESD.POLE4  [0] = 0x00;
	ESD.STR_ESD.POLE4  [1] = 0x01;
	memset ( ESD.STR_ESD.IMPR,   0x40, 8 );
	ESD.STR_ESD.POLE6      = 0x00;
	memset ( ESD.STR_ESD.ADPRG,  0x00, 3 );
	ESD.STR_ESD.POLE8      = 0x40;
	memset ( ESD.STR_ESD.DLPRG,  0x00, 3 );
	memset ( ESD.STR_ESD.POLE10, 0x40,40 );
	memset ( ESD.STR_ESD.POLE11, 0x40, 8 );

	/*
	***** и н и ц и а л и з а ц и я   полей буфера формирования записей TXT-типа
	*****                             для выходного объектного файла
	*/

	TXT.STR_TXT.POLE1      =  0x02;
	memcpy ( TXT.STR_TXT.POLE2, "TXT", 3 );
	TXT.STR_TXT.POLE3      =  0x40;
	memset ( TXT.STR_TXT.ADOP,   0x00, 3 );
	memset ( TXT.STR_TXT.POLE5,  0x40, 2 );
	memset ( TXT.STR_TXT.DLNOP,  0X00, 2 );
	memset ( TXT.STR_TXT.POLE7,  0x40, 2 );
	TXT.STR_TXT.POLE71 [0] = 0x00;
	TXT.STR_TXT.POLE71 [1] = 0x01;
	memset ( TXT.STR_TXT.OPER,   0x40,56 );
	memset ( TXT.STR_TXT.POLE9,  0x40, 8 );

	/*
	***** и н и ц и а л и з а ц и я   полей буфера формирования записей END-типа
	*****                             для выходного объектного файла
	*/

	END.STR_END.POLE1      =  0x02;
	memcpy ( END.STR_END.POLE2, "END", 3 );
	memset ( END.STR_END.POLE3,  0x40,68 );
	memset ( END.STR_END.POLE9,  0x40, 8 );

}

/*..........................................................................*/

int main( int argc, char **argv )                /*главная программа       */
{
	FILE *fp;
	char *ptr = argv [1];
	unsigned char ASSTEXT [DL_ASSTEXT][80];
	int I1 , I2 , RAB;                              /* переменные цикла      */

	INITUNION ();				/* начальное заполнение  */
	/* буферов формирования  */
	/* записей выходного объ-*/
	/* ектного  файла        */

	strcpy ( NFIL, ptr );

	if ( argc != 2 )
	{
		printf ( "%s\n", "Ошибка в командной строке" );
		return;
	}


	if ( strcmp ( &NFIL [ strlen ( NFIL )-3 ], "ass" ) )
	{
		printf ( "%s\n", "Неверный тип файла с исходным текстом" );
		return;
	}
	else
	{
		if ( (fp = fopen ( NFIL , "rb" )) == NULL )   /*при неудачн.открыт.ф-ла */
		{
			printf ( "%s\n", "Не найден файл с исходным текстом" );
			return;
		}
		else
		{
			I1=0;
			while (fgets ( ASSTEXT [I1], 80, fp ) != NULL)
			{
				I1++;
				if (I1>DL_ASSTEXT)
				{
					break;
				}
			}
			goto main1;

			printf ( "%s\n", "Переполнение буфера чтения исх.текста" );
			return;
		}
	}

main1:

	fclose ( fp );
	NFIL [ strlen ( NFIL )-3 ] = '\x0';


	for ( I1=0; I1 < DL_ASSTEXT; I1++ )
	{

		memcpy ( TEK_ISX_KARTA.BUFCARD , ASSTEXT[I1], 80 );

		if ( TEK_ISX_KARTA.STRUCT_BUFCARD.METKA [0] == ' ' )
			goto CONT1;

		ITSYM += 1;                                   /* переход к след.стр.TSYM*/
		PRNMET = 'Y';                                 /* устан.призн.налич.метки*/
		memcpy ( T_SYM[ITSYM].IMSYM, TEK_ISX_KARTA.STRUCT_BUFCARD.METKA , 8 ); /* и                      */
		T_SYM[ITSYM].ZNSYM = CHADR;                   /* его значение(отн.адр.) */
CONT1:

		for ( I2=0; I2 < NPOP; I2++ )                 /*для всех стр.таб.пс.опер*/
		{          
			if( !memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_POP[I2].MNCPOP , 5) )
			{
				switch ( T_POP[I2].BXPROG () )           /* уйти в подпр.обработки */
				{
				case 0:
					goto CONT2;                            /* и завершить цикл       */
				case 1:
					goto ERR1;
				case 100:
					goto CONT3;
				}
			}
		}
		for ( I3=0; I3 < NOP ; I3++ )                 /*для всех стр.таб.м.опер.*/
		{
			if( !memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_MOP[I3].MNCOP , 5) )                                         /* то:                    */
			{
				T_MOP[I3].BXPROG ();                     /* уйти в подпр.обработки */
				PRNMET = 'N';                            /* снять призн.налич.метки*/
				goto CONT2;                              /* и завершить цикл       */
			}
		}

		goto ERR3;

CONT2:

		continue;                                     /*конец цикла обработки   */
	}                                              /*карт исх.текста         */


	/*
	******       Н А Ч А Л О   В Т О Р О Г О  П Р О С М О Т Р А      *****
	*/

CONT3:

	T_MOP[0].BXPROG = SRR;                           /*установить указатели    */
	T_MOP[1].BXPROG = SRR;                           /*на подпрограммы обраб-ки*/
	T_MOP[2].BXPROG = SRX;                           /*команд АССЕМБЛЕРА при   */
	T_MOP[3].BXPROG = SRX;                           /*втором просмотре        */
	T_MOP[4].BXPROG = SRX;
	T_MOP[5].BXPROG = SRX;

	//Our awesome input
	T_MOP[6].BXPROG = SRS;                           /*втором просмотре        */
	T_MOP[7].BXPROG = SRX;
	T_MOP[8].BXPROG = SSS;
	T_MOP[9].BXPROG = SRX;

	T_POP[0].BXPROG = SDC;                           /*установить указатели    */
	T_POP[1].BXPROG = SDS;                           /*на подпрограммы обраб-ки*/
	T_POP[2].BXPROG = SEND;                          /*псевдокоманд АССЕМБЛЕРА */
	T_POP[3].BXPROG = SEQU;                          /*при втором просмотре    */
	T_POP[4].BXPROG = SSTART;
	T_POP[5].BXPROG = SUSING;

	for ( I1=0; I1 < DL_ASSTEXT; I1++ )             /*для карт с 1 по конечную*/
	{
		memcpy ( TEK_ISX_KARTA.BUFCARD , ASSTEXT [I1], 80 );

		for ( I2=0; I2 < NPOP; I2++ )                 /*для всех стр.таб.пс.опер*/
		{                                            /*выполнить следущее:     */
			if( !memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_POP[I2].MNCPOP , 5) )
			{
				switch ( T_POP[I2].BXPROG () )           /* уйти в подпр.обработки */
				{
				case 0:
					goto CONT4;                            /* и завершить цикл       */
				case 100:                               /*уйти на формирование    */
					goto CONT5;                            /*об'ектного файла        */
				}
			}                                          /*                        */
		}

		for ( I3=0; I3 < NOP ; I3++ )
		{
			if( !memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_MOP[I3].MNCOP , 5) )
			{
				switch ( T_MOP[I3].BXPROG () )           /* уйти в подпр.обработки */
				{
				case 0:
					goto CONT4;                           /* и завершить цикл       */
				case 2:                                /*выдать диагностическое  */
					goto ERR2;                            /*сообщение               */
				case 4:                                /*выдать диагностическое  */
					goto ERR4;                            /*сообщение               */
				case 5:                                /*выдать диагностическое  */
					goto ERR5;                            /*сообщение               */
				case 6:                                /*выдать диагностическое  */
					goto ERR6;                            /*сообщение               */
				case 7:                                /*выдать диагностическое  */
					goto ERR6;                            /*сообщение               */

				}
			}

		}

CONT4:
		continue;                                      /*конец цикла обработки   */
	}                                              /*карт исх.текста         */

CONT5:

	if ( ITCARD == (RAB = SOBJFILE ()) )
		printf( "%s\n", "успешое завершение трансляции" );
	else
	{
		if ( RAB == -7 )
			goto ERR7;
		else
			printf( "%s\n", "ошибка при формировании об'ектного файла" );
	}
	return;

ERR1:
	printf ("%s\n","ошибка формата данных");        /*выдать диагностич.сообщ.*/
	goto CONT6;

ERR2:
	printf ("%s\n","необ'явленный идентификатор");  /*выдать диагностич.сообщ.*/
	goto CONT6;

ERR3:
	printf ("%s\n","ошибка кода операции");         /*выдать диагностич.сообщ.*/
	goto CONT6;

ERR4:
	printf ("%s\n","ошибка второго операнда");      /*выдать диагностич.сообщ.*/
	goto CONT6;

ERR5:
	printf ("%s\n","ошибка базирования");           /*выдать диагностич.сообщ.*/
	goto CONT6;

ERR6:
	printf ("%s\n","недопустимый номер регистра");  /*выдать диагностич.сообщ.*/
	goto CONT6;

ERR7:
	printf ("%s\n","ошибка открытия об'ектн.файла");/*выдать диагностич.сообщ.*/
	goto CONT6;


CONT6:
	printf ("%s%d\n","ошибка в карте N ",I1+1);     /*выдать диагностич.сообщ.*/

	return 0;
}
