/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 创建一个简单的GUI界面，使用的是官方实例，方便客户检测触摸。
*              简单修改了下界面显示效果，方便测试。
*	修改记录 :
*		版本号   日期         作者          说明
*		v1.0    2016-01-14  Eric2013  	    首版    
*                                     
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
#include "MainTask.h"



#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_BUTTON_0 (GUI_ID_USER + 0x01)
#define ID_BUTTON_1 (GUI_ID_USER + 0x02)

/*
*********************************************************************************************************
*	                       GUI_WIDGET_CREATE_INFO类型数组
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 0, 0, 480, 272, 0, 0x64, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 100, 50, 200, 50, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_1, 100, 130, 200, 50, 0, 0x0, 0 },
};

/*
*********************************************************************************************************
*	函 数 名: _cbCallback
*	功能说明: 对话框回调函数	
*	形    参: pMsg  回调参数 
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbDialog(WM_MESSAGE * pMsg) 
{
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) 
	{
		case WM_INIT_DIALOG:
			//
			// 初始化'Framewin'
			//
			hItem = pMsg->hWin;
			FRAMEWIN_SetTitleHeight(hItem, 24);
			FRAMEWIN_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
			FRAMEWIN_SetText(hItem, "USB");
			FRAMEWIN_SetFont(hItem, GUI_FONT_24B_ASCII);
		
			//
			// 初始化 'Button'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
			BUTTON_SetFont(hItem, GUI_FONT_24B_ASCII);
			BUTTON_SetText(hItem, "USB Open");
		
			//
			// 初始化 'Button'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
			BUTTON_SetFont(hItem, GUI_FONT_24B_ASCII);
			BUTTON_SetText(hItem, "USB Close");
		
		    /* 函数WM_SetSize或者WM_SetWindowPos也可以使用，用于识别不同的屏幕 */
			WM_SetXSize(pMsg->hWin, LCD_GetXSize());
			WM_SetYSize(pMsg->hWin, LCD_GetYSize());
			break;
			
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			switch(Id) 
			{
				/* 打开USB */
				case ID_BUTTON_0: 
					switch(NCode) 
					{
						case WM_NOTIFICATION_CLICKED:
							usbd_OpenMassStorage();
							break;
						
						case WM_NOTIFICATION_RELEASED:
							break;
					}
					break;

				/* 关闭USB */					
				case ID_BUTTON_1:
					switch(NCode) 
					{
						case WM_NOTIFICATION_CLICKED:
							usbd_CloseMassStorage();
							break;
						
						case WM_NOTIFICATION_RELEASED:
							break;
					}
					break;
			}
			break;
			
		default:
			WM_DefaultProc(pMsg);
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: MainTask
*	功能说明: GUI主函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MainTask(void) 
{
	/* 初始化 */
	GUI_Init();
	WM_MULTIBUF_Enable(1);
	
	GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbDialog, 0, 0, 0);
	while(1) 
	{
		GUI_Delay(10);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
