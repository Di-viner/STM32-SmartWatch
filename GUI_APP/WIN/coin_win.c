#include "coin_win.h"
#include "gui_app.h"
#include "delay.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
static lv_obj_t *img;
static char coin_state;
static char is_flipping;
static lv_obj_t *result_label;
static lv_obj_t *start_label;
static lv_obj_t *start_btn;
static lv_task_t *task = NULL;
static u8 count;
extern uint8_t hour;
extern uint8_t min;
extern uint8_t sec;

LV_IMG_DECLARE(coin1);
LV_IMG_DECLARE(coin2);
LV_IMG_DECLARE(coin3)

static void flip_coin(void);

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == USER_EVENT_BACK) // 窗体返回事件,点击左上角的"< Back"按钮会触发此事件
	{
		// printf("quit\r\n");
		if (task) // 删除掉任务
		{
			lv_task_del(task);
			task = NULL;
		}
	}
	else
	{
		if (obj == start_btn)
		{
			if (event == LV_EVENT_RELEASED && is_flipping == 0)
			{
				flip_coin();
			}
		}
	}
}

static void coin_task(lv_task_t *t)
{
	if (count > 0 && is_flipping == 1)
	{
		coin_state = (coin_state + 1) % 3;
		if (coin_state == 0)
		{
			lv_img_set_src(img, &coin1);
		}
		else if (coin_state == 1)
		{
			lv_img_set_src(img, &coin2);
		}
		else
		{
			lv_img_set_src(img, &coin3);
		}
		lv_obj_realign(img);

		count--;

		if (count == 0)
		{
			if (coin_state == 0)
				lv_label_set_text(result_label, "THE COIN IS ON #FF0000 HEADS#");
			else if (coin_state == 2)
				lv_label_set_text(result_label, "THE COIN IS ON #FF0000 TAILS#");
			is_flipping = 0;
		}
	}
}

static void flip_coin()
{
	u8 rand = (hour + min + sec) % 2;
	count = 30;
	if (rand == 0)
		count = count + 2;
	printf("%d\r\n", count);

	coin_state = 0; // 把硬币摆正
	lv_img_set_src(img, &coin1);
	lv_obj_realign(img);

	is_flipping = 1;
}

void coin_win_create(lv_obj_t *parent)
{
	coin_state = 0;
	is_flipping = 0;
	count = 0;
	lv_obj_t *win = general_win_create(parent, "Flip a Coin !");
	lv_obj_set_event_cb(win, event_handler); // 设置事件回调函数

	img = lv_img_create(win, NULL);
	lv_img_set_src(img, &coin1);
	lv_obj_align(img, win, LV_ALIGN_CENTER, 0, -150);

	start_btn = lv_btn_create(win, NULL);
	lv_obj_align(start_btn, win, LV_ALIGN_CENTER, 0, 0);
	start_label = lv_label_create(start_btn, NULL);
	lv_label_set_recolor(start_label, true);
	lv_label_set_text(start_label, "#FF0000 START#");
	lv_obj_set_event_cb(start_btn, event_handler);

	result_label = lv_label_create(win, NULL);
	lv_label_set_recolor(result_label, true);
	lv_label_set_text(result_label, "WAIT TO START");
	lv_obj_align(result_label, win, LV_ALIGN_CENTER, 0, 80);

	if (task == NULL)
		task = lv_task_create(coin_task, 100, LV_TASK_PRIO_MID, NULL);
}
