#include "login_win.h"
#include "gui_app.h"
#include "main_win.h"
#include <string.h>
#include <stdio.h>

// 变量定义
static lv_obj_t *login_win = NULL;

static lv_obj_t *time_label;
static lv_obj_t *day_label;
static lv_obj_t *lock_label;
static lv_obj_t *arc_hour;
static lv_obj_t *arc_min;
static lv_obj_t *arc_sec;
static lv_obj_t *arc_battery;
static lv_obj_t *arc_decorate_1;
static lv_obj_t *arc_decorate_2;

static lv_style_t arc_style_hour;
static lv_style_t arc_style_min;
static lv_style_t arc_style_sec;
static lv_style_t arc_style_battery;  // 一个圆弧表示电池
static lv_style_t arc_style_decorate; // 装饰圆环

static lv_style_t slider_bg_style;
static lv_style_t slider_indic_style;
static lv_style_t slider_knob_style;

static lv_style_t timelabel_style;
static lv_style_t locklabel_style;
static lv_task_t *task = NULL;

// 意义不明的东西
static lv_obj_t *battery_img;

// 时间是外部全局
extern uint8_t hour;
extern uint8_t min;
extern uint8_t sec;

LV_FONT_DECLARE(my_font_76);
LV_FONT_DECLARE(my_font);
LV_IMG_DECLARE(battery);

// 事件回调函数
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED || event == LV_EVENT_RELEASED)
	{
		if (lv_slider_get_value(obj) == 100)
		{
			lv_obj_del(login_win); // 删除整个登录窗体
			login_win = NULL;
			main_win_create(lv_scr_act()); // 跳转到主界面
			if (task)
			{
				lv_task_del(task);
				task = NULL;
			}
		}
		else
		{
			lv_slider_set_value(obj, 0, LV_ANIM_ON);
		}
	}
}

static void set_progress()
{
	char buff[10];
	if (arc_sec != NULL && arc_min != NULL && arc_hour != NULL)
	{
		lv_arc_set_angles(arc_sec, 0, 6 * sec);
		lv_arc_set_angles(arc_min, 0, 6 * min);
		lv_arc_set_angles(arc_hour, 0, 15 * hour);
	}

	sprintf(buff, "%d%d\n%d%d", hour / 10, hour % 10, min / 10, min % 10);
	// printf("%s\r\n", buff);
	if (time_label != NULL)
	{
		lv_label_set_text(time_label, buff);
		lv_obj_realign(time_label);
	}
}

static void progress_task(lv_task_t *t)
{
	set_progress();
}

// 创建登录窗体
void login_win_create(lv_obj_t *parent)
{
	uint16_t parent_width, parent_height;
	uint16_t arc_size = 300; // 最小圆环半径
	uint8_t arc_step = 50;	 // 圆环间距
	uint8_t lock_dis = 150;	 // 锁离圆环间距
	int8_t lock_label_y = -40;
	int8_t lock_label_x = -126;

	parent_width = lv_obj_get_width(parent);
	parent_height = lv_obj_get_height(parent);

	login_win = lv_cont_create(parent, NULL);
	lv_obj_set_size(login_win, parent_width, parent_height);
	lv_cont_set_layout(login_win, LV_LAYOUT_OFF);

	lv_style_copy(&arc_style_hour, &lv_style_plain);
	arc_style_hour.line.color = LV_COLOR_YELLOW;
	arc_style_hour.line.width = 15;
	arc_style_hour.line.rounded = 1;
	lv_style_copy(&arc_style_min, &arc_style_hour);
	lv_style_copy(&arc_style_sec, &arc_style_hour);
	lv_style_copy(&arc_style_battery, &arc_style_hour);
	lv_style_copy(&arc_style_decorate, &arc_style_hour);

	arc_style_min.line.color = LV_COLOR_ORANGE;
	arc_style_sec.line.color = LV_COLOR_RED;
	arc_style_decorate.line.color = LV_COLOR_BLUE;
	arc_style_battery.line.color = LV_COLOR_GREEN;
	arc_style_battery.line.width = 25;

	arc_battery = lv_arc_create(login_win, NULL);
	lv_arc_set_style(arc_battery, LV_ARC_STYLE_MAIN, &arc_style_battery);
	lv_obj_set_size(arc_battery, 700, 700);
	lv_arc_set_angles(arc_battery, 150, 210);
	lv_obj_align(arc_battery, NULL, LV_ALIGN_CENTER, 0, 0);

	battery_img = lv_img_create(login_win, NULL);
	lv_img_set_src(battery_img, &battery);
	lv_obj_align(battery_img, arc_battery, LV_ALIGN_IN_TOP_MID, 0, 50);

	arc_decorate_1 = lv_arc_create(login_win, NULL);
	lv_arc_set_style(arc_decorate_1, LV_ARC_STYLE_MAIN, &arc_style_decorate);
	lv_obj_set_size(arc_decorate_1, 450, 450);
	lv_obj_align(arc_decorate_1, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_arc_set_angles(arc_decorate_1, 60, 180 - 60);

	arc_decorate_2 = lv_arc_create(login_win, NULL);
	lv_arc_set_style(arc_decorate_2, LV_ARC_STYLE_MAIN, &arc_style_decorate);
	lv_obj_set_size(arc_decorate_2, 450, 450);
	lv_obj_align(arc_decorate_2, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_arc_set_angles(arc_decorate_2, 270 - 30, 270 + 30);

	arc_hour = lv_arc_create(login_win, NULL);
	lv_arc_set_style(arc_hour, LV_ARC_STYLE_MAIN, &arc_style_hour);
	lv_obj_set_size(arc_hour, arc_size, arc_size);
	lv_obj_align(arc_hour, NULL, LV_ALIGN_CENTER, 0, 0);

	arc_min = lv_arc_create(login_win, NULL);
	lv_arc_set_style(arc_min, LV_ARC_STYLE_MAIN, &arc_style_min);
	lv_obj_set_size(arc_min, arc_size + arc_step, arc_size + arc_step);
	lv_obj_align(arc_min, NULL, LV_ALIGN_CENTER, 0, 0);

	arc_sec = lv_arc_create(login_win, NULL);
	lv_arc_set_style(arc_sec, LV_ARC_STYLE_MAIN, &arc_style_sec);
	lv_obj_set_size(arc_sec, arc_size + 2 * arc_step, arc_size + 2 * arc_step);
	lv_obj_align(arc_sec, login_win, LV_ALIGN_CENTER, 0, 0);

	// 定义label
	time_label = lv_label_create(login_win, NULL);
	lv_style_copy(&timelabel_style, &lv_style_plain_color);
	timelabel_style.text.font = &my_font_76;
	lv_label_set_style(time_label, LV_LABEL_STYLE_MAIN, &timelabel_style);
	lv_obj_align(time_label, login_win, LV_ALIGN_CENTER, 0, 0);

	day_label = lv_label_create(login_win, NULL);
	lv_label_set_text(day_label, "2022/11/29 Tuesday"); // 修改成可设置
	lv_obj_align(day_label, arc_sec, LV_ALIGN_OUT_TOP_MID, 0, -20);

	lv_style_copy(&slider_bg_style, &lv_style_pretty);
	slider_bg_style.body.main_color = LV_COLOR_BLACK;
	slider_bg_style.body.grad_color = LV_COLOR_GRAY;
	slider_bg_style.body.radius = LV_RADIUS_CIRCLE;
	slider_bg_style.body.border.color = LV_COLOR_WHITE;

	lv_style_copy(&slider_indic_style, &lv_style_pretty_color);
	slider_indic_style.body.main_color = LV_COLOR_MAKE(0x5F, 0xB8, 0x78);
	slider_indic_style.body.grad_color = LV_COLOR_MAKE(0x5F, 0xB8, 0x78);
	slider_indic_style.body.radius = LV_RADIUS_CIRCLE;
	slider_indic_style.body.shadow.width = 8;
	slider_indic_style.body.shadow.color = slider_indic_style.body.main_color;
	slider_indic_style.body.padding.left = 3;
	slider_indic_style.body.padding.right = 3;
	slider_indic_style.body.padding.top = 3;
	slider_indic_style.body.padding.bottom = 3;

	lv_style_copy(&slider_knob_style, &lv_style_pretty);
	slider_knob_style.body.radius = LV_RADIUS_CIRCLE;
	slider_knob_style.body.opa = LV_OPA_70;

	lv_obj_t *lock = lv_slider_create(login_win, NULL);
	lv_obj_set_size(lock, 300, 25);
	lv_slider_set_range(lock, 0, 100);

	lv_slider_set_style(lock, LV_SLIDER_STYLE_BG, &slider_bg_style);
	lv_slider_set_style(lock, LV_SLIDER_STYLE_INDIC, &slider_indic_style);
	lv_slider_set_style(lock, LV_SLIDER_STYLE_KNOB, &slider_knob_style);
	lv_obj_align(lock, arc_sec, LV_ALIGN_OUT_BOTTOM_MID, 0, lock_dis);
	lv_obj_set_event_cb(lock, event_handler);

	lock_label = lv_label_create(login_win, NULL);
	lv_style_copy(&locklabel_style, &lv_style_plain_color);
	locklabel_style.text.font = &lv_font_roboto_28;
	lv_label_set_style(lock_label, LV_LABEL_STYLE_MAIN, &locklabel_style);
	lv_obj_align(lock_label, lock, LV_ALIGN_CENTER, lock_label_x, lock_label_y);
	lv_label_set_text(lock_label, ">> SLIDE TO UNLOCK >>");

	set_progress();

	if (task == NULL)
		task = lv_task_create(progress_task, 1000, LV_TASK_PRIO_MID, NULL); // 1s刷新一次
}
