#include "DxfReader.h"
#include "Core/StringUtils.h"
#include "IO/Log.h"


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
	if (source_->IsEof()) { return -1; }
	
	//grab the next line
	String l = source_->ReadLine();

	//get the code
	int code = ToInt(l);

	//switch over group codes
	switch (code)
	{
	case 0:
		URHO3D_LOGINFO("Starting or ending a group.");
		ReadGroupName();
		break;
	default:
		URHO3D_LOGINFO("Group code {0} not implemented.", code);
		break;
	}

}

String DxfReader::ReadGroupName()
{
	//check for end of file
	if (source_->IsEof()) { return ""; }

	//grab the next line
	String l = source_->ReadLine();

	//trim? To upper?
	if (l == "SECTION")
	{
		URHO3D_LOGINFO("Parsing section");
	}
	else if (l == "ENDSEC")
	{
		URHO3D_LOGINFO("Finished parsing section");
		ReadGroupCode();
	}
	else
	{
		URHO3D_LOGINFO("Group name {0} not implemented.", l);
	}

	return l;
}


