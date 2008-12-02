// generated by Fast Light User Interface Designer (fluid) version 1.0108

//#include <libintl.h>
#include "flgui/Fl_App.h"

//---------------------------------------------------------------------
/**
 * Default constructor
 * @param L window caption
 * @param leftsoft if true - LeftSoftMenu is used, false - LeftSoft
 * @param leftsoft if true - RightSoftMenu is used, false - RightSoft
 */

Fl_App::Fl_App(const char *L, bool leftsoft, bool rightsoft)
	:Fl_Window(0, APPVIEW_STATUS_HEIGHT, APPVIEW_WIDTH, APPVIEW_HEIGHT, L)
{
// widget constructor
	// Fl_Scroll* AppArea
	int normal_size = theme_fontsize(THEME_FONT_NORMAL);
	FL_NORMAL_SIZE = normal_size;
	labelsize(normal_size);

	AppArea = new Fl_Scroll(0, 0, APPVIEW_WIDTH, APPVIEW_AREA_HEIGHT);
	AppArea->user_data(this);
	AppArea->end();
	if (leftsoft) {
		LeftSoftMenu = new Fl_Menu_Button(0, APPVIEW_AREA_HEIGHT,
						  APPVIEW_WIDTH / 2, APPVIEW_CONTROL_HEIGHT,
						  "&Options");
		LeftSoftMenu->box(FL_FLAT_BOX);
		LeftSoft = NULL;
	} else {
		LeftSoft = new Fl_Button(0, APPVIEW_AREA_HEIGHT,
					 APPVIEW_WIDTH / 2, APPVIEW_CONTROL_HEIGHT, "OK");
		LeftSoftMenu = NULL;
		LeftSoft->shortcut(Key_LeftSoft);
		LeftSoft->box(FL_FLAT_BOX);
	}
	// Fl_Menu_Button* LeftSoft
	if (rightsoft) {
		RightSoftMenu = new Fl_Menu_Button(APPVIEW_WIDTH / 2, APPVIEW_AREA_HEIGHT,
						   APPVIEW_WIDTH / 2, APPVIEW_CONTROL_HEIGHT,
						   "&Close");
		RightSoftMenu->box(FL_FLAT_BOX);
		RightSoft = NULL;
	} else {
		RightSoft = new Fl_Button(APPVIEW_WIDTH / 2, APPVIEW_AREA_HEIGHT,
					  APPVIEW_WIDTH / 2, APPVIEW_CONTROL_HEIGHT, "&Close");
		RightSoftMenu = NULL;
		RightSoft->shortcut(Key_RightSoft);
		RightSoft->box(FL_FLAT_BOX);
	}
	// Fl_Menu_Button* RightSoft
	end();

	tbus_socket = tbus_register_service((char *)L);
	Fl::add_fd(tbus_socket, &handle_tbus_message, (void *)this);
}

Fl_App::~Fl_App()
{
	tbus_close();
	Fl::remove_fd(tbus_socket);
}

void Fl_App::handle_tbus_message(int fd, void *data)
{
	Fl_App *app = (Fl_App *) data;
	int ret;
	struct tbus_message msg;

	ret = tbus_get_message(&msg);
	if (ret < 0)
		return;
	switch (ret) {
	case TBUS_MSG_SIGNAL:
		app->handle_signal(&msg);
		break;
	case TBUS_MSG_CALL_METHOD:
		app->handle_method(&msg);
		break;
	case TBUS_MSG_RETURN_METHOD:
		app->handle_method_return(&msg);
		break;
	case TBUS_MSG_ERROR:
		app->handle_error(&msg);
		break;
	}

	tbus_msg_free(&msg);
	return;
}

#define WAIT_TIME_SLICE		500

int Fl_App::TBusWaitForMsg(struct tbus_message *msg, char *service, char *object, int timeout)
{
	int wait_result, type, tries, i = 0;

	tries = timeout / WAIT_TIME_SLICE;
	do {
		Fl::check();	/* let FLTK process events */
		wait_result = tbus_wait_message(WAIT_TIME_SLICE);
		if ((wait_result == 0) ||	/*timeout */
		    (wait_result == -1))	/*error */
			continue;

		type = tbus_get_message(msg);
		if (!strcmp(object, msg->object) && !strcmp(service, msg->service_sender)) {
			return 0;
		}
		tbus_msg_free(msg);
	} while (i++ < tries);

	return -1;		/* timeout or error */
}

void Fl_App::handle_signal(struct tbus_message *msg)
{
	return;
}
void Fl_App::handle_method(struct tbus_message *msg)
{
	return;
}
void Fl_App::handle_method_return(struct tbus_message *msg)
{
	return;
}
void Fl_App::handle_error(struct tbus_message *msg)
{
	return;
}
