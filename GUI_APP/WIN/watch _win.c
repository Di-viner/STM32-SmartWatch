#include "watch_win.h"
#include "gui_app.h"
#include <stdio.h>

static lv_task_t *task = NULL;
static lv_obj_t *sec_label;
static lv_obj_t *func_label;
static lv_obj_t *clear_label;
static lv_obj_t *func_btn;
static lv_obj_t *clear_btn;
static lv_style_t sec_label_style;

static u8 min;
static u8 sec;
static u8 msec;

static char isCounting = 0;

LV_FONT_DECLARE(my_font_76);

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == USER_EVENT_BACK) // 窗体返回事件,点击左上角的"< Back"按钮会触发此事件
	{
		// printf("quit\r\n");
		if (task) // 删除掉任务
		{
			isCounting = 0;
			min = 0;
			sec = 0;
			msec = 0;
			lv_task_del(task);
			task = NULL;
		}
	}
	else
	{
		if (obj == clear_btn)
		{
			if (event == LV_EVENT_RELEASED && isCounting == 0)
			{
				min = 0;
				sec = 0;
				msec = 0;
			}
		}
		if (obj == func_btn)
		{
			if (event == LV_EVENT_RELEASED)
			{
				isCounting = !isCounting;
				if (isCounting == 0)
				{ // 暂停计数
					lv_label_set_text(func_label, "#FF0000 START#");
				}
				else
				{
					lv_label_set_text(func_label, "STOP#");
				}
			}
		}
	}
}

static void sec_task(lv_task_t *t)
{
	if (isCounting)
	{
		msec = (msec + 1) % 10;
		if (msec == 0)
		{
			sec = (sec + 1) % 60;
			if (sec == 0)
			{
				min = (min + 1) % 100;
			}
		}
	}
	char buff[20];
	sprintf(buff, "%d%d:%d%d.%d", min / 10, min % 10, sec / 10, sec % 10, msec);
	lv_label_set_text(sec_label, buff);
}

void watch_win_create(lv_obj_t *parent)
{
	lv_obj_t *win = general_win_create(parent, "Stopwatch");
	lv_obj_set_event_cb(win, event_handler); // 设置事件回调函数

	sec_label = lv_label_create(win, NULL);
	lv_style_copy(&sec_label_style, &lv_style_plain_color);
	sec_label_style.text.font = &my_font_76;
	lv_label_set_style(sec_label, LV_LABEL_STYLE_MAIN, &sec_label_style);
	lv_label_set_text(sec_label, "00:00.0");
	lv_obj_align(sec_label, win, LV_ALIGN_IN_TOP_MID, 0, 150);

	func_btn = lv_btn_create(win, NULL);
	lv_obj_align(func_btn, win, LV_ALIGN_CENTER, -50, -100);
	func_label = lv_label_create(func_btn, NULL);
	lv_label_set_recolor(func_label, true);
	lv_label_set_text(func_label, "#FF0000 START#");
	lv_obj_set_event_cb(func_btn, event_handler);

	clear_btn = lv_btn_create(win, NULL);
	lv_obj_align(clear_btn, win, LV_ALIGN_CENTER, 50, -100);
	clear_label = lv_label_create(clear_btn, NULL);
	lv_label_set_text(clear_label, "CLEAR");
	lv_obj_set_event_cb(clear_btn, event_handler);

	if (task == NULL)
		task = lv_task_create(sec_task, 100, LV_TASK_PRIO_MID, NULL);
}
