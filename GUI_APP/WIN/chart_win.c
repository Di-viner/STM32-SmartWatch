#include "chart_win.h"
#include "gui_app.h"
#include <stdio.h>
// �����Ѿ����ĳ�Temperature��

#define POINT_COUNT 10	   // �����������е����ݵ����
#define SLIDER_MAX_VAL 100 // ��������ֵ
lv_coord_t series1_y[POINT_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
lv_coord_t series2_y[POINT_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static lv_obj_t *chart1;
static lv_obj_t *chart2;
static lv_chart_series_t *series1; // �¶�
static lv_chart_series_t *series2; // ����
static lv_obj_t *slider1;
static lv_obj_t *sw1;
static lv_obj_t *cb1;
static lv_obj_t *temp_label;
static lv_obj_t *light_label;
static lv_task_t *task = NULL;

extern u8 light_val;
extern u16 temp_val;

// �¼��ص�����
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
	float val;
	uint8_t i;
	bool sta;
	if (event == USER_EVENT_BACK) // ���巵���¼�,������Ͻǵ�"< Back"��ť�ᴥ�����¼�
	{
		// printf("quit\r\n");
		if (task) // ɾ��������
		{
			lv_task_del(task);
			task = NULL;
		}
	}
	else if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (obj == slider1) // ����,�����ı������ߵ�����
		{
			val = lv_slider_get_value(obj); // ��ȡ����ĵ�ǰֵ
			val /= SLIDER_MAX_VAL;			// �õ�����ϵ��
			for (i = 0; i < POINT_COUNT; i++)
			{
				series1->points[i] = (lv_coord_t)(val * series1_y[i]); // ��������ϵ��,�޸����ݵ��ֵ
				series2->points[i] = (lv_coord_t)(val * series2_y[i]);
			}
			lv_chart_refresh(chart1); // ����ͼ��
			lv_chart_refresh(chart2);
		}
		else if (obj == sw1) // ����,����ѡ��ͼ�������
		{
			sta = lv_sw_get_state(obj);
			if (sta)
			{
				lv_chart_set_type(chart1, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE); // ����Ϊɢ������ߵ����
				lv_chart_set_type(chart2, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);
			}
			else
			{
				lv_chart_set_type(chart1, LV_CHART_TYPE_COLUMN); // ��״ͼ
				lv_chart_set_type(chart2, LV_CHART_TYPE_COLUMN);
			}
		}
		else if (obj == cb1) // ��ѡ��,Ҳ������ѡ��ͼ�������
		{
			sta = lv_cb_is_checked(obj);
			if (sta)
			{
				lv_chart_set_type(chart1, LV_CHART_TYPE_AREA); // ���ͼ
				lv_chart_set_type(chart2, LV_CHART_TYPE_AREA);
			}
			else
			{
				lv_chart_set_type(chart1, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE); // ����Ϊɢ������ߵ����
				lv_chart_set_type(chart2, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);
			}
		}
	}
}

static void updatechart()
{
	lv_chart_set_next(chart1, series1, temp_val / 100);
	lv_chart_set_next(chart2, series2, light_val);
}

static void chart_task(lv_task_t *t)
{
	char buff[30];
	sprintf(buff, "CURRENT TEMP: %d.%d C", temp_val / 100, temp_val % 100);
	if (temp_label != NULL)
		lv_label_set_text(temp_label, buff);

	sprintf(buff, "LIGHT: %d", light_val);
	if (light_label != NULL)
		lv_label_set_text(light_label, buff);

	// ����ͼ��
	updatechart();
}

// ��������
void chart_win_create(lv_obj_t *parent)
{
	lv_obj_t *win;
	win = general_win_create(parent, "Temperature");
	lv_obj_set_event_cb(win, event_handler);

	// ����ͼ��1
	chart1 = lv_chart_create(win, NULL);
	lv_obj_set_size(chart1, 180, 200);									 // ���ô�С
	lv_obj_align(chart1, NULL, LV_ALIGN_IN_TOP_MID, 17, 40);			 // ���ö��뷽ʽ
	lv_chart_set_type(chart1, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE); // ����Ϊɢ������ߵ����
	lv_chart_set_series_opa(chart1, LV_OPA_80);							 // ���������ߵ�͸����
	lv_chart_set_series_width(chart1, 4);								 // ���������ߵĿ��
	lv_chart_set_series_darking(chart1, LV_OPA_80);						 // ���������ߵĺ���ӰЧ��
	lv_chart_set_point_count(chart1, POINT_COUNT);						 // �������ݵ�ĸ���
	lv_chart_set_div_line_count(chart1, 4, 4);							 // ����ˮƽ�ʹ�ֱ�ָ���
	lv_chart_set_range(chart1, 0, 100);									 // ����y�����ֵ��Χ
	lv_chart_set_y_tick_length(chart1, 10, 3);
	lv_chart_set_y_tick_texts(chart1, "100\n80\n60\n40\n20\n0", 2, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_x_tick_length(chart1, 10, 3);
	lv_chart_set_x_tick_texts(chart1, "0\n2\n4\n6\n8\n10", 2, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_margin(chart1, 40);
	lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);
	// ��ͼ�������������
	series1 = lv_chart_add_series(chart1, LV_COLOR_RED);		   // ָ��Ϊ��ɫ
	lv_chart_set_points(chart1, series1, (lv_coord_t *)series1_y); // ��ʼ�����ݵ��ֵ

	// ��������
	slider1 = lv_slider_create(win, NULL);
	lv_obj_set_size(slider1, 200, 20);				 // ���ô�С
	lv_slider_set_range(slider1, 0, SLIDER_MAX_VAL); // ���÷�Χ
	lv_slider_set_value(slider1, SLIDER_MAX_VAL, LV_ANIM_OFF);
	lv_obj_align(slider1, chart1, LV_ALIGN_OUT_BOTTOM_MID, -17, 45); // ���ö��뷽ʽ
	lv_obj_set_event_cb(slider1, event_handler);					 // �����¼��ص�����

	// ������ѡ��
	cb1 = lv_cb_create(win, NULL);
	lv_cb_set_text(cb1, "Area type");							// �����ı�
	lv_cb_set_checked(cb1, false);								// ���ø�ѡ��Ĭ��û�б�ѡ��
	lv_obj_align(cb1, slider1, LV_ALIGN_OUT_BOTTOM_MID, 0, 15); // ���ö��뷽ʽ
	lv_obj_set_event_cb(cb1, event_handler);					// �����¼��ص�����

	// ��������
	sw1 = lv_sw_create(win, NULL);
	lv_obj_set_size(sw1, 80, 40);							// ���ô�С
	lv_obj_align(sw1, cb1, LV_ALIGN_OUT_BOTTOM_MID, 0, 15); // ���ö��뷽ʽ
	lv_sw_on(sw1, LV_ANIM_ON);								// Ĭ��Ϊ��״̬
	lv_obj_set_event_cb(sw1, event_handler);				// �����¼��ص�����

	temp_label = lv_label_create(win, NULL);
	lv_obj_align(temp_label, sw1, LV_ALIGN_OUT_BOTTOM_MID, -30, 15);
	light_label = lv_label_create(win, NULL);
	lv_obj_align(light_label, sw1, LV_ALIGN_OUT_BOTTOM_MID, -30, 35);

	chart2 = lv_chart_create(win, NULL);
	lv_obj_set_size(chart2, 180, 200);									 // ���ô�С
	lv_obj_align(chart2, NULL, LV_ALIGN_IN_TOP_MID, 17, 40);			 // ���ö��뷽ʽ
	lv_chart_set_type(chart2, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE); // ����Ϊɢ������ߵ����
	lv_chart_set_series_opa(chart2, LV_OPA_80);							 // ���������ߵ�͸����
	lv_chart_set_series_width(chart2, 4);								 // ���������ߵĿ��
	lv_chart_set_series_darking(chart2, LV_OPA_80);						 // ���������ߵĺ���ӰЧ��
	lv_chart_set_point_count(chart2, POINT_COUNT);						 // �������ݵ�ĸ���
	lv_chart_set_div_line_count(chart2, 4, 4);							 // ����ˮƽ�ʹ�ֱ�ָ���
	lv_chart_set_range(chart2, 0, 100);									 // ����y�����ֵ��Χ
	lv_chart_set_y_tick_length(chart2, 10, 3);
	lv_chart_set_y_tick_texts(chart2, "100\n80\n60\n40\n20\n0", 2, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_x_tick_length(chart2, 10, 3);
	lv_chart_set_x_tick_texts(chart2, "0\n2\n4\n6\n8\n10", 2, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_margin(chart2, 40);
	lv_chart_set_update_mode(chart2, LV_CHART_UPDATE_MODE_SHIFT);
	// ��ͼ�������������
	series2 = lv_chart_add_series(chart2, LV_COLOR_GREEN);		   // ָ��Ϊ��ɫ
	lv_chart_set_points(chart2, series2, (lv_coord_t *)series2_y); // ��ʼ�����ݵ��ֵ
	lv_obj_align(chart2, win, LV_ALIGN_CENTER, 25, 200);

	if (task == NULL)
		task = lv_task_create(chart_task, 500, LV_TASK_PRIO_MID, NULL); // 0.5sˢ��һ��
}
