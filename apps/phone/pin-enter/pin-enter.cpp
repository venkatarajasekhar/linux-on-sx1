// generated by Fast Light User Interface Designer (fluid) version 1.0107

#include "pin-enter.h"
//#include <FL/fl_ask.H>

#include <stdlib.h>

UserInterface ui;

//-----------------------------------------------------------------------------
static const char *pin_type_names[__NUM_GSMD_PIN] = {
	"READY", "SIM PIN", "SIM PUK", "Phone-to-SIM PIN",
	"Phone-to-very-first SIM PIN", "Phone-to-very-first SIM PUK",
	"SIM PIN2", "SIM PUK2", "Network personalization PIN",
	"Network personalizaiton PUK",
	"Network subset personalisation PIN",
	"Network subset personalisation PUK",
	"Service provider personalisation PIN",
	"Service provider personalisation PUK",
	"Corporate personalisation PIN",
	"Corporate personalisation PUK",
};

//-----------------------------------------------------------------------------
const char *lgsm_pin_name(int ptype)
{
	if (ptype >= __NUM_GSMD_PIN)
		return "unknown";

	return pin_type_names[ptype];
}

//-----------------------------------------------------------------------------
void UserInterface::cb_pin_code_i(Fl_Input* o, void* v) {
  //pin code changed;
//	msg->value("pin changed");
}
//-----------------------------------------------------------------------------
void UserInterface::cb_pin_code(Fl_Input* o, void* v) {
  ((UserInterface*)v)->cb_pin_code_i(o,v);
}

//-----------------------------------------------------------------------------
void UserInterface::cb_pin_ok_timer(void *v)
{
	exit(1);
}

//-----------------------------------------------------------------------------
void UserInterface::cb_wait_animation(void *data)
{

}

//-----------------------------------------------------------------------------
void UserInterface::handle_method_return(struct tbus_message *msg)
{
	if(!strcmp("PhoneServer", msg->service_sender))
	if(!strcmp("PIN/Input", msg->object)) {
		int ret, result = -1;
		ret = tbus_get_message_args(msg, "i", &result);
		if(ret < 0)
			return;
		if(result == 0) {
			message->value("PIN OK");
			Fl::add_timeout(1.0, cb_pin_ok_timer, (void *)this);
//			usleep(200000);	// 0.2 sec
//			exit(1);
		} else {
			message->value("PIN Error!");
//			message->value(lgsm_pin_name(GetPINType()));
			pin_code->value("");
//			tries left = ...
		}
	}
}

//-----------------------------------------------------------------------------
void UserInterface::cb_LeftSoft_i(Fl_Button*, void*) {
  // OK button;
//	message->value("OK");
	int ret;
	char *oldpin = (char *)pin_code->value();
	char *newpin = "";

	if(strlen(oldpin) < 4)
		return;

	ret = tbus_call_method("PhoneServer", "PIN/Input", "ss",
			&oldpin, &newpin);
//	Fl::add_timeout(1.0, cb_wait_animation);
}

//-----------------------------------------------------------------------------
void UserInterface::cb_LeftSoft(Fl_Button* o, void* v) {
  ((UserInterface*)v)->cb_LeftSoft_i(o,v);
}

//-----------------------------------------------------------------------------
void UserInterface::cb_RightSoft_i(Fl_Button*, void*) {
  // Cancel button;
	message->value("Cancel");
//	exit(1);
	// Tell others to shutdown system!!!
}
//-----------------------------------------------------------------------------
void UserInterface::cb_RightSoft(Fl_Button* o, void* v) {
  ((UserInterface*)v)->cb_RightSoft_i(o,v);
}
#include <FL/fl_ask.H>

//-----------------------------------------------------------------------------
UserInterface::UserInterface()
	: Fl_App("PIN-code"){

    { Fl_Output* o = message = new Fl_Output(0, 103, 176, 25);
      o->box(FL_FLAT_BOX);
      o->color((Fl_Color)48);
      AppArea -> add(o);
    }
    {
      Fl_Secret_Input* o = pin_code = new Fl_Secret_Input(0, 127, 176, 34);

      o->box(FL_FLAT_BOX);
      o->color((Fl_Color)19);
      o->textsize(18);
      o->callback((Fl_Callback*)cb_pin_code, (void *)this);
      o->when(FL_WHEN_CHANGED);
      AppArea -> add(o);
    }
//	LeftSoft->labeltype(FL_SHADOW_LABEL);
	LeftSoft->callback((Fl_Callback*)cb_LeftSoft, (void *)this);

//	RightSoft->labeltype(FL_SHADOW_LABEL);
	RightSoft->callback((Fl_Callback*)cb_RightSoft, (void *)this);

	message->value("Please enter PIN");

	pin_code->activate();

	PinOK = false;
}

//-----------------------------------------------------------------------------
int UserInterface::EnterPIN(char *oldpin, char *newpin)
{
	int ret;
	struct tbus_message tmsg;

	ret = tbus_call_method_and_wait(&tmsg, "PhoneServer", "PIN/Input", "ss",
			&oldpin, &newpin);
	if(ret < 0)
		return ret;

	int result = -1;
	ret = tbus_get_message_args(&tmsg, "i", &result);

	return result;
}

//-----------------------------------------------------------------------------
int UserInterface::GetPINType()
{
	int ret, counter = 0, type = -1;
	struct tbus_message tmsg;

	do {
		ret = tbus_call_method_and_wait(&tmsg, "PhoneServer", "PIN/GetStatus", "");
		if(ret < 0)
			continue;
		ret = tbus_get_message_args(&tmsg, "i", &type);
		if(ret < 0) {
			// do something ???
			usleep(1000);
			continue;
		}
		tbus_msg_free(&tmsg);
		break;
	} while(counter++ < 5);

	return type;
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int type;
// create GUI to PIN input
// ask phoneserver if some PIN is needed
	type = ui.GetPINType();
	if(type == GSMD_PIN_READY)
		return 0;
// enter PIN
	ui.message->value(lgsm_pin_name(type));
	ui.show(argc,argv);

	return Fl::run();
}
//-----------------------------------------------------------------------------
