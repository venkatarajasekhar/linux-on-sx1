/* ipc_dsy.c
*
* SX1 indication server
*
* Copyright 2007,2008 by Vladimir Ananiev (Vovan888 at gmail com )
*
* handles messages from modem. on channel 5 (AMUX::105)
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/select.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>

#include <ipc/tbus.h>
#include <ipc/shareddata.h>
#include <arch/sx1/ipc_dsy.h>
#include <flphone/debug.h>

// timeout in seconds
#define IPC_TIMEOUT 3
#define uint	unsigned int
#define MODEMDEVICE "/dev/mux4"
#define LSOCKET	"/tmp/sx_dsy"	// local socket to listen on
#define MAXMSG	104

static volatile int terminate = 0;
int _priority;

/* IPC interface */
struct SharedSystem *shdata;	/* shared memory segment */
static int ipc_fd;		/* IPC file descriptor */

static int fd_mux;		/* file descriptor for /dev/mux4, should be opened blocking */

//-----------------------------------------------------------------
static int dsy_init_serial(void)
{
	struct termios options;

	fd_mux = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if (fd_mux < 0) {
		ERRLOG("Error: open /dev/mux4 failed!");
		/*FIXME we should not exit on real device*/
		exit(-1);
	}
	/* set port settings */
	cfmakeraw(&options);
	options.c_iflag = IGNBRK;
	/* Enable the receiver and set local mode and 8N1 */
	options.c_cflag = (CLOCAL | CREAD | CS8 | HUPCL);
	options.c_cflag |= CRTSCTS;	// enable hardware flow control (CNEW_RTCCTS)
	options.c_cflag |= B38400;	// Set speed
	options.c_lflag = 0;	// set raw input
	options.c_oflag = 0;	// set raw output
	/* Set the new options for the port... */
	tcsetattr(fd_mux, TCSANOW, &options);
	return 0;
}

//-----------------------------------------------------------------
/* waits for reading length bytes from port. with timeout */
static int Read(unsigned char *frame, int length, int readtimeout)
{
	fd_set all_set, read_set;
	struct timeval timeoutz;
	int act_read = 0, retval;

	if ((frame == NULL) || (length <= 0))
		return -1;

	/* Initialize the timeout data structure. */
	timeoutz.tv_sec = readtimeout;
	timeoutz.tv_usec = 0;

	/* Initialize the file descriptor set. */
	FD_ZERO(&all_set);
	FD_SET(fd_mux, &all_set);

	do {
		read_set = all_set;
		if ((retval =
		     select(FD_SETSIZE, &read_set, NULL, NULL,
			    &timeoutz)) < 0) {
			DBGLOG("read select error = %d", errno);
			return -1;
		}

		if (retval == 0) {
			DBGLOG("read timeout!");
			return -1;
		}

		if (FD_ISSET(fd_mux, &read_set)) {
			do {
				retval =
				    read(fd_mux, frame + act_read,
					 length - act_read);
				if (retval >= 0)
					act_read += retval;
				else
					return -1;
			} while (act_read < length);
			return act_read;
		}
	} while (1);
}

//-----------------------------------------------------------------
/* write frame to serial port fd_mux */
static int Write(unsigned char *frame, int length)
{
	int written = 0, retval;
	if ((frame == NULL) || (length <= 0))
		return -1;
	do {
		retval = write(fd_mux, frame + written, length - written);
		if (retval > 0)
			written += retval;
		else
			return -1;
	} while (written < length);
	return written;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
// 5069C80C             ; IpcHelper::PutHeader
void PutHeader(unsigned char *buf, unsigned char IPC_Group, unsigned char cmd,
	       unsigned short length)
{
	buf[0] = IPC_DEST_ACCESS;
	buf[1] = IPC_Group;
	buf[2] = cmd;
	buf[3] = 0x30;
	*(unsigned short *)(buf + 4) = length - 6;
}

// CDsyIndicationHandler::HandleRagbag(TIpcMsgHdr *, TPtr8 &)
int HandleRagbag(unsigned char *buf, unsigned char *res)
{
	unsigned char cmd;
	unsigned char c1, c2;
	unsigned short data16;
	int data32;

	cmd = buf[2];
	switch (cmd) {
	case 0:		// CDsyIndicationHandler::NotifyAudioLinkOpenReq(TPtr8 &)
		// CBtAudioStateMachine::ModemAudioOpenReq(void);
		/*TODO*/
		break;
	case 1:		// CDsyIndicationHandler::NotifyAudioLinkCloseReq(TPtr8 &)
		// CBtAudioStateMachine::ModemAudioCloseReq(void)
		/*TODO*/
		break;
	case 5:		// CDsyIndicationHandler::SetLightSensorSettings(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);	// unused ???
		break;
	case 6:		// CDsyIndicationHandler::NotifyEmailMessage(TPtr8 &)
		data32 = *(unsigned char *)(buf + 6);
		// CDosEventManager::EmailMessage(int data32)
		/*TODO*/
		break;
	case 7:		//CDsyIndicationHandler::NotifyFaxMessage(TPtr8 &)
		data32 = *(unsigned char *)(buf + 6);
		//  CDosEventManager::FaxMessage(int data32)
		/*TODO*/
		break;
	case 8:		// CDsyIndicationHandler::NotifyVoiceMailStatus(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDosEventManager::VoiceMailStatus(TSAVoiceMailStatus)  data16
		/*TODO*/
		break;
	case 9:		// CDsyIndicationHandler::SetTempSensorValues(TPtr8 &)
		c1 = buf[6];	// unused ???
		c2 = buf[7];	// unused ???
		break;
	case 0x0B:		// CDsyIndicationHandler::NotifyCallsForwardingStatus(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDosEventManager::CallsForwardingStatus(TSACallsForwardingStatus)  data16
		/*TODO*/
		break;
	case 0x13:		// CDsyIndicationHandler::BacklightTempHandle(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// RBusLogicalChannel::DoControl(int, void *) 8, &data16
		/*TODO*/
		break;
	case 0x14:		// CDsyIndicationHandler::NotifyCarKitIgnition(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDsyFactory::CarKitIgnitionInd(unsigned short)  data16
		/*TODO*/
		break;
	default:
		return -1;
	}
	return 0;
}

// CDsyIndicationHandler::HandleSim(TIpcMsgHdr *, TPtr8 &)
int HandleSim(unsigned char *buf, unsigned char *res)
{
	unsigned char cmd;
	unsigned short data16;
	int data32;

	cmd = buf[2];
	switch (cmd) {
	case 0:		// CDsyIndicationHandler::NotifySimState(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDosEventManager::SimState(TDosSimState)  data16
		if (data16 == 7) {
			// CDsyFactory::SetSimChangedFlag(CDsyFactory::TSimChangedFlag, int)  0, 1
		}
		break;
	case 2:		// CDsyIndicationHandler::NotifySimChanged(TPtr8 &)
		data32 = *(unsigned char *)(buf + 6);
		// CDosEventManager::SimChanged(int)  data32
		break;
	case 3:		// CDsyIndicationHandler::NotifyCurrentOwnedSimStatus(TPtr8 &)
		data32 = *(unsigned char *)(buf + 6);
		// CDosEventManager::CurrentSimOwnedStatus(TSACurrentSimOwnedSimStatus) data32
		break;
	case 5:		// CDsyIndicationHandler::NotifySimLockStatus(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDosEventManager::SimLockStatus(TSASimLockStatus)  1
		break;
	default:
		return -1;
	}
	return 0;
}

// CDsyIndicationHandler::HandleBattery(TIpcMsgHdr *, TPtr8 &)
int HandleBattery(unsigned char *buf, unsigned char *res)
{
	unsigned char cmd;
	unsigned char c1;
	unsigned short data16;
	static int bars_done = 0;
	int bars;

	cmd = buf[2];
	switch (cmd) {
	case BAT_ChargingStateRes:	// CDsyIndicationHandler::NotifyChargingState(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDosEventManager::ChargingState(TDosChargingState)  data16
		int charging = (int)data16;
		shdata->Battery.Charging = charging;
		tbus_emit_signal("BatteryCharging","i", &charging);
		break;
	case BAT_StatusRes:	// CDsyIndicationHandler::NotifyBatteryStatus(TPtr8 &)
		data16 = *(unsigned short *)(buf + 6);
		// CDosEventManager::BatteryStatus(TDosBatteryStatus)  c1
		int status = (int)data16;
		shdata->Battery.Status = status;
		break;
	case BAT_BarsRes:	// CCDsyIndicationHandler::NotifyBatteryBars(TPtr8 &)
		bars = *(unsigned char *)(buf + 6);
		//CDosEventManager::BatteryBars(int)  c1 = 0..7
//		if (!bars_done) {
			shdata->Battery.ChargeLevel = (bars > 2)?bars - 2:0;
			tbus_emit_signal("BatteryChargeLevel","i", &bars);
			bars_done = 1;
//		}
		break;
	case BAT_LowWarningRes:	// CDsyIndicationHandler::NotifyBatteryLowWarning(TPtr8 &)
		c1 = *(unsigned char *)(buf + 6);
		// CDosEventManager::BatteryLowWarning(int)  c1
		shdata->Battery.Low = c1;
		tbus_emit_signal("BatteryLow","");
		break;
	default:
		return -1;
	}

	return 0;
}

// 50699AC0             ; CDsyIndicationHandler::HandleIndication(TIpcMsgHdr *, TPtr8 &, TPtr8 &)
// decodes incoming package and puts response into res
int HandleIndication(unsigned char *buf, unsigned char *res)
{
	unsigned char IPC_Group = buf[1];
	unsigned char cmd;
	unsigned char err = 0;

	if (IPC_Group > 10)
		return -2;	// error
	err = buf[3];
	if ((err != 0x30) && (err != 0xFF))
		return -1;

	cmd = buf[2];

	switch (IPC_Group) {
	case IPC_GROUP_RAGBAG:	// CDsyIndicationHandler::HandleRagbag(TIpcMsgHdr *, TPtr8 &)
		return HandleRagbag(buf, res);
	case IPC_GROUP_SIM:	// CDsyIndicationHandler::HandleSim(TIpcMsgHdr *, TPtr8 &)
		return HandleSim(buf, res);
	case IPC_GROUP_BAT:	// CDsyIndicationHandler::HandleBattery(TIpcMsgHdr *, TPtr8 &)
		return HandleBattery(buf, res);
	case IPC_GROUP_RSSI:	// CDsyIndicationHandler::HandleRssi(TIpcMsgHdr *, TPtr8 &)
		if (cmd == 0) {	// CDsyIndicationHandler::NotifyNetworkBars(TPtr8 &)
//			int netbars = *(unsigned short *)(buf + 6);
			// CDosEventManager::NetworkBars(int)  data16
			// we have this info from gsmd2
//			shdata->PhoneServer.Network_Signal = netbars;
//			tbus_emit_signal("NetworkBars", "i", &netbars);
			return 0;
		}
		return -1;
	case IPC_GROUP_POWERUP:	// CDsyIndicationHandler::HandlePowerup(TIpcMsgHdr *, TPtr8 &)
		if (cmd == 1) {	// CDsyIndicationHandler::SendPingResponse(void)
			return 0;
		}
		return -1;
		//  CDsyIndicationHandler::HandlePowerdown(TIpcMsgHdr *, TPtr8 &)
	case IPC_GROUP_POWEROFF:
		if (cmd == PWROFF_SEXITSHUTDOWN) {	// CDsyIndicationHandler::NotifyShutdown(void)
			 /*TODO*/
			    // many things...
			    // many things...
			    return 0;
		}
		return -1;
	default:
		return 0;
	}
}

//-----------------------------------------------------------------
/* processes messages from modem on serial port */
int process_modem(int fd)
{
	unsigned char buffer_in[MAXMSG];
	unsigned char buffer_out[MAXMSG];
	int err, group, cmd;
	int length;		// message length
	int len_read, len_write;

/*	DBGLOG("process_modem");*/
	/* read first 6 bytes of message (header) */
	len_read = Read(buffer_in, 6, IPC_TIMEOUT);
	if (len_read != 6)
		return -1;	// Error

	length = *(unsigned short *)(buffer_in + 4);
	if (length) {
		/* read the rest of data (if exists) */
		len_read = Read(buffer_in + 6, length, IPC_TIMEOUT);
		if (len_read != length)
			return -1;	// Error
	}
	group = buffer_in[1];
	cmd = buffer_in[2];
	DBGLOG("%02X %02X %02X", group, cmd, buffer_in[6]);

//	if (buffer_in[0] != IPC_DEST_DSY)
//		return -1;	/* Error - packet is not for us */

	/* Process IPC message from Egold */
	err = HandleIndication(buffer_in, buffer_out);

	/* send ACK response to modem */
	PutHeader(buffer_out, buffer_in[1], buffer_in[2], 6);
	length = 6;	// + *(unsigned short *)(buffer_out + 4);
	len_write = Write(buffer_out, length);
	if (len_write != length)
		return -1;

	return 0;
}

/*
 * Function responsible by all signal handlers treatment
 * any new signal must be added here
 */
void signal_treatment(int param)
{
	switch (param) {
	case SIGPIPE:
		exit(0);
		break;
	case SIGHUP:
		//reread the configuration files
		break;
	case SIGINT:
		//exit(0);
		terminate = 1;
		break;
	case SIGKILL:
		//kill immediatly
		//i'm not sure if i put exit or sustain the terminate attribution
		terminate = 1;
		//exit(0);
		break;
	case SIGUSR1:
		terminate = 1;
		//sig_term(param);
	case SIGTERM:
		terminate = 1;
		break;
	default:
		exit(0);
		break;
	}
}

/*-----------------------------------------------------------------*/
static int dsy_init(void)
{
	/* TODO handle errors here */
	dsy_init_serial();		// Init serial port

	ipc_fd = tbus_register_service("sx1_dsy");

	shdata = ShmMap(SHARED_SYSTEM);

	/* Subscribe to different signals here */
	return 0;
}

/*-----------------------------------------------------------------*/
/* Handle IPC message
 * This message only tells indicator that its value is changed
 * Actual value is stored in sharedmem
*/
static int ipc_handle(int fd)
{
	int ret;
	struct tbus_message msg;

	DBGLOG("");

	ret = tbus_get_message(&msg);
	if (ret < 0)
		return -1;
	switch(msg.type) {
		case TBUS_MSG_SIGNAL:
			/* we received a signal */
//			ipc_signal(&msg);
		case TBUS_MSG_CALL_METHOD:
			break;
	}

	tbus_msg_free(&msg);
	return 0;
}

//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
	fd_set active_fd_set, read_fd_set;
	struct timeval timeout;

#ifndef DEBUG
	daemon(0, 0);
#endif
	/* SIGNALS treatment */
	signal(SIGHUP, signal_treatment);
	signal(SIGPIPE, signal_treatment);
	signal(SIGKILL, signal_treatment);
	signal(SIGINT, signal_treatment);
	signal(SIGUSR1, signal_treatment);
	signal(SIGTERM, signal_treatment);

	INITSYSLOG(argv[0]);

	dsy_init();

//-----------------------------------------------------
	/* Initialize the timeout data structure. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	/* Initialize the file descriptor set. */
	FD_ZERO(&active_fd_set);
	FD_SET(fd_mux, &active_fd_set);
	FD_SET(ipc_fd, &active_fd_set);

	/*  ---  descriptor processing loop --- */
	while (!terminate) {
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			DBGLOG("select error = %d", errno);
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(fd_mux, &read_fd_set)) {
			/* Message from modem */
			if (process_modem(fd_mux)) {
				DBGLOG("error in process_modem");
			}
		}
		if (FD_ISSET(ipc_fd, &read_fd_set)) {
			/* message from IPC server */
			ipc_handle(ipc_fd);
		}
	}
	closelog();
	close(fd_mux);
	tbus_close();

	return 0;
}
