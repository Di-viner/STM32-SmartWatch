#include "gauge_win.h"
#include "gui_app.h"
#include <stdio.h>
#include "MeterStepAlgorithm.h"

static lv_obj_t *gauge1;
static lv_obj_t *label1;
static lv_obj_t *lmeter1;

lv_color_t needle_colors[2]; // ÿһ��ָ�����ɫ
static lv_task_t *task = NULL;

extern sportsInfo_t SportsInfo;

// �¼��ص�����
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == USER_EVENT_BACK) // ���巵���¼�,������Ͻǵ�"< Back"��ť�ᴥ�����¼�
	{
		if (task) // ɾ��������
		{
			lv_task_del(task);
			task = NULL;
		}
	}
}

// ����ص�����
void task_cb(lv_task_t *task)
{
	char buff[40];

	// ����ָ�����ֵ
	lv_gauge_set_value(gauge1, 0, SportsInfo.walkspeed);
	// �Ѵ��ٶ���ʾ�ڱ�ǩ��,Ȼ����ݲ�ͬ����ֵ��ʾ����ͬ����ɫ
	// ��ɫ����ȫ,��ɫ������,��ɫ����Σ��
	sprintf(buff, "#%s %.2f m/s#", SportsInfo.walkspeed < 6 ? "00FF00" : (SportsInfo.walkspeed < 8 ? "FFFF00" : "FF0000"), SportsInfo.walkspeed);
	lv_label_set_text(label1, buff);
	// ���ÿ̶�ָʾ������ֵ
	lv_lmeter_set_value(lmeter1, SportsInfo.walkspeed);
}

// ��������
void gauge_win_create(lv_obj_t *parent)
{
	lv_obj_t *win;
	win = general_win_create(parent, "Speed");
	lv_obj_set_event_cb(win, event_handler); // �����¼��ص�����

	// �����Ǳ���
	gauge1 = lv_gauge_create(win, NULL);
	lv_obj_set_size(gauge1, 200, 200); // ���ô�С
	lv_gauge_set_range(gauge1, 0, 10); // �����Ǳ��̵ķ�Χ
	needle_colors[0] = LV_COLOR_BLUE;
	needle_colors[1] = LV_COLOR_PURPLE;
	lv_gauge_set_needle_count(gauge1, sizeof(needle_colors) / sizeof(needle_colors[0]), needle_colors);
	lv_gauge_set_value(gauge1, 0, SportsInfo.walkspeed);	// ����ָ��1ָ�����ֵ,���ǰ�ָ��1�����ٶ�ָ��
	lv_gauge_set_value(gauge1, 1, 6);						// ����ָ��2ָ�����ֵ,������ָ��ؼ���ֵ��
	lv_gauge_set_critical_value(gauge1, 6);					// ���ùؼ���ֵ��
	lv_gauge_set_scale(gauge1, 240, 31, 6);					// ���ýǶ�,�̶��ߵ�����,��ֵ��ǩ������
	lv_obj_align(gauge1, NULL, LV_ALIGN_IN_TOP_MID, 0, 40); // ���ö��뷽ʽ

	// ����һ����ǩ,������ʾ�Ǳ�����ָ��1����ֵ
	label1 = lv_label_create(win, NULL);
	lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);  // ���ó��ı�ģʽ
	lv_obj_set_width(label1, 80);						  // ���ÿ��
	lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);	  // �����ı����ж���
	lv_label_set_body_draw(label1, true);				  // ʹ�ܱ����ػ�
	lv_obj_align(label1, gauge1, LV_ALIGN_CENTER, 0, 60); // ���ö��뷽ʽ
	lv_label_set_text(label1, "0 m/s");					  // �����ı�
	lv_label_set_recolor(label1, true);					  // ʹ���ı��ػ�ɫ

	// ����һ���̶�ָʾ��
	lmeter1 = lv_lmeter_create(win, NULL);
	lv_obj_set_size(lmeter1, 150, 150);							   // ���ô�С
	lv_obj_align(lmeter1, gauge1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10); // ���ö��뷽ʽ
	lv_lmeter_set_range(lmeter1, 0, 10);						   // ���÷�Χ
	lv_lmeter_set_value(lmeter1, SportsInfo.walkspeed);			   // ���õ�ǰ��ֵ
	lv_lmeter_set_scale(lmeter1, 240, 31);						   // ���ýǶȺͿ̶��ߵ�����

	// ����һ��������ģ���ٶ�ָ��ı仯
	if (task == NULL)
		task = lv_task_create(task_cb, 600, LV_TASK_PRIO_MID, NULL);
}
