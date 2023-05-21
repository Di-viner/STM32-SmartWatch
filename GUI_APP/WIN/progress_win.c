#include "progress_win.h"
#include "gui_app.h"
#include "MeterStepAlgorithm.h"
#include <stdio.h>

static lv_obj_t *bg_arc;
static lv_obj_t *step_arc;

static lv_obj_t *step_label;
static lv_obj_t *dis_label;
static lv_obj_t *calory_label;
static lv_obj_t *longsit_label;

static lv_task_t *task = NULL;

static lv_style_t my_arc_style;

uint8_t progress_val;

extern sportsInfo_t SportsInfo;

// �¼��ص�����
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == USER_EVENT_BACK) // ���巵���¼�,������Ͻǵ�"< Back"��ť�ᴥ�����¼�
	{
		printf("quit\r\n");
		if (task) // ɾ��������
		{
			lv_task_del(task);
			task = NULL;
		}
	}
}

// ����ص�����
static void progress_task(lv_task_t *t)
{
	u8 steps = SportsInfo.stepCount;
	char buff[30];
	sprintf(buff, "STEP\n%c%c%c%c", steps / 1000 + '0', (steps % 1000) / 100 + '0', (steps % 100) / 10 + '0', (steps % 10) + '0');
	lv_arc_set_angles(step_arc, 0, SportsInfo.stepCount);
	lv_label_set_text(step_label, buff);
	sprintf(buff, "Total Distance: %.2f m", SportsInfo.distance);
	lv_label_set_text(dis_label, buff);
	sprintf(buff, "Total Calory Consuption: %.2f c", SportsInfo.calories);
	lv_label_set_text(calory_label, buff);
}

// ��������
void progress_win_create(lv_obj_t *parent)
{
	lv_obj_t *win = general_win_create(parent, "Status");
	lv_obj_set_event_cb(win, event_handler); // �����¼��ص�����

	// ����Ԥ���ض���
	lv_obj_t *preload1 = lv_preload_create(win, NULL);
	lv_obj_set_size(preload1, 250, 250);						 // ���ô�С
	lv_obj_align(preload1, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);	 // ���ö��뷽ʽ
	lv_preload_set_arc_length(preload1, 45);					 // ����СԲ����Ӧ�ĽǶ�
	lv_preload_set_dir(preload1, LV_PRELOAD_DIR_BACKWARD);		 // ����Ϊ��ʱ��
	lv_preload_set_type(preload1, LV_PRELOAD_TYPE_FILLSPIN_ARC); // ������ת�����ķ�ʽ
	lv_preload_set_spin_time(preload1, 2000);					 // ������ת���ٶ�,תһȦ����Ҫ��ʱ��

	// �������α���
	lv_style_copy(&my_arc_style, &lv_style_plain);
	my_arc_style.line.color = LV_COLOR_WHITE;
	bg_arc = lv_arc_create(win, NULL);
	lv_arc_set_style(bg_arc, LV_ARC_STYLE_MAIN, &my_arc_style);
	lv_arc_set_angles(bg_arc, 0, 360); // ���ýǶ�
	lv_obj_set_size(bg_arc, 200, 200); // ���ô�С,���õĿ�Ⱥ͸߶ȱ������,���ΰ뾶���ڿ�ȵ�һ��
	lv_obj_align(bg_arc, preload1, LV_ALIGN_CENTER, 0, 0);

	step_arc = lv_arc_create(win, NULL);
	lv_arc_set_angles(step_arc, 0, 0);	 // ���ýǶ�
	lv_obj_set_size(step_arc, 200, 200); // ���ô�С,���õĿ�Ⱥ͸߶ȱ������,���ΰ뾶���ڿ�ȵ�һ��
	lv_obj_align(step_arc, preload1, LV_ALIGN_CENTER, 0, 0);

	step_label = lv_label_create(win, NULL);
	lv_obj_align(step_label, step_arc, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(step_label, "STEP\n0000");

	dis_label = lv_label_create(win, NULL);
	lv_obj_align(dis_label, win, LV_ALIGN_CENTER, -75, -30);
	lv_label_set_text(dis_label, "Total Distance:\n0 m");

	calory_label = lv_label_create(win, NULL);
	lv_obj_align(calory_label, win, LV_ALIGN_CENTER, -75, 0);
	lv_label_set_text(calory_label, "Total Calory Consuption:\n0 c");

	longsit_label = lv_label_create(win, NULL);
	lv_obj_align(longsit_label, win, LV_ALIGN_CENTER, -75, 30);
	lv_label_set_text(longsit_label, "Long Sit Count: 0 time");

	if (task == NULL)
		task = lv_task_create(progress_task, 100, LV_TASK_PRIO_MID, NULL);
}
