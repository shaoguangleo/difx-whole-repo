//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id$
// $HeadURL$
// $LastChangedRevision$
// $Author$
// $LastChangedDate$
//
//============================================================================
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include "../difxmessage.h"
#include "difxmessageinternal.h"

const int MIN_SEND_GAP=40;

int expandEntityRefrences(char *dest, const char *src)
{
	int i, j;

	for(i = j = 0; src[i]; i++)
	{
		if(src[i] == '>')
		{
			strcpy(dest+j, "&gt;");
			j += 4;
		}
		else if(src[i] == '<')
		{
			strcpy(dest+j, "&lt;");
			j += 4;
		}
		else if(src[i] == '&')
		{
			strcpy(dest+j, "&amp;");
			j += 5;
		}
		else if(src[i] == '"')
		{
			strcpy(dest+j, "&quot;");
			j += 6;
		}
		else if(src[i] == '\'')
		{
			strcpy(dest+j, "&apos;");
			j += 6;
		}
		else if(src[i] < 32)	/* ascii chars < 32 are not allowed */
		{
			sprintf(dest+j, "[[%2d]]", src[i]);
			j += 6;
		}
		else
		{
			dest[j] = src[i];
			j++;
		}
	}

	dest[j] = 0;

	return j - i;
}

int difxMessageSend(const char *message)
{
	static int first = 1;
	static struct timeval tv0;

	struct timeval tv;
	int dt;

	if(difxMessagePort < 0)
	{
		return -1;
	}

	if(first)
	{
		first = 0;
		gettimeofday(&tv0, 0);
	}
	else
	{
		gettimeofday(&tv, 0);
		dt = 1000000*(tv.tv_sec - tv0.tv_sec) + (tv.tv_usec - tv0.tv_usec);
		if(dt < MIN_SEND_GAP && dt > 0)
		{
			/* The minimum gap prevents two messages from being sent too soon 
			 * after each other -- a condition that apparently can lead to lost
			 * messages 
			 */
			usleep(MIN_SEND_GAP-dt);
		}
		tv0 = tv;
	}

	return MulticastSend(difxMessageGroup, difxMessagePort, message, strlen(message));
}

int difxMessageSendProcessState(const char *state)
{
	const int MaxSize=200;
	char message[MaxSize];

	if(difxMessagePort < 0)
	{
		return -1;
	}

	snprintf(message, MaxSize, "%s %s", difxMessageIdentifier, state);

	return difxMessageSend(message);
}

int difxMessageSendLoad(const DifxMessageLoad *load)
{
	const int BodySize = 700;
	char message[DIFX_MESSAGE_LENGTH];
	char body[BodySize];
	int v;

	v = snprintf(body, BodySize,
		
		"<difxLoad>"
		  "<cpuLoad>%4.2f</cpuLoad>"
		  "<totalMemory>%d</totalMemory>"
		  "<usedMemory>%d</usedMemory>"
		  "<netRXRate>%d</netRXRate>"
		  "<netTXRate>%d</netTXRate>"
		"</difxLoad>",

		load->cpuLoad,
		load->totalMemory,
		load->usedMemory,
		load->netRXRate,
		load->netTXRate);

	if(v >= BodySize)
	{
		fprintf(stderr, "difxMessageSendLoad: message body overflow (%d >= %d)\n",
			v, BodySize);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat, 
		DifxMessageTypeStrings[DIFX_MESSAGE_LOAD],
		difxMessageSequenceNumber++, body);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendLoad: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

int difxMessageSendDifxAlert(const char *alertMessage, int severity)
{
	char message[DIFX_MESSAGE_LENGTH];
	char alertMessageExpanded[600];
	char body[DIFX_MESSAGE_LENGTH];
	int v;

	expandEntityRefrences(alertMessageExpanded, alertMessage);

	if(difxMessagePort < 0)
	{
		/* send to stderr or stdout if no port is defined */
		if(severity < DIFX_ALERT_LEVEL_WARNING)
		{
			fprintf(stderr, "[%s %d] %7s %s\n", difxMessageHostname, difxMessageMpiProcessId, difxMessageAlertString[severity], alertMessage);
		}
		else
		{
			printf("[%s %d] %7s %s\n", difxMessageHostname, difxMessageMpiProcessId, difxMessageAlertString[severity], alertMessage);
		}
	}
	else
	{
		v = snprintf(body, DIFX_MESSAGE_LENGTH,
			
			"<difxAlert>"
			  "<alertMessage>%s</alertMessage>"
			  "<severity>%d</severity>"
			"</difxAlert>",

			alertMessageExpanded,
			severity);

		if(v >= DIFX_MESSAGE_LENGTH)
		{
			fprintf(stderr, "difxMessageSendDifxAlert: message body overflow (%d >= %d)\n",
				v, DIFX_MESSAGE_LENGTH);
			return -1;
		}

		v = snprintf(message, DIFX_MESSAGE_LENGTH,
			difxMessageXMLFormat, 
			DifxMessageTypeStrings[DIFX_MESSAGE_ALERT],
			difxMessageSequenceNumber++, body);
		
		if(v >= DIFX_MESSAGE_LENGTH)
		{
			fprintf(stderr, "difxMessageSendDifxAlert: message overflow (%d >= %d)\n",
				v, DIFX_MESSAGE_LENGTH);
			return -1;
		}

		difxMessageSend(message);

		/* Make sure all fatal errors go to the console */
		if(severity == DIFX_ALERT_LEVEL_FATAL)
		{
			fprintf(stderr, "[%s %d] %7s %s\n", difxMessageHostname, difxMessageMpiProcessId, difxMessageAlertString[severity], alertMessage);
		}
	}
	
	return 0;
}

int difxMessageSendCondition(const DifxMessageCondition *cond)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	char bins[32*DIFX_MESSAGE_N_CONDITION_BINS];
	int i, v;
	char *b;

	bins[0] = 0;

	if(difxMessagePort < 0)
	{
		b = bins;
		for(i = 0; i < DIFX_MESSAGE_N_CONDITION_BINS; i++)
		{
			b += sprintf(b, " %d", cond->bin[i]);
		}
		printf("%s[%d] = %s %s\n", 
			cond->moduleVSN, 
			cond->moduleSlot,
			cond->serialNumber,
			bins);
	}
	else
	{
		b = bins;
		for(i = 0; i < DIFX_MESSAGE_N_CONDITION_BINS; i++)
		{
			b += sprintf(b, "<bin%d>%d</bin%d>", i, cond->bin[i], i);
		}

		v = snprintf(body, DIFX_MESSAGE_LENGTH,
			
			"<difxCondition>"
			  "<serialNumber>%s</serialNumber>"
			  "<modelNumber>%s</modelNumber>"
			  "<size>%d</size>"
			  "<moduleVSN>%s</moduleVSN>"
			  "<moduleSlot>%d</moduleSlot>"
			  "<startMJD>%7.5f</startMJD>"
			  "<stopMJD>%7.5f</stopMJD>"
			  "%s"
			"</difxCondition>",

			cond->serialNumber,
			cond->modelNumber,
			cond->diskSize,
			cond->moduleVSN, 
			cond->moduleSlot,
			cond->startMJD,
			cond->stopMJD,
			bins);

		if(v >= DIFX_MESSAGE_LENGTH)
		{
			fprintf(stderr, "difxMessageSendCondition: message body overflow (%d >= %d)\n",
				v, DIFX_MESSAGE_LENGTH);
			return -1;
		}

		v = snprintf(message, DIFX_MESSAGE_LENGTH,
			difxMessageXMLFormat, 
			DifxMessageTypeStrings[DIFX_MESSAGE_CONDITION],
			difxMessageSequenceNumber++, body);

		if(v >= DIFX_MESSAGE_LENGTH)
		{
			fprintf(stderr, "difxMessageSendCondition: message overflow (%d >= %d)\n",
				v, DIFX_MESSAGE_LENGTH);
			return -1;
		}
		
		difxMessageSend(message);
	}

	return 0;
}

int difxMessageSendMark5Status(const DifxMessageMk5Status *mk5status)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	char vsnA[10], vsnB[10];
	char scanName[64];
	char bank;
	int i, v;
	

	if(strlen(mk5status->vsnA) != 8)
	{
		strcpy(vsnA, "none");
	}
	else
	{
		for(i = 0; i < 9; i++)
		{
			vsnA[i] = toupper(mk5status->vsnA[i]);
		}
	}
	if(strlen(mk5status->vsnB) != 8)
	{
		strcpy(vsnB, "none");
	}
	else
	{
		for(i = 0; i < 9; i++)
		{
			vsnB[i] = toupper(mk5status->vsnB[i]);
		}
	}
	if(!isalpha(mk5status->activeBank))
	{
		bank = ' ';
	}
	else
	{
		bank = toupper(mk5status->activeBank);
	}
	strncpy(scanName, mk5status->scanName, 63);
	scanName[63] = 0;
	v = snprintf(body, DIFX_MESSAGE_LENGTH, 
	
		"<mark5Status>"
		  "<bankAVSN>%s</bankAVSN>"
		  "<bankBVSN>%s</bankBVSN>"
		  "<statusWord>0x%08x</statusWord>"
		  "<activeBank>%c</activeBank>"
		  "<state>%s</state>"
		  "<scanNumber>%d</scanNumber>"
		  "<scanName>%s</scanName>"
		  "<position>%lld</position>"
		  "<playRate>%7.3f</playRate>"
		  "<dataMJD>%13.7f</dataMJD>"
		"</mark5Status>",

		vsnA,
		vsnB,
		mk5status->status,
		bank,
		Mk5StateStrings[mk5status->state],
		mk5status->scanNumber,
		scanName,
		mk5status->position,
		mk5status->rate,
		mk5status->dataMJD);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendMark5Status: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat, 
		DifxMessageTypeStrings[DIFX_MESSAGE_MARK5STATUS],
		difxMessageSequenceNumber++, body);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendMark5Status: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}
	
	return difxMessageSend(message);
}

int difxMessageSendMk5Version(const DifxMessageMk5Version *mk5version)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	char dbInfo[DIFX_MESSAGE_LENGTH] = "";
	int v;

	if(mk5version->DB_PCBVersion[0] != 0)
	{
		v = snprintf(dbInfo, DIFX_MESSAGE_LENGTH,
		  "<DaughterBoard>"
		    "<PCBVer>%s</PCBVer>"
		    "<PCBType>%s</PCBType>"
		    "<PCBSubType>%s</PCBSubType>"
		    "<FPGAConfig>%s</FPGAConfig>"
		    "<FPGAConfigVer>%s</FPGAConfigVer>"
		  "</DaughterBoard>",

		mk5version->DB_PCBVersion,
		mk5version->DB_PCBType,
		mk5version->DB_PCBSubType,
		mk5version->DB_FPGAConfig,
		mk5version->DB_FPGAConfigVersion);

		if(v >= DIFX_MESSAGE_LENGTH)
		{
			fprintf(stderr, "difxMessageSendMk5Version: dbinfo overflow (%d >= %d)\n",
				v, DIFX_MESSAGE_LENGTH);
			return -1;
		}
	}

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
	
		"<mark5Version>"
		  "<ApiVer>%s</ApiVer>"
		  "<ApiDate>%s</ApiDate>"
		  "<FirmVer>%s</FirmVer>"
		  "<FirmDate>%s</FirmDate>"
		  "<MonVer>%s</MonVer>"
		  "<XbarVer>%s</XbarVer>"
		  "<AtaVer>%s</AtaVer>"
		  "<UAtaVer>%s</UAtaVer>"
		  "<DriverVer>%s</DriverVer>"
		  "<BoardType>%s</BoardType>"
		  "<SerialNum>%d</SerialNum>"
		  "%s"
		"</mark5Version>",

		mk5version->ApiVersion,
		mk5version->ApiDateCode,
		mk5version->FirmwareVersion,
		mk5version->FirmDateCode,
		mk5version->MonitorVersion,
		mk5version->XbarVersion,
		mk5version->AtaVersion,
		mk5version->UAtaVersion,
		mk5version->DriverVersion,
		mk5version->BoardType,
		mk5version->SerialNum,
		dbInfo);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendMk5Version: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat, 
		DifxMessageTypeStrings[DIFX_MESSAGE_MARK5STATUS],
		difxMessageSequenceNumber++, body);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendMk5Version: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}
	
	return difxMessageSend(message);
}

int difxMessageSendDifxStatus(enum DifxState state, const char *stateMessage,
	double visMJD, int numdatastreams, float *weight)
{
	char message[DIFX_MESSAGE_LENGTH];
	char weightstr[DIFX_MESSAGE_LENGTH];
	char stateMessageExpanded[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	int i, n, v;
	
	expandEntityRefrences(stateMessageExpanded, stateMessage);

	weightstr[0] = 0;
	n = 0;

	for(i = 0; i < numdatastreams; i++)
	{
		if(weight[i] >= 0)
		{
			n += sprintf(weightstr + n, 
				"<weight ant=\"%d\" wt=\"%5.3f\"/>", 
				i, weight[i]);

			if(n > DIFX_MESSAGE_LENGTH-100)
			{
				fprintf(stderr, "difxMessageSendDifxStatus: string overflow\n");
				return -1;
			}
		}
	}

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxStatus>"
		  "<state>%s</state>"
		  "<message>%s</message>"
		  "<visibilityMJD>%9.7f</visibilityMJD>"
		  "%s"
		"</difxStatus>",

		DifxStateStrings[state],
		stateMessageExpanded,
		visMJD,
		weightstr);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxStatus: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_STATUS],
		difxMessageSequenceNumber++, body);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxStatus: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}
	
	return difxMessageSend(message);
}

int difxMessageSendDifxStatus2(const char *jobName, enum DifxState state, const char *stateMessage)
{
	char message[DIFX_MESSAGE_LENGTH];
	char stateMessageExpanded[DIFX_MESSAGE_LENGTH];
	int v;

	expandEntityRefrences(stateMessageExpanded, stateMessage);

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<difxMessage>"
		  "<header>"
		    "<from>%s</from>"
		    "<mpiProcessId>-1</mpiProcessId>"
		    "<identifier>%s</identifier>"
		    "<type>DifxStatusMessage</type>"
		  "</header>"
		  "<body>"
		    "<seqNumber>%d</seqNumber>"
		    "<difxStatus>"
		      "<state>%s</state>"
		      "<message>%s</message>"
		      "<visibilityMJD>0</visibilityMJD>"
		    "</difxStatus>"
		  "</body>"
		"</difxMessage>\n",
		difxMessageHostname,
		jobName,
		difxMessageSequenceNumber++,
		DifxStateStrings[state],
		stateMessageExpanded);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxStatus2: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

int difxMessageSendDifxInfo(const char *infoMessage)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	char infoMessageExpanded[DIFX_MESSAGE_LENGTH];
	int v;

	expandEntityRefrences(infoMessageExpanded, infoMessage);

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxInfo>"
		  "<message>%s</message>"
		"</difxInfo>",

		infoMessageExpanded);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxInfo: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_INFO],
		difxMessageSequenceNumber++, body);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxInfo: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

int difxMessageSendDifxCommand(const char *command)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	char commandExpanded[DIFX_MESSAGE_LENGTH];
	int v;

	expandEntityRefrences(commandExpanded, command);

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxCommand>"
		  "<command>%s</command>"
		"</difxCommand>",

		commandExpanded);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendCommand: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_COMMAND],
		difxMessageSequenceNumber++, body);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendCommand: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

/* mpiDestination: 
	>= 0 implies mpiId, 
	  -1 implies ALL, 
	  -2 implies all Datastrems, 
	  -3 implies all Cores
*/
int difxMessageSendDifxParameter(const char *name, 
	const char *value, int mpiDestination)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	int v;

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxParameter>"
		  "<targetMpiId>%d</targetMpiId>"
		  "<name>%s</name>"
		  "<value>%s</value>"
		"</difxParameter>",

		mpiDestination,
		name,
		value);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendParameter: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_PARAMETER],
		difxMessageSequenceNumber++, body);

	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendParameter: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

int difxMessageSendDifxParameter1(const char *name, int index1,
	const char *value, int mpiDestination)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	int v;

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxParameter>"
		  "<targetMpiId>%d</targetMpiId>"
		  "<name>%s</name>"
		  "<index1>%d</index1>"
		  "<value>%s</value>"
		"</difxParameter>",

		mpiDestination,
		name,
		index1,
		value);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxParameter1: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_PARAMETER],
		difxMessageSequenceNumber++, body);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxParameter1: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

int difxMessageSendDifxParameter2(const char *name, int index1, int index2,
	const char *value, int mpiDestination)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	int v;

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxParameter>"
		  "<targetMpiId>%d</targetMpiId>"
		  "<name>%s</name>"
		  "<index1>%d</index1>"
		  "<index2>%d</index1>"
		  "<value>%s</value>"
		"</difxParameter>",

		mpiDestination,
		name,
		index1,
		index2,
		value);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxParameter2: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_PARAMETER],
		difxMessageSequenceNumber++, body);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxParameter2: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}

int difxMessageSendDifxParameterGeneral(const char *name, int nIndex, const int *index,
	const char *value, int mpiDestination)
{
	char message[DIFX_MESSAGE_LENGTH];
	char body[DIFX_MESSAGE_LENGTH];
	char indices[DIFX_MESSAGE_LENGTH];
	int i, v;
	int p=0;

	for(i = 0; i < nIndex; i++)
	{
		p += sprintf(indices + p, "<index%d>%d</index%d>", i+1, index[i], i+1);

		if(p > DIFX_MESSAGE_LENGTH-100)
		{
			fprintf(stderr, "difxMessageSendDifxParameterGeneral: string overflow\n");
			return -1;
		}
	}

	v = snprintf(body, DIFX_MESSAGE_LENGTH,
		
		"<difxParameter>"
		  "<targetMpiId>%d</targetMpiId>"
		  "<name>%s</name>"
		  "%s"
		  "<value>%s</value>"
		"</difxParameter>",

		mpiDestination,
		name,
		indices,
		value);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxParameterGeneral: message body overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	v = snprintf(message, DIFX_MESSAGE_LENGTH,
		difxMessageXMLFormat,
		DifxMessageTypeStrings[DIFX_MESSAGE_PARAMETER],
		difxMessageSequenceNumber++, body);
	
	if(v >= DIFX_MESSAGE_LENGTH)
	{
		fprintf(stderr, "difxMessageSendDifxParameterGeneral: message overflow (%d >= %d)\n",
			v, DIFX_MESSAGE_LENGTH);
		return -1;
	}

	return difxMessageSend(message);
}
