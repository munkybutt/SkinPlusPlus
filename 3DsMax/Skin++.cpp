//**************************************************************************/
// Copyright (c) 1998-2018 Autodesk, Inc.
// All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "Skin++.h"

#define SkinPlusPlus_CLASS_ID	Class_ID(0x99f0ea10, 0x69916a18)


class SkinPlusPlus : public GUP
{
public:
	//Constructor/Destructor
	SkinPlusPlus();
	virtual ~SkinPlusPlus();

	// GUP Methods
	virtual DWORD     Start();
	virtual void      Stop();
	virtual DWORD_PTR Control(DWORD parameter);
	virtual void      DeleteThis();

	// Loading/Saving
	virtual IOResult Save(ISave* isave);
	virtual IOResult Load(ILoad* iload);
};



class SkinPlusPlusClassDesc : public ClassDesc2 
{
public:
	virtual int           IsPublic() override                       { return TRUE; }
	virtual void*         Create(BOOL /*loading = FALSE*/) override { return new SkinPlusPlus(); }
	virtual const TCHAR * ClassName() override                      { return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID     SuperClassID() override                   { return GUP_CLASS_ID; }
	virtual Class_ID      ClassID() override                        { return SkinPlusPlus_CLASS_ID; }
	virtual const TCHAR*  Category() override                       { return GetString(IDS_CATEGORY); }

	virtual const TCHAR*  InternalName() override                   { return _T("SkinPlusPlusUtility"); } // Returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE     HInstance() override                      { return hInstance; } // Returns owning module handle


};

ClassDesc2* GetSkinPlusPlusDesc()
{
	static SkinPlusPlusClassDesc SkinPlusPlusDesc;
	return &SkinPlusPlusDesc; 
}




SkinPlusPlus::SkinPlusPlus()
{

}

SkinPlusPlus::~SkinPlusPlus()
{

}

void SkinPlusPlus::DeleteThis()
{
	delete this;
}

// Activate and Stay Resident
DWORD SkinPlusPlus::Start()
{
	return GUPRESULT_KEEP;
}

void SkinPlusPlus::Stop()
{
}

DWORD_PTR SkinPlusPlus::Control( DWORD /*parameter*/ )
{
	return 0;
}

IOResult SkinPlusPlus::Save(ISave* /*isave*/)
{
	return IO_OK;
}

IOResult SkinPlusPlus::Load(ILoad* /*iload*/)
{
	return IO_OK;
}

