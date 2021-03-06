// generated by Fast Light User Interface Designer (fluid) version 1.0107

#ifndef phone__h
#define phone__h
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>

#include <flgui/Fl_App.h>
#include <gsmd/event.h>

#include <flphone/debug.h>

#define TIMEOUT_QNETWORK	2.0
#define TIMEOUT_END_CALL	3.0

#define STATE_IDLE		0
#define STATE_INCOMING		1
#define STATE_DIALING		2
#define STATE_CALL_ACTIVE	3
#define STATE_END_CALL		4
#define STATE_HOLD		5

class PhoneApp:public Fl_App {
      public:
	void RemoveIllegalChars(char *str, int len);
	char *PhoneNumber;
	void GotoState(int newAppState);
	void GreenButtonPressed();
	void RedButtonPressed();
	void ConnectSignals();
	int RegisteredToNetwork();
	static void cb_timeout_qregister(void *);
	void SetOperatorName();
	static void cb_timeout_end_call(void *);

	struct SharedSystem *Shm;

	 PhoneApp();
	~PhoneApp();
	Fl_Group *grp_dial;
	Fl_Input *NumberInput;
      private:
	void cb_NumberInput_i(Fl_Input *, void *);
	static void cb_NumberInput(Fl_Input *, void *);

	virtual void handle_signal(struct tbus_message *msg);
	virtual void handle_method_return(struct tbus_message *msg);

      public:
	 Fl_Text_Display * search;
      private:
	void cb_search_i(Fl_Text_Display *, void *);
	static void cb_search(Fl_Text_Display *, void *);
      public:
	 Fl_Group * grp_call;
	Fl_Output *OpName;
	Fl_Output *CallType;
	Fl_Output *ContactName;
	Fl_Output *Number;
	Fl_Button *btn_end_call;
      private:
	void cb_btn_end_call_i(Fl_Button *, void *);
	static void cb_btn_end_call(Fl_Button *, void *);
      public:
	static Fl_Menu_Item menu_LeftSoft[];
      private:
	void cb_Add_i(Fl_Menu_ *, void *);
	static void cb_Add(Fl_Menu_ *, void *);
	void cb_Call_i(Fl_Menu_ *, void *);
	static void cb_Call(Fl_Menu_ *, void *);
	void cb_Copy_i(Fl_Menu_ *, void *);
	static void cb_Copy(Fl_Menu_ *, void *);
	void cb_Speaker_i(Fl_Menu_ *, void *);
	static void cb_Speaker(Fl_Menu_ *, void *);
	void cb_Hold_i(Fl_Menu_ *, void *);
	static void cb_Hold(Fl_Menu_ *, void *);
	void cb_Disable_i(Fl_Menu_ *, void *);
	static void cb_Disable(Fl_Menu_ *, void *);
	void cb_Send_i(Fl_Menu_ *, void *);
	static void cb_Send(Fl_Menu_ *, void *);
	void cb_End_i(Fl_Menu_ *, void *);
	static void cb_End(Fl_Menu_ *, void *);
      private:
	void cb_RightSoft_i(Fl_Menu_Button *, void *);
	static void cb_RightSoft(Fl_Menu_Button *, void *);
};
#endif
