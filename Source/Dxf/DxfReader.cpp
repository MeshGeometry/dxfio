#include "DxfReader.h"
#include "Core/StringUtils.h"
#include "IO/Log.h"

//these codes signal that a new group is starting or ending
static const char* topLevelCodes[] = 
{
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9"
};


DxfReader::DxfReader(Context* context, String path) : Object(context)
{
	FileSystem* fs = new FileSystem(GetContext());
	bool res = fs->FileExists(path);

	//make sure that this file exists
	assert(res);

	//create the file
	source_ = new File(GetContext(), path, FILE_READ);

	//double check
	assert(source_);

	//create the log
	GetContext()->RegisterSubsystem(new Log(GetContext()));

}

int DxfReader::ReadGroupCode()
{
	//check for end of file
	if (source_->IsEof()) { 
		return -1; 
	}
	
	//grab the next line
	String l = source_->ReadLine().Trimmed();

	URHO3D_LOGINFO("Code Value: " + l);

	//get the code
	int code = ToInt(l);

	//switch over group codes
	switch (code)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		//URHO3D_LOGINFO("Starting or ending a group. Code " + String(code));
		ReadGroupName();
		break;
	default:
		//URHO3D_LOGINFO("Group code " + String(code) + " not implemented.");
		break;
	}

}

String DxfReader::ReadGroupName()
{
	//check for end of file
	if (source_->IsEof()) { 
		return ""; 
	}

	//grab the next line
	String l = source_->ReadLine().Trimmed();

	URHO3D_LOGINFO("Group Name: " + l);

	//in any case, try to read the value of the group
	ReadGroupValue();

	return l;
}

Variant DxfReader::ReadGroupValue()
{
	//check for end of file
	if (source_->IsEof()) {
		return "";
	}

	//grab the next line
	int lastLength = 0;
	String l = source_->ReadLine();
	lastLength = l.Length();
	l = l.Trimmed();
	URHO3D_LOGINFO("Group Value: " + l);

	//parse group content until we encounter a digit between 0 -9
	int counter = 0;
	int topLevelIndex = GetStringListIndex(l.CString(), topLevelCodes, -1);
	while (topLevelIndex < 0 && counter < 10)
	{
		l = source_->ReadLine();
		lastLength = l.Length();
		l = l.Trimmed();
		URHO3D_LOGINFO("Group Value: " + l);
		topLevelIndex = GetStringListIndex(l.CString(), topLevelCodes, -1);
		counter++;
	}

	//back up by a line and call group code
	source_->Seek(source_->GetPosition() - lastLength);
	ReadGroupCode();
}


