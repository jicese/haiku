/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "AttributeEditor.h"
#include <Box.h>
#include <Alert.h>
#include <Button.h>
#include <CheckBox.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <GridLayoutBuilder.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <GridView.h>
#include <Mime.h>
#include <Locale.h>
#include <String.h>
#include <Roster.h>
#include <NodeInfo.h>
#include <fs_attr.h>
#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AttributeEditor"

enum {
	MSG_OPENMAP = 'omap',
};

AttributeEditor::AttributeEditor(BPoint at, BWindow *listener,  entry_ref& ref)
	:BWindow(BRect(at.x, at.y, at.x + 500, at.y + 220),
	 B_TRANSLATE("Edit Attributes"),
     B_FLOATING_WINDOW,
	 B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	 fListener(listener),
	 entryRef(ref)
{
	activeRef = new entry_ref(entryRef);
	AddToSubset(listener);
	Setup();
	Show();
}

void AttributeEditor::Setup()
{
	GetFileAttributes();

	// Update title
	BEntry entry(&entryRef);
	char fileName[B_FILE_NAME_LENGTH];
	entry.GetName(fileName);
    BString title(B_TRANSLATE("Edit Attributes"));
    title << ": " << fileName;
	SetTitle(title.String());
	
	BGridView* view = new BGridView(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING);
	BGridLayout* layout = view->GridLayout();
	float spacing = be_control_look->DefaultItemSpacing();
	layout->SetInsets(spacing, spacing, spacing, spacing);
    SetLayout(layout);
    locationTextControl = NULL;
	int32 count = fAttributes.CountItems();
	for (int32 i = 0; i < count; i++) {
		Attribute* attribute = fAttributes.ItemAt(i);
		
        BMessage *controlMessage = new BMessage(kAttributeChanged);
        controlMessage->AddString("attribute", attribute->name);
 		BTextControl *control= new BTextControl(attribute->name, attribute->publicName, attribute->stringValue, controlMessage);
		control->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
		layout->AddItem(control->CreateLabelLayoutItem(), 0, i, 1, 1);
		if(attribute->width>99)
			layout->AddItem(control->CreateTextViewLayoutItem(), 1, i, 3, 1);
		else
			layout->AddItem(control->CreateTextViewLayoutItem(), 1, i, 1, 1);
		attribute->textControl = control;
		if(!strcmp(attribute->name, "Location"))
		{
			locationTextControl = control;
			BButton *button = new BButton("☉", new BMessage(MSG_OPENMAP));
//			button->SetFlat(true);
			button->SetExplicitMaxSize(BSize(be_plain_font->Size()*2, be_plain_font->Size() * 2));
			button->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));
			layout->AddView(button, 2, i, 1, 1);
		}
	}
}

void AttributeEditor::GetFileAttributes()
{
	const entry_ref *ref = &entryRef;
	BNode node(ref);

	if (node.InitCheck() != B_OK)
		return;

	BNodeInfo nodeInfo(&node);
	if (nodeInfo.InitCheck() != B_OK)
		return;
		
	char mimeString[B_MIME_TYPE_LENGTH];
	if (nodeInfo.GetType(mimeString) != B_OK)
	{
		//TODO fallback to image
		return;
	}

	BMessage info;
	BMimeType mime(mimeString);
	BMimeType superType;
	fAttributes.MakeEmpty();
	if(mime.GetSupertype(&superType) == B_OK)
	{
		if(superType.GetAttrInfo(&info) == B_NO_ERROR) {
			GetAttributes(info);
		}
	}
	
	if(mime.GetAttrInfo(&info) == B_NO_ERROR) {
		GetAttributes(info);
	}
}

void AttributeEditor::GetAttributes(BMessage info)
{
		bool editable;

		const entry_ref *ref = &entryRef;
		BFile* file =  new(std::nothrow) BFile(ref, B_READ_ONLY);
		attr_info attrInfo;
	
		int32 index = 0;
		while(info.FindBool("attr:editable", index, &editable) == B_OK)
		{
			if(!editable)
			{
				index++;
				continue;
			}

			Attribute *attribute = new Attribute();
			if(info.FindString("attr:name", index, &attribute->name) != B_OK)
				break;

			if(info.FindString("attr:public_name", index, &attribute->publicName) != B_OK)
				break;

			if(info.FindInt32("attr:width", index, &attribute->width) != B_OK)
				break;
				
			if(info.FindInt32("attr:type", index, &attribute->type) != B_OK)
				break;

			if (file != NULL && file->GetAttrInfo(attribute->name, &attrInfo) == B_OK)
			{
				if(attribute->type == B_STRING_TYPE)
				{
					char *value = (char*)calloc(attrInfo.size, 1);
					file->ReadAttr(attribute->name, B_STRING_TYPE, 0, value, attrInfo.size);
					attribute->stringValue = value;
					free(value);
				}
				if(attribute->type == B_INT32_TYPE)
				{
					int32 value;
					file->ReadAttr(attribute->name, B_INT32_TYPE, 0, &value, sizeof(int32));
					attribute->stringValue.SetToFormat("%" B_PRId32, value);
				}
				if(attribute->type == B_DOUBLE_TYPE)
				{
					double value;
					file->ReadAttr(attribute->name, B_DOUBLE_TYPE, 0, &value, sizeof(double));
					attribute->stringValue.SetToFormat("%lf", value);
				}			
			}

			//BAlert *alert = new BAlert("Found attribute", attribute->name.String(), "OK");
			//alert->Go(NULL);
			fAttributes.AddItem(attribute);
			index++;
		}
		delete file;
}

void AttributeEditor::SetAttributeValues()
{
	const entry_ref *ref = &entryRef;
	attr_info attrInfo;
	BFile* file =  new(std::nothrow) BFile(ref, B_READ_ONLY);
		
	if(!file)
	{
		// TODO Clear all attributes and values?
		return;
	}
	
	// Update title
	BEntry entry(&entryRef);
	char fileName[B_FILE_NAME_LENGTH];
	entry.GetName(fileName);
    BString title(B_TRANSLATE("Edit Attributes"));
    title << ": " << fileName;
	SetTitle(title.String());

	for (int32 index = 0; index < fAttributes.CountItems(); ++index) {
		Attribute* attribute = fAttributes.ItemAt(index);
		attribute->stringValue = "";
		printf("Name %s\n", attribute->name.String());
	
		if (file->GetAttrInfo(attribute->name, &attrInfo) == B_OK)
		{
			if(attribute->type == B_STRING_TYPE)
			{
				char *value = (char*)calloc(attrInfo.size, 1);
				file->ReadAttr(attribute->name, B_STRING_TYPE, 0, value, attrInfo.size);
				attribute->stringValue = value;
				free(value);
			}
			if(attribute->type == B_INT32_TYPE)
			{
				int32 value;
				file->ReadAttr(attribute->name, B_INT32_TYPE, 0, &value, sizeof(int32));
				attribute->stringValue.SetToFormat("%" B_PRId32, value);
			}
			if(attribute->type == B_DOUBLE_TYPE)
			{
				double value;
				file->ReadAttr(attribute->name, B_DOUBLE_TYPE, 0, &value, sizeof(double));
				attribute->stringValue.SetToFormat("%lf", value);
			}
		}
		attribute->textControl->SetText(attribute->stringValue.String());
	}
	delete file;
}

void AttributeEditor::MessageReceived(BMessage* message)
{
	switch(message->what) 
	{
		case MSG_OPENMAP:
		{
			//Test
			float lon = 0;
			float lat = 0;
			if(strchr(locationTextControl->Text(), '/'))
			{
				char *locationString = (char*)malloc(strlen(locationTextControl->Text()) + 1);
				if(locationString)
				{
					strcpy(locationString, locationTextControl->Text());
					lon=atof(strtok(locationString, "/"));
					lat=atof(strtok(NULL, "/"));
					printf ("To send %f %f\n", lon, lat);
					free(locationString);
				}
			}			
/*			
			BString mapURL("http://www.openstreetmap.org");
			if(strchr(locationTextControl->Text(), '/') >  0)
			{
				char *locationString = (char*)malloc(strlen(locationTextControl->Text()) + 1);
				if(locationString)
				{
					strcpy(locationString, locationTextControl->Text());
					mapURL << "/?mlat=" << strtok(locationString, "/");
					mapURL << "&mlon=" << strtok(NULL, "/");
					//mapURL << "/#map=12/" << locationTextControl->Text();
					printf("MAP URL: %s\n", mapURL.String());
					free(locationString);
				}
			}
			const char * args[] = { mapURL.String(), NULL };
			be_roster->Launch("application/x-vnd.Haiku-WebPositive", 1, (char **)args);
*/
					
			BMessage* message = new BMessage(B_SET_PROPERTY);
			message->AddFloat("longitude", lon);
			message->AddFloat("latitude", lat);
			team_id mapTeam = be_roster->TeamFor("application/x-vnd.Haiku-Maps");
			if (mapTeam < 0) {
				status_t result = be_roster->Launch("application/x-vnd.Haiku-Maps", message);
				if (result != B_NO_ERROR) {
					BAlert* alert = new BAlert("", B_TRANSLATE(
					 "Map application not found."), B_TRANSLATE("OK"));
					alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
					alert->Go();
					return;
				}
			} 
			else {
				app_info appInfo;
				if (be_roster->GetRunningAppInfo(mapTeam, &appInfo) == B_OK) {
					BMessenger messenger(appInfo.signature, mapTeam);
					if (messenger.IsValid())
					{
						printf("Sending location to Maps\n");
						messenger.SendMessage(message);
					}
				}
			}
			break;
		}
		case kAttributeRefresh:
		{
			SaveActiveAttribute();
			message->FindRef("ref", &entryRef);
			activeRef = new entry_ref(entryRef);
			//TODO mime change
			SetAttributeValues();
			break;
		}
		case B_MIME_DATA:
			printf("Drop\n");
			const void *data;
			ssize_t size;
			if (message->FindData("text/plain", B_MIME_TYPE, &data, &size) == B_OK)
			{
				printf("Data: %s\n", (char *)data);

				// TODO parse lat / lon, set to locationTextControl
				char *locationString = (char*)malloc(strlen((char *)data) + 1);
				if(locationString)
				{
					strcpy(locationString, (char *)data);
					printf("LocStr: %s\n", locationString);
					char *locUrl = strtok(locationString, "#");
					locUrl = strtok(NULL, "#");
					printf("LocURL: %s\n", locUrl);
					strtok(locUrl, "/");
					locationTextControl->SetText(strtok(NULL, "a"));
					free(locationString);
				}

			}

			break;
		case kAttributeChanged:
		{
			BString attrName;
			printf("Edit\n");
			message->FindString("attribute", &attrName);
			SaveChangedAttribute(attrName);
			break;
		}	
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

bool AttributeEditor::QuitRequested() {
	BMessage message(kAttributeEditClose);
	fListener.SendMessage(&message);
	return true;
}

void AttributeEditor::SaveActiveAttribute()
{
	// No quick way to get active control :(
	for (int32 index = 0; index < fAttributes.CountItems(); ++index) {
		BTextControl* control = fAttributes.ItemAt(index)->textControl;
		if(strcmp(control->Text(), fAttributes.ItemAt(index)->stringValue.String()))
		{
			SaveChangedAttribute(fAttributes.ItemAt(index)->name);
			return;
		}
	}

}

void AttributeEditor::SaveChangedAttribute(BString attrName)
{
	BString value;
		
	for (int32 i = 0; i < fAttributes.CountItems(); i++)
	{
		if (fAttributes.ItemAt(i)->name == attrName) 
		{
//			const entry_ref *ref = &entryRef;
			BFile * file = new BFile(activeRef, B_READ_WRITE);
			if (file->InitCheck() != B_NO_ERROR)
				return;
				
			BTextControl* control = fAttributes.ItemAt(i)->textControl;
			value = control->Text();
			fAttributes.ItemAt(i)->stringValue = value;

			if(fAttributes.ItemAt(i)->type == B_STRING_TYPE)
			{
				file->WriteAttr(attrName, B_STRING_TYPE, 0,	value, strlen(value) + 1);
			}
			if(fAttributes.ItemAt(i)->type == B_INT32_TYPE)
			{
				int32 int32value = (int32)atoi(value.String());
				file->WriteAttr(attrName, B_INT32_TYPE, 0,	&int32value, sizeof(int32));
			}
			if(fAttributes.ItemAt(i)->type == B_DOUBLE_TYPE)
			{
				double dvalue = (double)atof(value.String());
				file->WriteAttr(attrName, B_DOUBLE_TYPE, 0,	&dvalue, sizeof(double));
			}
			delete file;
			break;
		}
	}
}
