/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _H
#define _H


#include <SupportDefs.h>
#include <TextControl.h>
#include <Rect.h>
#include <Window.h>
#include <ObjectList.h>
#include <String.h>
#include <NodeInfo.h>

const uint32 kAttributeChanged = 'achg';
const uint32 kAttributeRefresh = 'aref';
const uint32 kAttributeEditClose = 'acls';

class AttributeEditor : public BWindow {
public:
	AttributeEditor(BPoint at, BWindow *listener,  entry_ref& ref);
	void Setup();
    void GetAttributes(BMessage info);
	virtual bool QuitRequested();    
protected:
	virtual void MessageReceived(BMessage*);

private:
	void	GetFileAttributes();
	void	SetAttributeValues();
	void	SaveActiveAttribute();
	void	SaveChangedAttribute(BString attrName);
	BMessenger 			fListener;
	entry_ref&          entryRef;
	entry_ref*    		activeRef;
	struct Attribute {
			BString		attribute;
			int32		type;
			int32		width;
			BString		name;
			BString		publicName;
			BString		stringValue;
			BTextControl *textControl;

	};
	BTextControl *locationTextControl;
	BObjectList<Attribute> fAttributes;
	enum {
		kMsgOK = 'mPOW',
	};
};


#endif // _H
