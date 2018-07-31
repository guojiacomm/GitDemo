/*
*********************************************************************************************************
*	                                  
*	模块名称 : uCOS-III
*	文件名称 : mian.c
*	版    本 : V1.0
*	说    明 : 本实验主要实现uCOS-II+STemWin+FatFS+USB Devce综合
*              实验目的：
*                1. 学习uCOS-II+STemWin+FatFS+USB Devce综合 
*                2. 本实验是实现SD卡模式U盘，SD卡的盘符在电脑识别出来的过程稍微慢一些，大家使用时
*                   等下即可。使用的是MicroUSB接口。
*              实验内容：
*                1. 共创建了如下几个任务，通过按下按键K1可以通过串口打印任务堆栈使用情况
*                   ==================================================
*                  优先级 使用栈 剩余栈 百分比 利用率   任务名
*                  Prio   Used  Free   Per    CPU     Taskname
*                   28    365    659   35%   12.79%   App Task GUI
*                    5     42    470   08%    0.00%   App Task COM
*                    4    179    333   34%    0.04%   App Task UserIF
*                    3     59    453   11%    0.00%   App Msp Pro
*                    2     56    456   10%    1.52%   App Task Start
*                   29     48     80   37%    0.01%   uC/OS-III Timer Task
*                   30     46     54   46%    0.17%   uC/OS-III Stat Task
*                    1     46     82   35%    1.14%   uC/OS-III Tick Task
*                   31     34     94   26%   84.30%   uC/OS-III Idle Task
*                    串口软件建议使用SecureCRT（V6光盘里面有此软件）查看打印信息。
*                    App Task Start任务  ：实现按键扫描。
*                    App Task MspPro任务 ：实现截图功能，将图片以BMP格式保存到SD卡中。
*                    App Task UserIF任务 ：按键消息处理。
*                    App Task COM任务    ：LED闪烁。
*                    App Task GUI任务    ：GUI任务。
*                2. (1) 凡是用到printf函数的全部通过函数App_Printf实现。
*                   (2) App_Printf函数做了信号量的互斥操作，解决资源共享问题。
*                3. K2按键按下，实现截图功能，将图片以BMP格式保存到SD卡中。
*                4. emWin界面上的两个按钮分别实现打开USB设备和关闭USB设备
*              注意事项：
*                 1. 如果用户通过MDK或者IAR编译器开启FPU，那么使用了浮点运算的任务在创建的时候
*                    务必要使能参数OS_OPT_TASK_SAVE_FP，切记！ 没有用到浮点运算的任务不需要此参数，
*                    设置了此参数才可以对浮点寄存器进行入栈和出栈操作。
*                 2. 为了支持uC/Probe调试组件:
*                      (1).MDK要勾选option->output -> Debug Infomation选项，选择优化等级0
*                      (2).IAR要选择选择优化等级low，其中如下两个地方比勾选
*                        a. option->C/C++ Compiler->Output选项中勾选Generate debug information。
*                        b. option->linker->Output选项中勾选Include Generate debug information in output。
*                      (3).使能uCOS-III宏定义
*                          #define OS_CFG_DBG_EN             1u
*                      	   #define OS_CFG_STAT_TASK_EN       1u 同时初始化统计任务
*                      	   #define CPU_CFG_INT_DIS_MEAS_EN   1u 
*                      (4).为了方便调试，参数OS_CFG_APP_HOOKS_EN，OS_CFG_ARG_CHK_EN都使能
*                 3. 支持IAR7.5自带的uCOS-III调试组件。
*                 4. 本实验推荐使用串口软件SecureCRT，要不串口打印效果不整齐。此软件在
*                    V6开发板光盘里面有。
*                 5. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者            说明
*       V1.0    2016-02-18   Eric2013    1. ST固件库1.6.1版本
*                                        2. BSP驱动包V1.2
*                                        3. uCOS-II版本V3.05.00
*                                        4. uC/CPU版本V1.30.02
*                                        5. uC/LIB版本V1.38.01
*                                        6. FatFS版本V0.11a
*                                        7. USB库版本V2.2.0
*                                       
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
#include "MainTask.h"


/*
*********************************************************************************************************
*                                       静态全局变量
*********************************************************************************************************
*/                                                          
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   AppTaskMsgProTCB;
static  CPU_STK  AppTaskMsgProStk[APP_CFG_TASK_MsgPro_STK_SIZE];

static  OS_TCB   AppTaskCOMTCB;
static  CPU_STK  AppTaskCOMStk[APP_CFG_TASK_COM_STK_SIZE];

static  OS_TCB   AppTaskUserIFTCB;
static  CPU_STK  AppTaskUserIFStk[APP_CFG_TASK_USER_IF_STK_SIZE];

static  OS_TCB   AppTaskGUITCB;
static  CPU_STK  AppTaskGUIStk[APP_CFG_TASK_GUI_STK_SIZE];

/*
*********************************************************************************************************
*                                      函数声明
*********************************************************************************************************
*/
static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskMsgPro         (void     *p_arg);
static  void  AppTaskUserIF         (void     *p_arg);
static  void  AppTaskCOM			(void 	  *p_arg);
static  void  AppTaskCreate         (void);
static  void  DispTaskInfo          (void);
static  void  AppObjCreate          (void);
static  void  App_Printf (CPU_CHAR *format, ...);

/*
*******************************************************************************************************
*                               变量
*******************************************************************************************************
*/
static  OS_SEM       AppPrintfSemp;	/* 用于printf互斥 */
static  OS_SEM       SEM_SYNCH;	    /* 用于同步 */

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;  
	
	/* 初始化uC/OS-III 内核 */
    OSInit(&err);  

	/* 创建一个启动任务（也就是主任务）。启动任务会创建所有的应用程序任务 */
	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,  /* 任务控制块地址 */           
                 (CPU_CHAR     *)"App Task Start",  /* 任务名 */
                 (OS_TASK_PTR   )AppTaskStart,      /* 启动任务函数地址 */
                 (void         *)0,                 /* 传递给任务的参数 */
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO, /* 任务优先级 */
                 (CPU_STK      *)&AppTaskStartStk[0],     /* 堆栈基地址 */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10, /* 堆栈监测区，这里表示后10%作为监测区 */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,      /* 堆栈空间大小 */
                 (OS_MSG_QTY    )0,  /* 本任务支持接受的最大消息数 */
                 (OS_TICK       )0,  /* 设置时间片 */
                 (void         *)0,  /* 堆栈空间大小 */  
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		
				 /*  定义如下：
					OS_OPT_TASK_STK_CHK      使能检测任务栈，统计任务栈已用的和未用的
					OS_OPT_TASK_STK_CLR      在创建任务时，清零任务栈
					OS_OPT_TASK_SAVE_FP      如果CPU有浮点寄存器，则在任务切换时保存浮点寄存器的内容
				 */  
                 (OS_ERR       *)&err);

	/* 启动多任务系统，控制权交给uC/OS-III */
    OSStart(&err);                                               
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器。本任务主要实现按键检测。
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 2
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	uint8_t  ucCount = 0;
	uint8_t  ucCount1 = 0;
	
    /* 仅用于避免编译器告警，编译器不会产生任何目标代码 */	
    (void)p_arg;  
	
	/* BSP 初始化。 BSP = Board Support Package 板级支持包，可以理解为底层驱动。*/
	CPU_Init();  /* 此函数要优先调用，因为外设驱动中使用的us和ms延迟是基于此函数的 */
	bsp_Init();       
	BSP_Tick_Init();  
	
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
		
	/* 创建应用程序的任务 */
	AppTaskCreate();
	
	/* 创建任务通信 */
	AppObjCreate();
	
    while(1)
    {
		/* 1ms一次触摸扫描，电阻触摸屏 */
		if(g_tTP.Enable == 1)
		{
			TOUCH_Scan();
			
			/* 按键扫描 */
			ucCount++;
			if(ucCount == 10)
			{
				ucCount = 0;
				bsp_KeyScan();
			}
		    OSTimeDly(1, OS_OPT_TIME_DLY, &err);			
		}
		
		/* 20ms一次触摸扫描，电容触摸屏GT811 
		   GT811取20ms比较稳定，取10ms偶尔会有跳动。
		*/
		if(g_GT811.Enable == 1)
		{
			bsp_KeyScan();
			ucCount1++;
			if(ucCount1 == 2)
			{
				ucCount1 = 0;
				GT811_OnePiontScan();
			}
		    OSTimeDly(10, OS_OPT_TIME_DLY, &err);		
		}
		
		/* 10ms一次触摸扫描，电容触摸屏FT5X06 */
		if(g_tFT5X06.Enable == 1)
		{
			bsp_KeyScan();
			FT5X06_OnePiontScan();
		    OSTimeDly(10, OS_OPT_TIME_DLY, &err);
		}
    }     
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskMsgPro
*	功能说明: 实现截图功能，将图片以BMP格式保存到SD卡中
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 3
*********************************************************************************************************
*/
static void AppTaskMsgPro(void *p_arg)
{
	OS_ERR      err;
	uint8_t		Pic_Name = 0;
	char buf[20];

	(void)p_arg;
		  
	while(1)
	{	
		 /* 等待获取信号量同步消息，接收到后执行串口打印 */
		OSSemPend((OS_SEM *)&SEM_SYNCH,
				  (OS_TICK )0,
				  (OS_OPT  )OS_OPT_PEND_BLOCKING,
				  (CPU_TS  )0,
				  (OS_ERR *)&err);
		
		if(err == OS_ERR_NONE)
		{	
			sprintf(buf,"0:/PicSave/%d.bmp",Pic_Name);
			OSSchedLock(&err);
			
			/* 如果SD卡中没有PicSave文件，会进行创建 */
			result = f_mkdir("0:/PicSave");
			/* 创建截图 */
			result = f_open(&file,buf, FA_WRITE|FA_CREATE_ALWAYS);
			/* 向SD卡绘制BMP图片 */
			GUI_BMP_Serialize(_WriteByte2File, &file);
			
			/* 创建完成后关闭file */
		    result = f_close(&file);
			
			OSSchedUnlock(&err);
			Pic_Name++; 		
		}										  	 	       											  
	}   
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: 按键消息处理
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 4
*********************************************************************************************************
*/
static void AppTaskUserIF(void *p_arg)
{
	OS_ERR      err;
	uint8_t  ucKeyCode;
	
	(void)p_arg;	              /* 避免编译器报警 */
	
  
	while (1) 
	{   		
		ucKeyCode = bsp_GetKey();
		
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			  /* K1键按下 打印任务执行情况 */
					DispTaskInfo();	     
					break;
				
				case KEY_DOWN_K2:			  /* K2键按下 向消息队列发送数据 */ 
					OSSemPost((OS_SEM *)&SEM_SYNCH,
						      (OS_OPT  )OS_OPT_POST_1,
						      (OS_ERR *)&err);  
					break;			
				
				default:                     /* 其他的键值不处理 */
					break;
			}
		}
		
		OSTimeDly(20, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCom
*	功能说明: LED闪烁
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 5
*********************************************************************************************************
*/
static void AppTaskCOM(void *p_arg)
{
	OS_ERR  err;  	
	
	(void)p_arg;
	
	while(1)
	{	
		//bsp_LedToggle(2);
	    OSTimeDly(500, OS_OPT_TIME_DLY, &err);
	} 						  	 	       											   
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUI
*	功能说明: GUI任务，最低优先级		  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级：OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
static void AppTaskGUI(void *p_arg)
{
    (void)p_arg;		/* 避免编译器告警 */
		
	while (1) 
	{
		MainTask();
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;
	
	/**************创建MsgPro任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskMsgProTCB,             
                 (CPU_CHAR     *)"App Msp Pro",
                 (OS_TASK_PTR   )AppTaskMsgPro, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_MsgPro_PRIO,
                 (CPU_STK      *)&AppTaskMsgProStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_MsgPro_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_MsgPro_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
				 
	/**************创建USER IF任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskUserIFTCB,             
                 (CPU_CHAR     *)"App Task UserIF",
                 (OS_TASK_PTR   )AppTaskUserIF, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_USER_IF_PRIO,
                 (CPU_STK      *)&AppTaskUserIFStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_SAVE_FP | OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

	/**************创建COM任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskCOMTCB,            
                 (CPU_CHAR     *)"App Task COM",
                 (OS_TASK_PTR   )AppTaskCOM, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_COM_PRIO,
                 (CPU_STK      *)&AppTaskCOMStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);	

	/**************创建GUI任务*********************/			 
	OSTaskCreate((OS_TCB       *)&AppTaskGUITCB,              
                 (CPU_CHAR     *)"App Task GUI",
                 (OS_TASK_PTR   )AppTaskGUI, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_GUI_PRIO,
                 (CPU_STK      *)&AppTaskGUIStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR       *)&err);				 
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通讯
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
	OS_ERR      err;
	
	/* 
	   创建信号量数值为1的时候可以实现互斥功能，也就是只有一个资源可以使用 
	   本例程是将串口1的打印函数作为保护的资源。防止串口打印的时候被其它任务抢占
	   造成串口打印错乱。
	*/
	OSSemCreate((OS_SEM    *)&AppPrintfSemp,
				(CPU_CHAR  *)"AppPrintfSemp",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* 创建计数值为0，用于实现任务同步功能 */
	OSSemCreate((OS_SEM    *)&SEM_SYNCH,
				(CPU_CHAR  *)"SEM_SYNCH",
				(OS_SEM_CTR )0,
				(OS_ERR    *)&err);
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  App_Printf(CPU_CHAR *format, ...)
{
    CPU_CHAR  buf_str[80 + 1];
    va_list   v_args;
    OS_ERR    os_err;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* 互斥操作 */
    OSSemPend((OS_SEM  *)&AppPrintfSemp,
              (OS_TICK  )0u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&os_err);

    printf("%s", buf_str);

   (void)OSSemPost((OS_SEM  *)&AppPrintfSemp,
                   (OS_OPT   )OS_OPT_POST_1,
                   (OS_ERR  *)&os_err);

}

/*
*********************************************************************************************************
*	函 数 名: DispTaskInfo
*	功能说明: 将uCOS-III任务信息通过串口打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
	OS_TCB      *p_tcb;	        /* 定义一个任务控制块指针, TCB = TASK CONTROL BLOCK */
	float CPU = 0.0f;
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
    p_tcb = OSTaskDbgListPtr;
    CPU_CRITICAL_EXIT();
	
	/* 打印标题 */
	App_Printf("===============================================================\r\n");
	App_Printf(" 优先级 使用栈 剩余栈 百分比 利用率   任务名\r\n");
	App_Printf("  Prio   Used  Free   Per    CPU     Taskname\r\n");

	/* 遍历任务控制块列表(TCB list)，打印所有的任务的优先级和名称 */
	while (p_tcb != (OS_TCB *)0) 
	{
		CPU = (float)p_tcb->CPUUsage / 100;
		App_Printf("   %2d  %5d  %5d   %02d%%   %5.2f%%   %s\r\n", 
		p_tcb->Prio, 
		p_tcb->StkUsed, 
		p_tcb->StkFree, 
		(p_tcb->StkUsed * 100) / (p_tcb->StkUsed + p_tcb->StkFree),
		CPU,
		p_tcb->NamePtr);		
	 	
		CPU_CRITICAL_ENTER();
        p_tcb = p_tcb->DbgNextPtr;
        CPU_CRITICAL_EXIT();
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
