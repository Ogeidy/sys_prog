﻿/*компиляция используя -lncurses
например, gcc absgraph.c -o absgraph -lncurses
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <curses.h>

#define  NSPIS  5                                 /*разм.списка загр.прогр. */
#define  NOBJ   50                                /*разм.масс.об'ектных карт*/
#define  DOBLZ  1024                              /*длина области загрузки  */
#define  NOP 10                                    /*кол-во обрабатываемых   */
													/* команд                 */


char NFIL [30] = "\x0";

int  IOBJC   = 0;                                 /*инд.вакантн.стр. OBJCARD*/
char OBJCARD [NOBJ][80];                          /*масс.хранен.об'ектн.карт*/

int  ISPIS   = 0;                                 /*инд.вакантн.стр. SPISOK */
char SPISOK  [NSPIS][80];                         /*масс.хранен.списка прогр*/
int LENGTH, LENGTH2;

WINDOW *wblue, *wgreen, *wred, *wcyan, *wmargenta;

struct STR_BUF_TXT                                   /*структ.буфера карты TXT */
{
	unsigned char POLE1      ;                      /*место для кода 0x02     */
	unsigned char POLE2  [ 3];                      /*поле типа об'ектн.карты */
	unsigned char POLE3      ;                      /*пробел                  */
	unsigned char ADOP   [ 3];                      /*относит.адрес опреации  */
	unsigned char POLE5  [ 2];                      /*пробелы                 */
	unsigned char DLNOP  [ 2];                      /*длина операции          */
	unsigned char POLE7  [ 2];                      /*пробелы                 */
	unsigned char POLE71 [ 2];                      /*внутренний идент.прогр. */
	unsigned char OPER   [56];                      /*тело операции           */
	unsigned char POLE9  [ 8];                      /*идентификационное поле  */
};


union                                             /*определить об'единение  */
{
	struct STR_BUF_TXT STR_TXT;                     /*структура буфера        */
	unsigned char BUF_TXT [80];                     /*буфер карты TXT         */
} TXT;


unsigned char INST [6];                           /*массив, содерж. обрабат.*/
/*команду                 */


/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int FRR();                                        /*подпр.обр.опер.RR-форм. */

/*..........................................................................*/

/*п р о т о т и п  обращ.к*/
int FRX();                                        /*подпр.обр.опер.RX-форм. */
/*..........................................................................*/


int FRS();	//Прототип обращения к подпрограмме обработки операндов RS-форм
int FSS();	//Прототип обращения к подпрограмме обработки операндов SS-форм


int X1 = 1;                                       /* инициализация коорд.   */
int Y1 = 15;                                      /* на экране              */
int R1,                                           /*номер 1-го регистра-опе-*/
												 /*ранда в форматах RR и RX*/
	R2,                                           /*номер 2-го регистра-опе-*/
												  /*ранда в формате RX      */
	D,                                            /*смещение в формате RX   */
	X,                                            /*номер индексн. регистра */
												  /*в формате RX            */
	B,											 /*номер базового регистра */
													/*в формате RX          */

	B2, //Номер второго базового регистра в формате SS 
	D2; //Второе смещение в формате SS
												  /*в формате RX            */
unsigned long I,                                  /*счетчик адр.тек.ком-ды  */
	BAS_ADDR,                                     /*адрес начала обл.загруз.*/
	I1,ADDR,ADDR2,ARG,VS;                         /*вспомогательные перем.  */
unsigned long VR[16],                             /*массив,содерж.знач.рег. */
	LIGHTPTR;                                     /*адрес начала обл.отсвет.*/


int x,y,i,j,k,kk;                                 /*рабочие переменные      */

int CUR_IND;                                      /*индекс масс.обл.загр.,  */
												/*соотв.текущ.ком-де прогр*/
int BAS_IND;                                      /*индекс масс.обл.загр.,  */
												/*соотв.первой ком-ды прог*/

union U1                                        /*постоянная часть шабло- */
{                                              /*на отсветки регистров на*/
	struct                                        /*экране консоли          */
	{
		char NOMREG  [ 3];
		char ZNEQUEL [1];
	} REGS_ASC [16];

	char BUFR [16][4];
}R_ASC;

union u2                                        /*шаблон для расчета      */
{                                              /*элементов абсолютного   */
	struct
	{
		unsigned int SMESH;
		unsigned int SEGM;
	} VAL_P;
	unsigned char *P_OBLZ ;
} POINT;

unsigned char OBLZ [DOBLZ] ;                    /*область загрузки трас-  */
/*сируемой программы      */
/*
***** ТАБЛИЦА машинных операций
*/

struct TMOP                                      /*структ.стр.табл.маш.опер*/
{
	unsigned char MNCOP [5];                       /*мнемокод операции       */
	unsigned char CODOP    ;                       /*машинный код операции   */
	unsigned char DLOP     ;                       /*длина операции в байтах */
	int (*BXPROG)()        ;                       /*указатель на подпр.обраб*/
} T_MOP [NOP]  =                                /*об'явление табл.маш.опер*/
{
	{{'B' , 'A' , 'L' , 'R' , ' '} , '\x05', 2 , FRR},/*инициализация           */
	{{'B' , 'C' , 'R' , ' ' , ' '} , '\x07', 2 , FRR}, /*строк                   */
	{{'S' , 'T' , ' ' , ' ' , ' '} , '\x50', 4 , FRX}, /*таблицы                 */
	{{'L' , ' ' , ' ' , ' ' , ' '} , '\x58', 4 , FRX}, /*машинных                */
	{{'A' , ' ' , ' ' , ' ' , ' '} , '\x5A', 4 , FRX}, /*операций                */
	{{'S' , ' ' , ' ' , ' ' , ' '} , '\x5B', 4 , FRX}, /*                        */

	//Дополнительно внедренные машинные операции
	{{'S','R','L',' ',' '} , '\x88' , 4 , FRS }, /* Логические сдвиг вправо      */
	{{'C','V','D',' ',' '} , '\x4E' , 4 , FRX }, /* Перевод в 10ное число        */
	{{'Z','A','P',' ',' '} , '\xF8' , 6 , FSS }, /* Обнуление и добавление       */
	{{'I','C',' ',' ',' '} , '\x43' , 4 , FRX } /*  Загрузка байта в регистр     */
};
//..........................................................................
//п р о г р а м м а реализации семантики команды BALR
int P_BALR(void)
{
	if (R2 != 0)
		I = VR[R2];
	if (R1 != 0)
		VR[R1] = I;

	return 0;
}
//..........................................................................
//п р о г р а м м а реализации семантики команды BCR с маской 15
int P_BCR(void)
{
	int ret;

	ret = 1;
	if (R1 == 15)
	{
		ret = 0;
		if ((VR[R2] != 0) && (R2 != 0))
			I = VR[R2];
		else
		{
			if (R2 != 0)
			{
				waddstr(wcyan, "transition at = 0 or the completion of the trace of the program after pressing any key");
				wrefresh(wcyan);
				ret = 1;
			}
		}
	}  

	return ret;
}
/*..........................................................................*/

int P_ST()                                        /*  п р о г р а м м а     */
												  /*реализации семантики    */
{                                                 /*команды ST              */
	int sm,i;                                     /*рабочие                 */
	char bytes[4];                                  /*переменные              */

	ADDR = VR[B] + VR[X] + D;                       /*вычисление абс.адреса и */
	sm = (int) (ADDR -I);                           /*смещения                */

	bytes[0] = (VR[R1] -                            /*преобразование содержим.*/
		VR[R1]% 0x1000000L)/0x1000000L;   /*РОН, использованного в  */
	bytes[1] = ((VR[R1] -                           /*качестве первого оп-да, */
		VR[R1]%0x10000L)/0x10000L)%0x100; /*к виду, принятому в     */
	bytes[2] = ((VR[R1] % 0x10000L) -               /*ЕС ЭВМ                  */
		((VR[R1]%0x10000L)%0x100))/0x100; /*                        */
	bytes[3] = (VR[R1] % 0x10000L) % 0x100;         /*                        */

	for (i=0; i<4; i++)                             /*запись преобразованого  */
		OBLZ[BAS_IND + CUR_IND + sm + i] = bytes[i];   /*значения по адресу 2-г  */
	/*операнда                */
	return 0;                                       /*успешное заверш.прогр.  */
}

/*..........................................................................*/

int P_L()                                         /*  п р о г р а м м а     */
	/*реализации семантики    */
{                                                /*команды L               */
	int sm;                                        /*рабочая переменная      */

	ADDR = VR[B] + VR[X] + D;                      /*вычисление абс.адреса и */
	sm = (int) ( ADDR - I );                       /*смещения                */
	VR[R1] =                                       /*преобразование содержим.*/
		OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L +   /*второго операнда к виду,*/
		OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L + /*принятому в IBM PC, и   */
		OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +    /*запись в РОН, использ.в */
		OBLZ[BAS_IND + CUR_IND + sm + 3];             /*качестве 1-го операнда  */

	return 0;                                      /*успешное заверш.прогр.  */
}

/*..........................................................................*/

int P_A()                                              /*  п р о г р а м м а     */
												       /*реализации семантики    */
{                                                      /*команды A               */
	int sm;                                            /*рабочая переменная      */

	ADDR = VR[B] + VR[X] + D;                          /*вычисление абс.адреса и */
	sm = ( int ) ( ADDR -I );                          /*смещения                */
	ARG = OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L+   /*формирование содержимого*/
		OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L +  /*второго операнда в сог- */
		OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +     /*лашениях ЕС ЭВМ         */
		OBLZ[BAS_IND + CUR_IND + sm + 3];              /*                        */
													   /*и                       */
	VR[R1] = VR[R1] + ARG;							   /*сложение с 1-м операндом*/

	return 0;                                       /*успешное заверш. прогр. */
}

/*..........................................................................*/

int P_S()                                         /* п р о г р а м м а      */
	/* реализации семантики   */
{                                                /* команды S              */
	int sm;                                         /*рабочая переменная      */

	ADDR = VR[B] + VR[X] + D;                       /*вычисление рабочего     */
	sm = ( int ) ( ADDR - I );                      /*адреса и смещения       */

	ARG = OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L+/*формирование содержимого*/
		OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L +/*второго операнда в сог- */
		OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +   /*лашениях ЕС ЭВМ         */
		OBLZ[BAS_IND + CUR_IND + sm + 3];            /*                        */
	/* и                      */
	VR[R1] = VR[R1] - ARG;                          /*выч-ие из 1-го операнда */

	return 0;                                       /*успешное заверш.прогр.  */
}

/*..........................................................................*/
int P_SRL()   //Программа реализации команды SRL                                  
{               
	VR[R1] >>= D; //Сдвиг регистра на велечину, записанную в смещении
	return 0;   
}

int P_CVD()   //Программа реализации команды CVD                                       
{
	int sm, i;  /*рабочие переменная      */
	unsigned char t;
	/* Вычисление абсолютного адреса операнда и смещения */
	ADDR = VR[B] + VR[X] + D;                
	sm = ( int ) ( ADDR - I );               

	/* Преобразование в десятичное представление */
	int value = VR[R1];		//Записыем значение регистра в переменную
	for (k = 7; k>=0; k--)
	{
		int i = (value % 10) << 4; //Записываем младший разряд во вторую половину байта
		value = value / 10;		   //Уменьшаем число на разряд
		i += value % 10;		   //Прибавляем более младший разряд
		value = value / 10;		   //Уменьшаем число на разряд
		OBLZ[BAS_IND + CUR_IND + sm + k] = i; //Записываем результат в соответсвующий байт
	}

	OBLZ[BAS_IND + CUR_IND + sm + 7] &= 0xF0; //Обнуляем половину младшего байта
	OBLZ[BAS_IND + CUR_IND + sm + 7] += 0xC;  //Обозначаем знак числа (+).

	return 0;                 
}
int P_ZAP() //Программа реализации команды ZAP                                           
{
	int l1 = LENGTH;	//Длина первого операнда
	int l2 = LENGTH2;	//Длина второго операнда
	int sm1 = ADDR - I; //Смещение первого операнда
	int sm2 = ADDR2 - I;//Смещение второго операнда
	while ( l2 >= 0 && l1 >= 0 )
	{
		//Побайтово копируем из второго операнда в первый
		OBLZ[BAS_IND + CUR_IND + sm1 + l1] = OBLZ[BAS_IND + CUR_IND + sm2 + l2]; 
		l1--;
		l2--;
	}
	return 0;
}

int P_IC()  //Программа реализации команды IC                                    
{                                              
	int sm;                                       
	ADDR = VR[B] + VR[X] + D;                      /*вычисление абс.адреса и */
	sm = (int) ( ADDR - I );                       /*смещения                */
	VR[R1] = OBLZ[BAS_IND + CUR_IND + sm];      /*преобразование содержим.*/       
	return 0;                                   /*второго операнда к виду,*/                          
}												/*принятому в IBM PC, и   */
												/*запись в РОН, использ.в */
												 /*качестве 1-го операнда  */
//..........................................................................
int FRR(void)
{
	int i, j;

	for (i = 0; i < NOP; i++)
	{
		if (INST[0] == T_MOP[i].CODOP)
		{
			waddstr(wgreen, "      ");
			for (j = 0; j < 5; j++)
				waddch(wgreen, T_MOP[i].MNCOP[j]);
			waddstr(wgreen, " "); 

			j = (INST[1] - (INST[1] % 0x10)) / 0x10;
			R1 = j;
			wprintw(wgreen, "%1d, ", j);
			j = INST[1] % 0x10;
			R2 = j;
			wprintw(wgreen, "%1d\n", j);
			break;
		}
	}

	return 0; 
}
//...........................................................................
int FRX(void)
{
	int i, j;

	for (i = 0; i < NOP; i++)
	{
		if (INST[0] == T_MOP[i].CODOP)
		{
			waddstr(wgreen, "  ");
			for (j = 0; j < 5; j++)
				waddch(wgreen, T_MOP[i].MNCOP[j]);
			waddstr(wgreen, " ");

			j = INST[1] >> 4;
			R1 = j;
			wprintw(wgreen, "%.1d, ", j);

			j = INST[2] % 16;
			j = j * 256 + INST[3];
			D = j;
			wprintw(wgreen, "X'%.3X'(", j);

			j = INST[1] % 16;
			X = j;
			wprintw(wgreen, "%1d, ", j);

			j = INST[2] >> 4;
			B = j;
			wprintw(wgreen, "%1d)", j);

			ADDR = VR[B] + VR[X] + D;
			wprintw(wgreen,"        %.06lX       \n", ADDR);

			break;
		}
	}

	return 0;
} 

int FRS() //Подпрограмма обработки операндов RS-форм
{
	int i, j;
		/* Цикл по всем возможным командам */
	for (i = 0; i < NOP; i++)
	{	
		/* КОП найден */
		if (INST[0] == T_MOP[i].CODOP)
		{
			/* Вывод мнемоники команды */
			waddstr(wgreen, "  ");
			for (j = 0; j < 5; j++)
				waddch(wgreen, T_MOP[i].MNCOP[j]);
			waddstr(wgreen, " ");
			//Вывод регистра
			j = INST[1] >> 4;
			R1 = j;
			wprintw(wgreen, "%.1d, ", j);

			//Вывод смещения
			j = INST[2] % 16;
			j = j * 256 + INST[3];
			D = j;
			wprintw(wgreen, "X'%.3X'(", j);

			//Вывлж индекса
			j = INST[1] % 16;
			X = j;
			wprintw(wgreen, "%1d, ", j);

			//Вывод базы
			j = INST[2] >> 4;
			B = j;
			wprintw(wgreen, "%1d)", j);

			//Вывод абсолютного адреса
			ADDR = VR[B] + VR[X] + D;
			wprintw(wgreen,"        %.06lX       \n", ADDR);
			break;
		}
	}

	return 0;
}

int FSS() //Подпрограмма обработки операндов SS-форм
{
	int i,j,k;                                        
	/* Цикл по всем возможным командам */
	for ( i=0; i < NOP; i++)          
	{                                 
		/* КОП найден */
		if ( INST [0] == T_MOP[i].CODOP )             
		{                                            
			/* Вывод мнемоники команды */
			wprintw(wgreen," ");                            
			for ( j=0; j < 5; j++)                     
				wprintw(wgreen,"%c",T_MOP[i].MNCOP[j]) ;      
			wprintw(wgreen," ");                         

			/* Вывод первого операнда */
			j = INST[2] % 16;                           
			j = j * 256 + INST[3];                      
			D = j; // Вывод смещения
			wprintw(wgreen,"X'%.3X'(",j);                      
			j = LENGTH = (INST[1]>>4);
			wprintw(wgreen,"%1d,", j+1);
			j = INST[2] >> 4;
			B = j; //Вывод базы
			wprintw(wgreen,"%1d),",j);
			ADDR = VR[B] + D; //Вычисление абсолютного адреса

			/* Вывод второго операнда */
			j = INST[4] % 16;
			j = j * 256 + INST[5];
			D2 = j;	//Вывод смещения
			wprintw(wgreen,"X'%.3X'(",j);
			LENGTH2 = LENGTH<<4;
			j = LENGTH2 = INST[1] - LENGTH2;
			wprintw(wgreen,"%1d,", j+1);
			j = INST[4] >> 4;
			B2 = j; //Вывод базы
			wprintw(wgreen,"%1d)",j);
			ADDR2 = VR[B2] + D2; //Запись абсолютного адреса

			/* Вывод абсолютных адресов операндов */
			wprintw(wgreen,"%.06lX,%.06lX\n",ADDR, ADDR2);   
			break;                                      
		}
	}

	return 0;
}
//...........................................................................   
//---------------------------------------------------------------------------
int wind(void)
{
	int j1, k, temp;

	x = 0;
	y = 16;

	kk = CUR_IND;
	LIGHTPTR = I1;

	for (j1 = 0; j1 < 8; j1++)
	{
		wprintw(wred, "%.06lX: ", I1);	
		for (j = 0; j < 4; j++)
		{
			for (k = 0; k < 4; k++)
				wprintw(wred, "%.02X", OBLZ[BAS_IND + kk + j * 4 + k]);
			waddstr(wred, " ");
		}

		waddstr(wred, "/* ");
		for (j = 0; j < 16; j++)
		{
			if (isprint (OBLZ[BAS_IND + kk]) )  
			{
				waddch(wred, OBLZ[BAS_IND + kk++]);
				wrefresh(wred);
			}	
			else 
			{
				waddstr(wred, ".");
				kk++;
			}
		}

		waddstr(wred, " */");
		I1 += 16;
	}
	wrefresh(wred);			//вывод на экран
	wclear(wred);				//очистка содержимого окна дампа

	return 0;
}
//---------------------------------------------------------------------------
//программа покомандной интерпретпции(отладки)
// загруженной программы
int sys(void) 			
{
	int res, temp;
	int ch;
	int gr_pos_x, gr_pos_y;
	int ii = 0, jj = 0;
	int gr_y;
	char wstr[80];
	int zizi = 0, tempI;


	I = BAS_ADDR;			//установить текущий адрес
	//равный начальному
	//нижнее поле     
	wmargenta = newwin(1, 80, 24, 0);
	wbkgd(wmargenta, COLOR_PAIR(COLOR_MAGENTA));
	waddstr(wmargenta, "\"PgUp\",\"PgDn\",\"Up\",\"Down\"->View dump; \"Enter\"->Run the next command");

	//строка состояния
	wcyan = newwin(1, 80, 23, 0);
	wbkgd(wcyan, COLOR_PAIR(COLOR_CYAN));

	//дамп области загрузки
	wred = newwin(8, 67, 15, 0);
	wbkgd(wred, COLOR_PAIR(COLOR_RED));

	//поле регистров
	wblue = newwin(16, 12, 0, 68);
	wbkgd(wblue, COLOR_PAIR(COLOR_BLUE));

	//текст  
	gr_pos_x = 0;
	gr_pos_y = 14; 
	gr_y = 11;
	wgreen = newwin(gr_y, 67, gr_pos_y, gr_pos_x);	//создадим новое окно
	wbkgd(wgreen, COLOR_PAIR(COLOR_GREEN));	//выбор цветовой пары


	keypad(wmargenta, TRUE);				//разрешить преобразование кодов клавиатуры

BEGIN:  

	//все допустимые коды к-нд сравнить с текущей и при 
	//совпадениизапомнить номер строки таблицы операций
	for (i = 0; i < NOP; i++)
	{
		if (OBLZ[BAS_IND + CUR_IND] == T_MOP[i].CODOP)
		{
			k = i;
			wprintw(wgreen, "%.06lX: ", I);
			//рисуем окно, выводим текст
			for (j = 0; j < 6; j++)                     /*                        */
			{                                        /*                        */
				if (j < T_MOP[i].DLOP)                  /*                        */
				{                                      /* выдать шестнадцатеричн.*/
					wprintw(wgreen, "%.02X", OBLZ[BAS_IND + CUR_IND + j]);
					/* запомнить его же в     */
					INST[j] =                                   /* переменной INST,       */
						OBLZ [BAS_IND + CUR_IND + j];/*                        */
				}                                      /*                        */
				else INST [j] = '\x00';                     /*                        */
			}
			if ((res = T_MOP[i].BXPROG()) != 0)    /* уйти в программу отобр.*/
				return (res);   			  /* ассемблерного эквивале-*/
			/* нта текущей команды,   */
			/*                        */
			goto l0;                                  /* перейти к дальнейшей  */
		}                  
	}
	return (6);

l0:
	//сдвиг окна вверх
	wrefresh(wgreen);
	ii++;
	if (gr_pos_y > 14 - gr_y + 1)
		mvwin(wgreen, gr_pos_y--, gr_pos_x);
	//при достижении некоторого положения, движение останавливается, и производится 
	//прокрутка окна
	else
	{
		for (jj = 0; jj < gr_y - 1; jj++)
		{
			temp = mvwinnstr(wgreen, jj + 1, 0, wstr, 67);    
			mvwaddnstr(wgreen, jj, 0, wstr, 67);
			wrefresh(wgreen);
		}
	} 
	wrefresh(wgreen);

	I += T_MOP[k].DLOP;                            /*коррекция счет-ка.адреса*/
	CUR_IND = ( int ) ( I - BAS_ADDR );            /*уст-ка текущ. индекса   */
	/*в массиве OBLZ          */
	I1 = I;                                        /*установка адреса начала */
	/*области отсветки        */

	for ( i = 0; i < 16; i++)
	{
		if (i < 10)
			waddstr(wblue, "R0");
		else 
			waddstr(wblue, "R");
		wprintw(wblue, "%d:", i);
		wprintw(wblue, "%.08lX", VR[i]);
	}      
	wrefresh(wblue);			//вывод на экран		  
	wclear(wblue);			//очистка окна регистров
	wind();   

	waddstr(wcyan, "готовность к выполнению очередной команды с адресом ");
	wprintw(wcyan, "%.06lX", I - T_MOP[k].DLOP);
	waddstr(wcyan, "\n");    				
	wrefresh(wcyan);
	wclear(wcyan);

WAIT:

	CUR_IND = (int)(I - BAS_ADDR);

	ch = wgetch(wmargenta);

	switch (ch)
	{
	case 10:
		{
			goto SKIP;
		}

	case  KEY_UP:
		{
			I1 = LIGHTPTR - 16;
			CUR_IND = (int)(I1 - BAS_ADDR);
			wind();
			goto WAIT;
		}

	case  KEY_DOWN:
		{
			I1 = LIGHTPTR + 16;
			CUR_IND = (int)(I1 - BAS_ADDR);
			wind();
			goto WAIT;      
		}

	case  KEY_PPAGE:
		{
			I1 = LIGHTPTR - 128;
			CUR_IND = (int)(I1 - BAS_ADDR);
			wind();
			goto WAIT;
		}

	case  KEY_NPAGE:
		{
			I1 = LIGHTPTR + 128 ;
			CUR_IND = (int)(I1 - BAS_ADDR);
			wind();
			goto WAIT;
		}
	}
	goto WAIT;

SKIP:

	switch (T_MOP[k].CODOP)                        //согласно  коду команды, 
	{                                              //селектируемой сч.адреса 
		//выбрать подпрогр.интер- 
	case 0x05 : P_BALR();                       //претации семантики      
		break;                         //текущей команды        
	case 0x07 : 
		{ 
			i = P_BCR();
			getch();
			if (i == 1)
			return 8;
		}
		break;
	case 0x50 :
		P_ST();
		break;
	case 0x58 : 
		P_L();
		break;
	case 0x5A : 
		P_A();
		break;
	case 0x5B : 
		P_S();
		break;
	//Вызов программ обработки внедренных машинных команд
	case 0x88 : 
		P_SRL();
		break;
	case 0x4E : 
		P_CVD();
		break;
	case 0xF8 : 
		P_ZAP();
		break;
	case 0x43 : 
		P_IC();
		break;
	}

	goto BEGIN;	

	delwin(wblue);			  
	delwin(wred);
	delwin(wgreen);
	delwin(wmargenta);

	return 0;
}
//...........................................................................
//..........................Инициализация curses..............................
int InitCurses(void)
{
	initscr();					//инициализация библиотеки curses
	curs_set(0);
	noecho();					//не показывать ввод
	cbreak();					//читать один символ 
	//за раз, не ждать \n
	keypad(stdscr, TRUE);				//разрешить преобразование кодов клавиатуры
	start_color(); 


	if (has_colors())
	{
		init_pair(COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
		init_pair(COLOR_GREEN, COLOR_BLACK, COLOR_GREEN);
		init_pair(COLOR_RED, COLOR_WHITE, COLOR_RED);
		init_pair(COLOR_CYAN, COLOR_BLACK, COLOR_CYAN);
		init_pair(COLOR_MAGENTA, COLOR_WHITE, COLOR_MAGENTA);
	}
	return 0;
}
//...........................................................................

int main( int argc, char **argv )                /* п р о г р а м м а      */
	/*абсолютного загрузчика  */
	/*об'ектных файлов        */
{
	int  I,K,N,J0,res;                              /*рабочие                 */
	unsigned long J;                                /*переменные              */
	FILE *fp;                                       /*программы               */
	char *ptr;

	//main programm


	if ( argc != 2 )
	{
		printf ( "%s\n", "Command line error" );
		return -1;
	}

	ptr = argv[1];
	strcpy ( NFIL, ptr );

	if ( strcmp ( &NFIL [ strlen ( NFIL )-3 ], "mod" ) )
	{
		goto ERR9;
		return -1;
	}

	if ((fp = fopen(NFIL,"rt")) == NULL)
		goto ERR1;                                     /*сообщение об ошибке     */
	else
	{
		while ( !feof( fp ) )                         /*читать все карты файла  */
		{                                            /*со списком              */
			fgets ( SPISOK [ISPIS++] , 80 , fp );       /*в массив SPISOK         */
			if ( ISPIS == NSPIS )                       /*если этот массив пере-  */
			{                                          /*полнен, то:             */
				fclose ( fp );                            /*закрыть файл со списком */
				goto ERR4;                                /*и выдать сообщение об ош*/
			}
		}
		fclose ( fp );                                /*закрыть файл SPISOK     */

		if ( ISPIS == 0 )                             /*если список пустойб     */
			/*то:                     */
			goto ERR2;                                   /* сообщение об ошибке    */
		else                                          /*иначе:                  */
			goto CONT1;                                  /* продолжить обработку   */
	}

CONT1:

	for ( I = 0; I < ISPIS; I++ )                   /*перебирая все собираемые*/
	{      
		/*об'ектные файлы,        */
		if ((fp = fopen(SPISOK[I], "rb" )) ==  NULL)                                          
			goto ERR3;                                   /*                        */
		else                                          /* иначе:                 */
		{                                          /*                        */
			while ( !feof( fp) )                        /*  читать файл до конца, */
			{                                          /*  размеcтить записи в   */   
				fread ( OBJCARD [IOBJC++] , 80 , 1 , fp );/*  массиве OBJCARD и,если*/
				if ( IOBJC == NOBJ )                      /*  считаны не все записи,*/
				{                                        /*  то:                   */
					fclose ( fp );                          /*   выдать сообщ.об ошиб.*/
					goto ERR5;                              /*                        */
				}                                                              
			}                                          /*                        */
			fclose ( fp );                              /*  закрыть очередной файл*/

			goto CONT2;                                 /*  и продолжить обработку*/
		}
	}

CONT2:

	POINT.P_OBLZ = OBLZ;                            /*расчитать абсолютный    */
	J = POINT.VAL_P.SEGM ;                          /*адрес области загрузки  */
	J = J << 4;                                     /*OBLZ в переменной J     */
	J += POINT.VAL_P.SMESH;

	if ( ( J0 = (int) J%8 ) == 0 )                  /*выровнять полученное    */
	{
		BAS_ADDR = J;                                 /*значение на границу     */
		BAS_IND  = 0;
	}
	else                                            /*двойного слова и запомн.*/
	{
		BAS_ADDR = ( ( J >> 3 ) + 1 ) << 3;           /*его в перем.BAS_ADDR,а  */
		BAS_IND = 8 - J0;                             /*соотв.индекс масс.OBLZ-в*/
	}						  /*перем.BAS_IND           */

	for ( I = 0; I < IOBJC; I++ )                   /*перебирая все считанные */
	{                                              /*карты об'ектных файлов, */
		if ( !memcmp ( &OBJCARD [I][1] , "TXT" , 3 ) )/*отобрать принадл.к типу */
		{                                            /*TXT и расчитать:        */
			memcpy ( TXT.BUF_TXT , OBJCARD [I] , 80 );  /*                        */
			J = TXT.STR_TXT.ADOP [0];                   /* в переменной J начальн.*/
			J = (J << 8) + TXT.STR_TXT.ADOP [1];        /*  индекс загрузки в мас-*/
			J = (J << 8) + TXT.STR_TXT.ADOP [2];        /*  сиве OBLZ             */
			J += BAS_IND;                               /*и                       */
			K = TXT.STR_TXT.DLNOP [0];                  /* в переменной K длину   */
			K = (K << 8) + TXT.STR_TXT.DLNOP [1];       /* загружаемых данных     */

			for ( N=0; N < K; N++ )                     /*загрузить данные с очер.*/
				OBLZ [ (int) J++ ] = TXT.STR_TXT.OPER [N]; /*об'ектной карты         */
		}
	}




	InitCurses();

	res = sys(); 

	switch (res)
	{
	case 6: 
		{
			endwin();
			goto ERR6;
		}
	case 7:
		{
			endwin();
			goto ERR7;
		}
	case 8:
		{
			endwin();
			goto ERR8;
		}
	}

	endwin(); 
END:
	printf( "\n%s\n", "Completion of treatment" );

	return 0;
	//Блок выдачи диагностических сообщений
ERR1:
	printf( "%s\n", "Error opening the file with a list of modules assembled" );
	goto END;

ERR2:
	printf( "%s\n", "An empty file with a list of modules assembled" );
	goto END;

ERR3:
	printf( "%s: %s\n" , "Error opening the file", SPISOK [I] );
	goto END;

ERR4:
	printf( "%s\n" , "Overflow list assembled modules" );
	goto END;

ERR5:
	printf( "%s\n" , "Overflow storage object maps" );
	goto END;

ERR6:
	printf( "%s\n" , "An invalid command code" );
	goto END;

ERR7:
	printf( "Interrupt - a mistake addressing\n" );
	goto END;

ERR8:
	goto END;

ERR9:
	printf ( "%s\n", "Неверный тип файла с исходным текстом" );
	goto END;
}
