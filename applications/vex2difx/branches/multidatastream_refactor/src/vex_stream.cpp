#include <cstdlib>
#include <cstring>
#include <regex.h>
#include "vex_stream.h"

bool VexStream::isInit(Init());	// force execution of the function below to initialize regular expressions
bool VexStream::Init()
{
	int v;

	// of form <fmt>/<threads>/<size>/<bits>	VDIF only
	v = regcomp(&matchType1, "^([A-Z]*VDIF[A-Z]*)/([1-9]+[0-9]*)/([1-9]+[0-9]*)/([1-9]+[0-9]*)$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType1 failed" << std::endl;

		exit(EXIT_FAILURE);
	}

	// of form <fmt>/<size>/<bits>		VDIF only
	v = regcomp(&matchType2, "^([A-Z]*VDIF[A-Z]*)/([1-9]+[0-9]*)/([1-9]+[0-9]*)$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType2 failed" << std::endl;

		exit(EXIT_FAILURE);
	}

	// of form <fmt><size>			VDIF only
	v = regcomp(&matchType3, "^([A-Z]*VDIF[A-Z]*])([1-9]+[0-9]*)$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType3 failed" << std::endl;

		exit(EXIT_FAILURE);
	}

	// of form <fmt>1_<fanout>	VLBA, Mark4, VLBN only
	v = regcomp(&matchType4, "^([A-Z]+])1_([124])$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType4 failed" << std::endl;

		exit(EXIT_FAILURE);
	}

	// of form <fmt>_<size>-<Mbps>-<nChan>-<bits>	VDIF only
	v = regcomp(&matchType5, "^([A-Z]*VDIF[A-Z]*)_([1-9]+[0-9]*)-([1-9]+[0-9]*)-([1-9]+[0-9]*)-([1-9]+[0-9]*)$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType5 failed" << std::endl;

		exit(EXIT_FAILURE);
	}

	// of form <fmt>-<Mbps>-<nChan>-<bits>
	v = regcomp(&matchType6, "^([A-Z]+)-([1-9]+[0-9]*)-([1-9]+[0-9]*)-([1-9]+[0-9]*)$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType6 failed" << std::endl;

		exit(EXIT_FAILURE);
	}

	// of form <fmt>1_<fanout>-<Mbps>-<nChan>-<bits>	Mark4, VLBA, VLBN only
	v = regcomp(&matchType7, "^([A-Z]+)1_([1-9]+[0-9]*)-([1-9]+[0-9]*)-([1-9]+[0-9]*)-([1-9]+[0-9]*)$", REG_EXTENDED);
	if(v != 0)
	{
		std::cerr << "Developer Error: VexStream::Init(): compiling matchType7 failed" << std::endl;

		exit(EXIT_FAILURE);
	}
	
	return true;
}

char VexStream::DataFormatNames[NumDataFormats+1][16] = 
{
	"NONE",
	"VDIF",
	"VDIFL",
	"Mark5B",
	"VLBA",
	"VLBN",
	"MKIV",
	"KVN5B",
	"LBASTD",
	"LBAVSOP",
	"S2",

	"Error"		// keep at the end
};

bool VexStream::isSingleThreadVDIF(const std::string &str)
{
	if(strcasecmp(str.c_str(), "VDIF") == 0 ||
	   strcasecmp(str.c_str(), "VDIFL") == 0)
	{
		return true;
	}

	return false;
}

enum VexStream::DataFormat VexStream::stringToDataFormat(const std::string &str)
{
	if(strcasecmp(str.c_str(), "VDIF") == 0 ||
	   strcasecmp(str.c_str(), "INTERLACEDVDIF") == 0)
	{
		return FormatVDIF;
	}
	else if(strcasecmp(str.c_str(), "VDIFL") == 0)
	{
		return FormatLegacyVDIF;
	}
	else if(strcasecmp(str.c_str(), "Mark5B") == 0 ||
	        strcasecmp(str.c_str(), "MK5B") == 0)
	{
		return FormatMark5B;
	}
	else if(strcasecmp(str.c_str(), "VLBA") == 0)
	{
		return FormatVLBA;
	}
	else if(strcasecmp(str.c_str(), "VLBN") == 0)
	{
		return FormatVLBN;
	}
	else if(strcasecmp(str.c_str(), "MKIV") == 0 ||
	        strcasecmp(str.c_str(), "Mark4") == 0)
	{
		return FormatMark4;
	}
	else if(strcasecmp(str.c_str(), "KVN5B") == 0)
	{
		return FormatKVN5B;
	}
	else if(strcasecmp(str.c_str(), "LBA") == 0 ||
	        strcasecmp(str.c_str(), "LBASTD") == 0)
	{
		return FormatLBASTD;
	}
	else if(strcasecmp(str.c_str(), "LBAVSOP") == 0)
	{
		return FormatLBAVSOP;
	}
	else if(strcasecmp(str.c_str(), "S2") == 0)
	{
		return FormatS2;
	}

	return NumDataFormats;
}

// FIXME: move to parserhelp
bool VexStream::parseThreads(const std::string &threadList)
{
	int i, t;
	char c;

	threads.clear();

	t = -1;
	i = 0;
	do
	{
		c = threadList[i];
		if(c >= '0' && c <= '9')
		{
			if(t < 0)
			{
				t = 0;
			}
			t *= 10;
			t += (c - '0');
		}
		else if(c == ',' || c == 0)
		{
			if(t < 0)
			{
				threads.clear();

				return false;
			}
			if(t >= 1024)
			{
				threads.clear();

				return false;
			}
			threads.push_back(t);
			t = -1;
		}
		else
		{
			threads.clear();
			
			return false;
		}
		++i;
	}
	while(c > 0);

	return true;
}

// Accepts strings of the following formats and populates appropriate members:

// VDIF/<size>/<bits>  -> format=VDIF,  dataFrameSize=<size>, nBit = <bits>, nThread=1, threads=[]
// VDIFL/<size>/<bits> -> format=VDIFL, dataFrameSize=<size>, nBit = <bits>, nThread=1, threads=[]
// VDIF_<size>-<Mbps>-<nChan>-<bits>   -> format=VDIF,  dataFrameSize=<size>+32, nBit=<bits>, nThread=1, threads=[], nChan=<nChan>
// VDIFL_<size>-<Mbps>-<nChan>-<bits>  -> format=VDIFL, dataFrameSize=<size>+16, nBit=<bits>, nThread=1, threads=[], nChan=<nChan>
// INTERLACEDVDIF/0:1:16:17/1032/2

static int matchInt(const std::string &str, const regmatch_t &match)
{
	return atoi(str.substr(match.rm_so, match.rm_eo-match.rm_so).c_str());
}

bool VexStream::parseFormatString(const std::string &formatName)
{
	const int MaxMatches = 6;
	regmatch_t match[MaxMatches];

	nRecordChan = 0;
	nBit = 0;
	nThread = 0;
	singleThread = false;

	for(int df = 0; df < NumDataFormats; ++df)
	{
		if(strcasecmp(formatName.c_str(), DataFormatNames[df]) == 0)
		{
			// Here we match the format name but get no additional information
			format = static_cast<DataFormat>(df);

			return true;
		}
	}

	if(regexec(&matchType1, formatName.c_str(), 5, match, 0) == 0)
	{
		// of form <fmt>/<threads>/<size>/<bits>
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		nThread = matchInt(formatName, match[2]);
		dataFrameSize = matchInt(formatName, match[3]);
		nBit = matchInt(formatName, match[4]);
	}
	else if(regexec(&matchType2, formatName.c_str(), 4, match, 0) == 0)
	{
		// of form <fmt>/<size>/<bits>
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		dataFrameSize = matchInt(formatName, match[2]);
		nBit = matchInt(formatName, match[3]);
		singleThread = isSingleThreadVDIF(formatName);
	}
	else if(regexec(&matchType3, formatName.c_str(), 3, match, 0) == 0)
	{
		// of form <fmt><size>	VDIF only
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		dataFrameSize = matchInt(formatName, match[2]);
		singleThread = isSingleThreadVDIF(formatName);
	}
	else if(regexec(&matchType4, formatName.c_str(), 3, match, 0) == 0)
	{
		// of form <fmt>1_<fanout>
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		if(format != FormatVLBA && format != FormatVLBN && format != FormatMark4)
		{
			format = NumDataFormats;

			return false;
		}
		fanout = matchInt(formatName, match[2]);
	}
	else if(regexec(&matchType5, formatName.c_str(), 6, match, 0) == 0)
	{
		// of form <fmt>_<size>-<Mbps>-<nChan>-<bits>	VDIF only
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		fanout = matchInt(formatName, match[2]);
		singleThread = isSingleThreadVDIF(formatName);
	}
	else if(regexec(&matchType6, formatName.c_str(), 5, match, 0) == 0)
	{
		// of form <fmt>-<Mbps>-<nChan>-<bits>
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		// Mbps not captured
		nRecordChan = matchInt(formatName, match[3]);
		nBit = matchInt(formatName, match[4]);
		singleThread = isSingleThreadVDIF(formatName);
	}
	else if(regexec(&matchType7, formatName.c_str(), 6, match, 0) == 0)
	{
		// of form <fmt>1_<fanout>-<Mbps>-<nChan>-<bits>	Mark4, VLBA, VLBN only
		format = stringToDataFormat(formatName.substr(0, match[1].rm_eo));
		if(format == NumDataFormats)
		{
			return false;
		}
		if(format != FormatVLBA && format != FormatVLBN && format != FormatMark4)
		{
			format = NumDataFormats;

			return false;
		}
		fanout = matchInt(formatName, match[2]);
		// Mbps not captured
		nRecordChan = matchInt(formatName, match[4]);
		nBit = matchInt(formatName, match[5]);
	}

	return false;
}

bool VexStream::isTrackFormat() const
{
	if(singleThread)
	{
		return true;
	}

	return (format == FormatVLBA || format == FormatVLBN || format == FormatMark4 || format == FormatMark5B || format == FormatKVN5B);
}

bool VexStream::isLBAFormat() const
{
	return (format == FormatLBASTD || format == FormatLBAVSOP);
}

bool VexStream::formatHasFanout() const
{
	return (format == FormatVLBA || format == FormatVLBN || format == FormatMark4);
}

bool VexStream::isVDIFFormat() const
{
	return (format == FormatVDIF || format == FormatLegacyVDIF);
}

void VexStream::setFanout(int fan)
{
	fanout = fan;
}

std::ostream& operator << (std::ostream &os, const VexStream &x)
{
	os << " [format=" << VexStream::DataFormatNames[x.format] << ", nBit=" << x.nBit << ", nRecordChan=" << x.nRecordChan << ", nThread=" << x.nThread << ", singleThread=" << x.singleThread << ", sampRate=" << x.sampRate;
	if(!x.threads.empty())
	{
		for(std::vector<int>::const_iterator it = x.threads.begin(); it != x.threads.end(); ++it)
		{
			if(it == x.threads.begin())
			{
				os << ", threads=";
			}
			else
			{
				os << ",";
			}
			os << *it;
		}
	}
	os << "]";
	
	return os;
}
