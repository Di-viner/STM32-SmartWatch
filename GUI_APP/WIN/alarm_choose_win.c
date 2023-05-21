#include "alarm_choose_win.h"
#include "gui_app.h"
#include <stdio.h>
#include <string.h>

// 支持5个闹钟
static lv_obj_t *alarm_labels[5];
static lv_obj_t *win;

// 小窗口
static lv_obj_t *hour_roller, *min_roller;
static lv_obj_t *hour_set_label = NULL;
static lv_obj_t *min_set_label = NULL;
static lv_obj_t *delete_button = NULL;
static lv_obj_t *confirm_button = NULL;
static lv_obj_t *hide_button = NULL;
static lv_obj_t *confirm_button_label = NULL;
static lv_obj_t *delete_button_label = NULL;
static lv_obj_t *hide_button_label = NULL;
static lv_obj_t *mbox = NULL;
static char click_enable = 1;
static char current_alarm = 0;
static char isConfirm = 0;

uint8_t hour_array[5];
uint8_t min_array[5]; // 节约位数，min最后一位表示en使能位

static const char *const MBOX_MAP[] = {"Yes", "No", ""};

static void release_roller(void);
static void mbox_create(lv_obj_t *parent);

// 闹钟设置的事件回调函数
static void event_handler_set(lv_obj_t *obj, lv_event_t event)
{
	uint16_t btn_id;
	if (obj == hide_button)
	{
		if (event == LV_EVENT_RELEASED)
		{
			release_roller();
			click_enable = 1;
		}
	}
	else if (obj == confirm_button)
	{
		if (event == LV_EVENT_RELEASED)
		{
			isConfirm = 1;
			mbox_create(lv_obj_get_parent(obj));
		}
	}
	else if (obj == delete_button)
	{
		if (event == LV_EVENT_RELEASED)
		{
			isConfirm = 0;
			mbox_create(lv_obj_get_parent(obj));
		}
	}
	else if (obj == mbox)
	{
		if (event == LV_EVENT_VALUE_CHANGED)
		{
			btn_id = *((uint16_t *)lv_event_get_data()); // 获取退出对话框中的按钮id
			if (btn_id == 0)							 // 点击了Yes按钮
			{
				if (isConfirm)
				{
					// printf("confirm!\r\n");
					char buff_hour[4];
					char buff_min[4];
					char buff[40];
					lv_roller_get_selected_str(hour_roller, buff_hour, 3);
					hour_array[current_alarm] = 10 * (buff_hour[0] - '0') + buff_hour[1] - '0';
					lv_roller_get_selected_str(min_roller, buff_min, 3);
					min_array[current_alarm] = 2 * (10 * (buff_min[0] - '0') + buff_min[1] - '0') + 1;
					// printf("%d:%d\r\n",hour_array[current_alarm],min_array[current_alarm]);
					sprintf(buff, "#FFFFFF %c%c : %c%c#\n#666666 Alarm Clock %d#", buff_hour[0], buff_hour[1], buff_min[0], buff_min[1], current_alarm);
					lv_obj_t *item_label = lv_list_get_btn_label(alarm_labels[current_alarm]);
					lv_label_set_text(item_label, buff);
					lv_label_set_recolor(item_label, true);
					release_roller();
					click_enable = 1;
				}
				else
				{
					// printf("delete!\r\n");

					hour_array[current_alarm] = 0;
					min_array[current_alarm] = 0;

					char buff[40];
					sprintf(buff, "#FFFFFF Click to edit#\n#666666 Alarm Clock %d#", current_alarm);

					lv_obj_t *item_label = lv_list_get_btn_label(alarm_labels[current_alarm]);
					lv_label_set_text(item_label, buff);
					lv_label_set_recolor(item_label, true);

					release_roller();
					click_enable = 1;
				}
			}
			lv_obj_del(mbox); // 关闭退出对话框
			mbox = NULL;
		}
	}
}

// 创建退出确认的消息对话框
static void mbox_create(lv_obj_t *parent)
{
	if (mbox) // 不要重复创建
		return;
	mbox = lv_mbox_create(parent, NULL);
	lv_obj_set_size(mbox, lv_obj_get_width(parent) * 0.7f, lv_obj_get_height(parent) / 2);
	if (isConfirm)
		lv_mbox_set_text(mbox, "Tip\nAre you to sure to save your edit?");
	else
		lv_mbox_set_text(mbox, "Tip\nAre you to sure to delete?");
	lv_mbox_add_btns(mbox, (const char **)MBOX_MAP);
	lv_obj_set_drag(mbox, true); // 设置对话框可以被拖拽
	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(mbox, event_handler_set);
}

static void create_roller()
{

	// 创建滚轮 * 2
	min_roller = lv_roller_create(win, NULL);
	lv_roller_set_options(min_roller, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59", LV_ROLLER_MODE_INIFINITE);
	lv_roller_set_selected(min_roller, min_array[current_alarm] / 2, LV_ANIM_OFF); // 设置默认值
	lv_roller_set_fix_width(min_roller, 140);									   // 设置固定宽度
	lv_roller_set_visible_row_count(min_roller, 4);								   // 设置可见的行数
	lv_obj_align(min_roller, alarm_labels[4], LV_ALIGN_OUT_BOTTOM_MID, 100, 150);  // 设置对齐方式

	hour_roller = lv_roller_create(win, NULL);
	lv_roller_set_options(hour_roller, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", LV_ROLLER_MODE_INIFINITE);
	lv_roller_set_selected(hour_roller, hour_array[current_alarm], LV_ANIM_OFF);		 // 设置默认值
	lv_roller_set_fix_width(hour_roller, 140);											 // 设置固定宽度
	lv_obj_align(hour_roller, alarm_labels[4], LV_ALIGN_OUT_BOTTOM_MID, -100, 150 + 10); // 设置对齐方式,hour偏下

	// 创建标签 * 2
	hour_set_label = lv_label_create(win, NULL);
	lv_label_set_text(hour_set_label, "Set Hour");
	lv_obj_align(hour_set_label, hour_roller, LV_ALIGN_OUT_TOP_MID, 0, -35 - 10);

	min_set_label = lv_label_create(win, NULL);
	lv_label_set_text(min_set_label, "Set Min");
	lv_obj_align(min_set_label, min_roller, LV_ALIGN_OUT_TOP_MID, 0, -35);

	// 创建按钮 * 3
	confirm_button = lv_btn_create(win, NULL);
	lv_obj_set_width(confirm_button, 80);
	lv_obj_align(confirm_button, hour_roller, LV_ALIGN_OUT_BOTTOM_MID, 0, 30 + 10);
	confirm_button_label = lv_label_create(confirm_button, NULL);
	lv_label_set_text(confirm_button_label, "CONFIRM");
	lv_obj_set_event_cb(confirm_button, event_handler_set);

	delete_button = lv_btn_create(win, NULL);
	lv_obj_set_width(delete_button, 80);
	lv_obj_align(delete_button, min_roller, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	delete_button_label = lv_label_create(delete_button, NULL);
	lv_label_set_text(delete_button_label, "DELETE");
	lv_obj_set_event_cb(delete_button, event_handler_set);

	hide_button = lv_btn_create(win, NULL);
	lv_obj_set_width(hide_button, 80);
	lv_obj_align(hide_button, alarm_labels[4], LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	hide_button_label = lv_label_create(hide_button, NULL);
	lv_label_set_text(hide_button_label, "HIDE");
	lv_obj_set_event_cb(hide_button, event_handler_set);

	click_enable = 0;
}

static void release_roller()
{
	lv_obj_del(hour_roller);
	lv_obj_del(min_roller);
	lv_obj_del(hour_set_label);
	lv_obj_del(min_set_label);
	lv_obj_del(confirm_button);
	lv_obj_del(delete_button);
	lv_obj_del(hide_button);
}

// 闹钟窗口的事件回调函数
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == USER_EVENT_BACK) // 窗体返回事件,点击左上角的"< Back"按钮会触发此事件
	{
		return;
	}
	for (uint8_t i = 0; i < 5; i++)
	{
		if (obj == alarm_labels[i] && event == LV_EVENT_CLICKED && click_enable == 1)
		{
			current_alarm = i;
			create_roller();
		}
	}
}

// 创建窗体
void alarm_choose_win_create(lv_obj_t *parent)
{
	click_enable = 1;
	win = general_win_create(parent, "Alarm Clock");
	lv_obj_set_event_cb(win, event_handler); // 设置事件回调函数

	// 创建列表
	lv_obj_t *list = lv_list_create(win, NULL);
	lv_obj_set_size(list, lv_obj_get_width(parent) * 0.9f, lv_obj_get_height(parent) - 60);
	lv_obj_align(list, win, LV_ALIGN_IN_TOP_MID, 0, 50);
	lv_list_set_sb_mode(list, LV_SB_MODE_AUTO);

	// 添加5个表项
	for (char i = 0; i < 5; i++)
	{

		if (min_array[i] % 2 == 0)
		{
			char buff[40];
			sprintf(buff, "#FFFFFF Click to edit#\n#666666 Alarm Clock %d#", i);
			alarm_labels[i] = lv_list_add_btn(list, NULL, buff); // 可以考虑上图?
		}
		else
		{
			char buff[40];
			sprintf(buff, "#FFFFFF %d%d : %d%d#\n#666666 Alarm Clock %d#", hour_array[i] / 10, hour_array[i] % 10, min_array[i] / 20, (min_array[i] / 2) % 10, i);
			alarm_labels[i] = lv_list_add_btn(list, NULL, buff);
		}

		lv_obj_t *item_label = lv_list_get_btn_label(alarm_labels[i]);
		lv_label_set_recolor(item_label, true);
		lv_obj_set_event_cb(alarm_labels[i], event_handler); // 设置事件回调函数
	}
}
