#include "calendar_win.h"
#include "gui_app.h"

// ����,��һ,�ܶ�,...,����
const char *const day_names[7] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
// һ��,����,...,ʮ����
const char *const month_names[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
// ��Ҫ��������ʾ������
const lv_calendar_date_t highlihted_days[2] = {{2018, 11, 9}, {2018, 11, 13}};

static lv_obj_t *calendar1;
static lv_obj_t *roller1;

// �¼��ص�����
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (obj == calendar1)
	{
		if (event == LV_EVENT_CLICKED)
		{
			lv_calendar_date_t *selected_date = lv_calendar_get_pressed_date(obj); // ��ȡ�����µ�����
			if (selected_date && (selected_date->year != 0))
			{
				lv_calendar_set_today_date(obj, selected_date);	 // ���û�ѡ�����������Ϊ���������,��TODAY����
				lv_calendar_set_showed_date(obj, selected_date); // ��ת������������ڵĽ���
				// lv_roller_set_selected(roller1,selected_date->day-1,LV_ANIM_ON);//����Ҳ��Ӧ�ĸ���
			}
		}
	}
}

// ��������
void calendar_win_create(lv_obj_t *parent)
{
	lv_obj_t *win = general_win_create(parent, "Calendar");

	// ��������
	calendar1 = lv_calendar_create(win, NULL);
	// lv_obj_set_size(calendar1,220,220);//���ô�С
	lv_obj_set_size(calendar1, 320, 320); // ���ô�С

	lv_obj_align(calendar1, NULL, LV_ALIGN_IN_TOP_MID, 0, 100); // ���ö��뷽ʽ
	lv_obj_set_event_cb(calendar1, event_handler);				// �����¼��ص�����
	lv_calendar_date_t today = {2020, 11, 25};
	lv_calendar_set_today_date(calendar1, &today);						// ����TODAY����
	lv_calendar_set_showed_date(calendar1, &today);						// ��ת��TODAY�������ڵĽ���
	lv_calendar_set_day_names(calendar1, (const char **)day_names);		// ����������Ϣ�еı���
	lv_calendar_set_month_names(calendar1, (const char **)month_names); // �����·ݵı���
}
