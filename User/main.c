/*
*********************************************************************************************************
*	                                  
*	ģ������ : uCOS-III
*	�ļ����� : mian.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����Ҫʵ��uCOS-II+STemWin+FatFS+USB Devce�ۺ�
*              ʵ��Ŀ�ģ�
*                1. ѧϰuCOS-II+STemWin+FatFS+USB Devce�ۺ� 
*                2. ��ʵ����ʵ��SD��ģʽU�̣�SD�����̷��ڵ���ʶ������Ĺ�����΢��һЩ�����ʹ��ʱ
*                   ���¼��ɡ�ʹ�õ���MicroUSB�ӿڡ�
*              ʵ�����ݣ�
*                1. �����������¼�������ͨ�����°���K1����ͨ�����ڴ�ӡ�����ջʹ�����
*                   ==================================================
*                  ���ȼ� ʹ��ջ ʣ��ջ �ٷֱ� ������   ������
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
*                    �����������ʹ��SecureCRT��V6���������д�������鿴��ӡ��Ϣ��
*                    App Task Start����  ��ʵ�ְ���ɨ�衣
*                    App Task MspPro���� ��ʵ�ֽ�ͼ���ܣ���ͼƬ��BMP��ʽ���浽SD���С�
*                    App Task UserIF���� ��������Ϣ����
*                    App Task COM����    ��LED��˸��
*                    App Task GUI����    ��GUI����
*                2. (1) �����õ�printf������ȫ��ͨ������App_Printfʵ�֡�
*                   (2) App_Printf���������ź����Ļ�������������Դ�������⡣
*                3. K2�������£�ʵ�ֽ�ͼ���ܣ���ͼƬ��BMP��ʽ���浽SD���С�
*                4. emWin�����ϵ�������ť�ֱ�ʵ�ִ�USB�豸�͹ر�USB�豸
*              ע�����
*                 1. ����û�ͨ��MDK����IAR����������FPU����ôʹ���˸�������������ڴ�����ʱ��
*                    ���Ҫʹ�ܲ���OS_OPT_TASK_SAVE_FP���мǣ� û���õ����������������Ҫ�˲�����
*                    �����˴˲����ſ��ԶԸ���Ĵ���������ջ�ͳ�ջ������
*                 2. Ϊ��֧��uC/Probe�������:
*                      (1).MDKҪ��ѡoption->output -> Debug Infomationѡ�ѡ���Ż��ȼ�0
*                      (2).IARҪѡ��ѡ���Ż��ȼ�low���������������ط��ȹ�ѡ
*                        a. option->C/C++ Compiler->Outputѡ���й�ѡGenerate debug information��
*                        b. option->linker->Outputѡ���й�ѡInclude Generate debug information in output��
*                      (3).ʹ��uCOS-III�궨��
*                          #define OS_CFG_DBG_EN             1u
*                      	   #define OS_CFG_STAT_TASK_EN       1u ͬʱ��ʼ��ͳ������
*                      	   #define CPU_CFG_INT_DIS_MEAS_EN   1u 
*                      (4).Ϊ�˷�����ԣ�����OS_CFG_APP_HOOKS_EN��OS_CFG_ARG_CHK_EN��ʹ��
*                 3. ֧��IAR7.5�Դ���uCOS-III���������
*                 4. ��ʵ���Ƽ�ʹ�ô������SecureCRT��Ҫ�����ڴ�ӡЧ�������롣�������
*                    V6��������������С�
*                 5. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����            ˵��
*       V1.0    2016-02-18   Eric2013    1. ST�̼���1.6.1�汾
*                                        2. BSP������V1.2
*                                        3. uCOS-II�汾V3.05.00
*                                        4. uC/CPU�汾V1.30.02
*                                        5. uC/LIB�汾V1.38.01
*                                        6. FatFS�汾V0.11a
*                                        7. USB��汾V2.2.0
*                                       
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
#include "MainTask.h"


/*
*********************************************************************************************************
*                                       ��̬ȫ�ֱ���
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
*                                      ��������
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
*                               ����
*******************************************************************************************************
*/
static  OS_SEM       AppPrintfSemp;	/* ����printf���� */
static  OS_SEM       SEM_SYNCH;	    /* ����ͬ�� */

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;  
	
	/* ��ʼ��uC/OS-III �ں� */
    OSInit(&err);  

	/* ����һ����������Ҳ���������񣩡���������ᴴ�����е�Ӧ�ó������� */
	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,  /* ������ƿ��ַ */           
                 (CPU_CHAR     *)"App Task Start",  /* ������ */
                 (OS_TASK_PTR   )AppTaskStart,      /* ������������ַ */
                 (void         *)0,                 /* ���ݸ�����Ĳ��� */
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO, /* �������ȼ� */
                 (CPU_STK      *)&AppTaskStartStk[0],     /* ��ջ����ַ */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10, /* ��ջ������������ʾ��10%��Ϊ����� */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,      /* ��ջ�ռ��С */
                 (OS_MSG_QTY    )0,  /* ������֧�ֽ��ܵ������Ϣ�� */
                 (OS_TICK       )0,  /* ����ʱ��Ƭ */
                 (void         *)0,  /* ��ջ�ռ��С */  
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		
				 /*  �������£�
					OS_OPT_TASK_STK_CHK      ʹ�ܼ������ջ��ͳ������ջ���õĺ�δ�õ�
					OS_OPT_TASK_STK_CLR      �ڴ�������ʱ����������ջ
					OS_OPT_TASK_SAVE_FP      ���CPU�и���Ĵ��������������л�ʱ���渡��Ĵ���������
				 */  
                 (OS_ERR       *)&err);

	/* ����������ϵͳ������Ȩ����uC/OS-III */
    OSStart(&err);                                               
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskStart
*	����˵��: ����һ�����������ڶ�����ϵͳ�����󣬱����ʼ���δ����������������Ҫʵ�ְ�����⡣
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 2
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	uint8_t  ucCount = 0;
	uint8_t  ucCount1 = 0;
	
    /* �����ڱ���������澯����������������κ�Ŀ����� */	
    (void)p_arg;  
	
	/* BSP ��ʼ���� BSP = Board Support Package �弶֧�ְ����������Ϊ�ײ�������*/
	CPU_Init();  /* �˺���Ҫ���ȵ��ã���Ϊ����������ʹ�õ�us��ms�ӳ��ǻ��ڴ˺����� */
	bsp_Init();       
	BSP_Tick_Init();  
	
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

#if 0
	/* ����Ӧ�ó�������� */
	AppTaskCreate();
	
	/* ��������ͨ�� */
	AppObjCreate();
#else
	AppTaskCreate();
	AppObjCreate();
#endif
    while(1)
    {
		/* 1msһ�δ���ɨ�裬���败���� */
		if(g_tTP.Enable == 1)
		{
			TOUCH_Scan();
			
			/* ����ɨ�� */
			ucCount++;
			if(ucCount == 10)
			{
				ucCount = 0;
				bsp_KeyScan();
			}
		    OSTimeDly(1, OS_OPT_TIME_DLY, &err);			
		}
		
		/* 20msһ�δ���ɨ�裬���ݴ�����GT811 
		   GT811ȡ20ms�Ƚ��ȶ���ȡ10msż������������
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
		
		/* 10msһ�δ���ɨ�裬���ݴ�����FT5X06 */
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
*	�� �� ��: AppTaskMsgPro
*	����˵��: ʵ�ֽ�ͼ���ܣ���ͼƬ��BMP��ʽ���浽SD����
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 3
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
		 /* �ȴ���ȡ�ź���ͬ����Ϣ�����յ���ִ�д��ڴ�ӡ */
		OSSemPend((OS_SEM *)&SEM_SYNCH,
				  (OS_TICK )0,
				  (OS_OPT  )OS_OPT_PEND_BLOCKING,
				  (CPU_TS  )0,
				  (OS_ERR *)&err);
		
		if(err == OS_ERR_NONE)
		{	
			sprintf(buf,"0:/PicSave/%d.bmp",Pic_Name);
			OSSchedLock(&err);
			
			/* ���SD����û��PicSave�ļ�������д��� */
			result = f_mkdir("0:/PicSave");
			/* ������ͼ */
			result = f_open(&file,buf, FA_WRITE|FA_CREATE_ALWAYS);
			/* ��SD������BMPͼƬ */
			GUI_BMP_Serialize(_WriteByte2File, &file);
			
			/* ������ɺ�ر�file */
		    result = f_close(&file);
			
			OSSchedUnlock(&err);
			Pic_Name++; 		
		}										  	 	       											  
	}   
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskUserIF
*	����˵��: ������Ϣ����
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 4
*********************************************************************************************************
*/
static void AppTaskUserIF(void *p_arg)
{
	OS_ERR      err;
	uint8_t  ucKeyCode;
	
	(void)p_arg;	              /* ������������� */
	
  
	while (1) 
	{   		
		ucKeyCode = bsp_GetKey();
		
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			  /* K1������ ��ӡ����ִ����� */
					DispTaskInfo();	     
					break;
				
				case KEY_DOWN_K2:			  /* K2������ ����Ϣ���з������� */ 
					OSSemPost((OS_SEM *)&SEM_SYNCH,
						      (OS_OPT  )OS_OPT_POST_1,
						      (OS_ERR *)&err);  
					break;			
				
				default:                     /* �����ļ�ֵ������ */
					break;
			}
		}
		
		OSTimeDly(20, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCom
*	����˵��: LED��˸
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 5
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
*	�� �� ��: AppTaskGUI
*	����˵��: GUI����������ȼ�		  			  
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ����OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
static void AppTaskGUI(void *p_arg)
{
    (void)p_arg;		/* ����������澯 */
		
	while (1) 
	{
		MainTask();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;
	
	/**************����MsgPro����*********************/
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
				 
	/**************����USER IF����*********************/
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

	/**************����COM����*********************/
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

	/**************����GUI����*********************/			 
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
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨѶ
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
	OS_ERR      err;
	
	/* 
	   �����ź�����ֵΪ1��ʱ�����ʵ�ֻ��⹦�ܣ�Ҳ����ֻ��һ����Դ����ʹ�� 
	   �������ǽ�����1�Ĵ�ӡ������Ϊ��������Դ����ֹ���ڴ�ӡ��ʱ������������ռ
	   ��ɴ��ڴ�ӡ���ҡ�
	*/
	OSSemCreate((OS_SEM    *)&AppPrintfSemp,
				(CPU_CHAR  *)"AppPrintfSemp",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* ��������ֵΪ0������ʵ������ͬ������ */
	OSSemCreate((OS_SEM    *)&SEM_SYNCH,
				(CPU_CHAR  *)"SEM_SYNCH",
				(OS_SEM_CTR )0,
				(OS_ERR    *)&err);
}

/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
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

	/* ������� */
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
*	�� �� ��: DispTaskInfo
*	����˵��: ��uCOS-III������Ϣͨ�����ڴ�ӡ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
	OS_TCB      *p_tcb;	        /* ����һ��������ƿ�ָ��, TCB = TASK CONTROL BLOCK */
	float CPU = 0.0f;
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
    p_tcb = OSTaskDbgListPtr;
    CPU_CRITICAL_EXIT();
	
	/* ��ӡ���� */
	App_Printf("===============================================================\r\n");
	App_Printf(" ���ȼ� ʹ��ջ ʣ��ջ �ٷֱ� ������   ������\r\n");
	App_Printf("  Prio   Used  Free   Per    CPU     Taskname\r\n");

	/* ����������ƿ��б�(TCB list)����ӡ���е���������ȼ������� */
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
