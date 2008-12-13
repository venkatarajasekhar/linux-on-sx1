// generated by Fast Light User Interface Designer (fluid) version 1.0108

//#include <libintl.h>
#include "flgui/Fl_App.h"

//---------------------------------------------------------------------
/**
 * Default constructor
 * @param L window caption
 */

Fl_App::Fl_App(const char *L)
	:Fl_Window(0, APPVIEW_STATUS_HEIGHT, APPVIEW_WIDTH, APPVIEW_HEIGHT, L)
{
// widget constructor
	int normal_size = theme_fontsize(THEME_FONT_NORMAL);
	FL_NORMAL_SIZE = normal_size;
	labelsize(normal_size);
	user_data(this);	/* parent of softkeys widgets */
	Fl_Window::box(FL_NO_BOX);

	AppArea = new Fl_Scroll(0, 0, APPVIEW_WIDTH, APPVIEW_AREA_HEIGHT);
	AppArea->user_data(this);	/* parent of all apparea widgets */
	AppArea->end();

	LeftSoftMenu = new Fl_SoftButton( true ,"Options");
	RightSoftMenu = new Fl_SoftButton( false, "Close");
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
