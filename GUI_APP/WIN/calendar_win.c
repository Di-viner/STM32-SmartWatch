#include "calendar_win.h"
#include "gui_app.h"

// 周日,周一,周二,...,周六
const char *const day_names[7] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
// 一月,二月,...,十二月
const char *const month_names[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
// 需要被高亮显示的日期
const lv_calendar_date_t highlihted_days[2] = {{2018, 11, 9}, {2018, 11, 13}};

static lv_obj_t *calendar1;
static lv_obj_t *roller1;

// 事件回调函数
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (obj == calendar1)
	{
		if (event == LV_EVENT_CLICKED)
		{
			lv_calendar_date_t *selected_date = lv_calendar_get_pressed_date(obj); // 获取被按下的日期
			if (selected_date && (selected_date->year != 0))
			{
				lv_calendar_set_today_date(obj, selected_date);	 // 将用户选择的日期设置为今天的日期,即TODAY日期
				lv_calendar_set_showed_date(obj, selected_date); // 跳转到这个日期所在的界面
				// lv_roller_set_selected(roller1,selected_date->day-1,LV_ANIM_ON);//滚轮也相应的更新
			}
		}
	}
}

// 创建窗体
void calendar_win_create(lv_obj_t *parent)
{
	lv_obj_t *win = general_win_create(parent, "Calendar");

	// 创建日期
	calendar1 = lv_calendar_create(win, NULL);
	// lv_obj_set_size(calendar1,220,220);//设置大小
	lv_obj_set_size(calendar1, 320, 320); // 设置大小

	lv_obj_align(calendar1, NULL, LV_ALIGN_IN_TOP_MID, 0, 100); // 设置对齐方式
	lv_obj_set_event_cb(calendar1, event_handler);				// 设置事件回调函数
	lv_calendar_date_t today = {2020, 11, 25};
	lv_calendar_set_today_date(calendar1, &today);						// 设置TODAY日期
	lv_calendar_set_showed_date(calendar1, &today);						// 跳转到TODAY日期所在的界面
	lv_calendar_set_day_names(calendar1, (const char **)day_names);		// 设置星期信息行的标题
	lv_calendar_set_month_names(calendar1, (const char **)month_names); // 设置月份的标题
}
