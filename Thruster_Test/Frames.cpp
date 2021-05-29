/*#include "Frames.h"



//Constructors
Frame::Frame(std::string empty)
{
	incoming = empty;
}
LongFrame::LongFrame(std::string empty)
{
	Frame::incoming = empty;
}
ShortFrame::ShortFrame(std::string empty)
{
	incoming = empty;
}


//Functions
void Frame::ReadNConvert(System::String^ ManInc, std::string out[])
{
	std::string inc = msclr::interop::marshal_as<std::string>(ManInc);

	int frameindex = 0;
	int stringindex = 0;
	out[stringindex] = "";
	while (inc[frameindex] != '>')
	{
		if (inc[frameindex] != '<')
		{
			if (inc[frameindex] != '|')
				out[stringindex] += inc[frameindex];
			else
			{
				stringindex++;
				out[stringindex] = "";
			}
		}

		frameindex++;
	}
}
void Frame::ConvertNWrite(std::string inc[], System::String^& out)
{
	out = "";
	out += "<";
	for (int wordIndex = 0; wordIndex < SHORT_FRAME_LENGTH; wordIndex++)
	{
		out += msclr::interop::marshal_as<System::String^>(inc[wordIndex]);
		if (wordIndex + 1 != SHORT_FRAME_LENGTH)
			out += "|";
	}
	out += ">";
}


void Frame::StringToFloatConv(std::string inc[], float out[], int length)
{
	for (int i = 0; i < length; i++)
	{
		out[i] = std::stof(inc[i]);
	}
}
void Frame::ModeToStringConv(ModeVals inc, std::string out[], int length)
{
	out[0] = std::to_string(inc.mode);
	out[1] = std::to_string(inc.duty);
	out[2] = std::to_string(inc.valOne);
	out[3] = std::to_string(inc.stay_time);
	out[4] = std::to_string(inc.valTwo);
}

void FrameToData(System::String^ readline, DataVals& data, LongFrame& LF)
{

	LF.ReadNConvert(readline, LF.interWords);

	LF.StringToFloatConv(LF.interWords, LF.convertedValues, LONG_FRAME_LENGTH);
	data.mins = (int)LF.convertedValues[0];
	data.secs = (int)LF.convertedValues[1];
	data.milis = (int)LF.convertedValues[2];
	data.temp1 = LF.convertedValues[3];
	data.temp2 = LF.convertedValues[4];
	data.tenso = LF.convertedValues[5];
	data.shunt = LF.convertedValues[6];
	for (int i = 7; i < 17; i++)
	{
		data.vibro[i] = LF.convertedValues[i];
	}

	data.timeInSecs = (float)(data.milis * 0.001 + data.secs + data.mins * 60);
}

System::String^ SendFrame(ModeVals& mode, ShortFrame& SF)
{
	System::String^ message = "<0|0|0|0|0>";
	SF.ModeToStringConv(mode, SF.interWords, SHORT_FRAME_LENGTH);
	SF.ConvertNWrite(SF.interWords, message);

	return message;
}*/