// example code for usage of listview elements

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_LISTVIEWTEST 1000452

#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"

struct TestData
{
	Int32 id;
	Char	name[20];
};

TestData testdata[] =
{
	{ 10, "Sharpen" },
	{ 12, "LensFlares" },
	{ 11, "Emboss" },
	{ 14, "Depth of Field" },
	{ 214, "AA" },

	{ 0, "" }
};

class ListViewDialog : public GeDialog
{
private:
	SimpleListView				listview1;
	SimpleListView				listview2;
	AutoAlloc<BaseSelect>	selection;
	Int32									counter2;

	void UpdateButtons(void);

public:
	ListViewDialog(void);
	virtual ~ListViewDialog(void);

	virtual Bool CreateLayout(void);
	virtual Bool InitValues(void);
	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);

};

enum
{
	IDC_OFFSET = 1001,
	IDC_ACCESS = 1002
};

ListViewDialog::ListViewDialog(void)
{
	counter2 = 0;
}

ListViewDialog::~ListViewDialog(void)
{
}

Bool ListViewDialog::CreateLayout(void)
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	res = LoadDialogResource(DLG_LISTVIEW, nullptr, 0);

	if (res)
	{
		listview1.AttachListView(this, GADGET_LISTVIEW1);
		listview2.AttachListView(this, GADGET_LISTVIEW2);
	}

	return res;
}

void ListViewDialog::UpdateButtons(void)
{
	if (!selection)
		return;

	Enable(GADGET_INSERT, listview1.GetSelection(selection));
	Enable(GADGET_REMOVE, listview2.GetSelection(selection));
}

Bool ListViewDialog::InitValues(void)
{
	// first call the parent instance
	if (!GeDialog::InitValues())
		return false;

	BaseContainer layout;
	BaseContainer data;
	Int32					i = 0;

	layout.SetInt32('name', LV_COLUMN_TEXT);
	//	layout.SetInt32('used',LV_COLUMN_CHECKBOX);
	listview1.SetLayout(1, layout);

	layout = BaseContainer();
	layout.SetInt32('chck', LV_COLUMN_CHECKBOX);
	layout.SetInt32('name', LV_COLUMN_TEXT);
	layout.SetInt32('bttn', LV_COLUMN_BUTTON);
	listview2.SetLayout(3, layout);

	data = BaseContainer();

	for (i = 0; testdata[i].id; i++)
	{
		data.SetString('name', testdata[i].name);
		//data.SetInt32('used',false);
		listview1.SetItem(testdata[i].id, data);
	}

	//data = BaseContainer();
	//for (i=0;testdata[i].id;i++)
	//{
	//	data.SetInt32('chck',true);
	//	data.SetString('name',testdata[i].name);
	//	data.SetString('bttn',"...");
	//	listview2.SetItem(testdata[i].id,data);
	//}

	listview1.DataChanged();
	listview2.DataChanged();

	UpdateButtons();

	return true;
}

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void GePrintF(const Char* format, ...)
{
	va_list arp;
	Char		buf[1024];

	va_start(arp, format);
	vsprintf_safe(buf, sizeof(buf), format, arp);
	GePrint(buf);
	va_end(arp);
}

Bool ListViewDialog::Command(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case GADGET_LISTVIEW1:
		case GADGET_LISTVIEW2:
		{
			switch (msg.GetInt32(BFM_ACTION_VALUE))
			{
				case LV_SIMPLE_SELECTIONCHANGED:
					GePrintF("Selection changed, id: %d, val: %p ", msg.GetInt32(LV_SIMPLE_ITEM_ID), msg.GetVoid(LV_SIMPLE_DATA));
					break;

				case LV_SIMPLE_CHECKBOXCHANGED:
					GePrintF("CheckBox changed, id: %d, col: %d, val: %p", msg.GetInt32(LV_SIMPLE_ITEM_ID), msg.GetInt32(LV_SIMPLE_COL_ID), msg.GetVoid(LV_SIMPLE_DATA));
					break;

				case LV_SIMPLE_FOCUSITEM:
					GePrintF("Focus set id: %d, col: %d", msg.GetInt32(LV_SIMPLE_ITEM_ID), msg.GetInt32(LV_SIMPLE_COL_ID));
					break;

				case LV_SIMPLE_BUTTONCLICK:
					GePrintF("Button clicked id: %d, col: %d", msg.GetInt32(LV_SIMPLE_ITEM_ID), msg.GetInt32(LV_SIMPLE_COL_ID));
					break;
			}
		}
			UpdateButtons();
			break;

		//		case GADGET_LISTVIEW2:
		//			break;

		case GADGET_INSERT:
		{
			AutoAlloc<BaseSelect> s2;
			if (selection && s2)
			{
				// TEST
				Int32					i, id2, count = listview1.GetItemCount();
				BaseContainer test;

				for (i = 0; i < count; i++)
				{
					listview1.GetItemLine(i, &id2, &test);
				}
				// TEST

				if (!listview1.GetSelection(selection))
				{
					GePrint("No Selection");
				}
				else
				{
					Int32	 a, b;
					String str;
					for (i = 0; selection->GetRange(i, LIMIT<Int32>::MAX, &a, &b); i++)
					{
						if (a == b)
							str += String::IntToString(a) + " ";
						else
							str += String::IntToString(a) + "-" + String::IntToString(b) + " ";
					}
					//				str.Delete(str.GetLength()-1,1);
					GePrint("Selection: " + str);

					BaseContainer data;
					for (i = 0; testdata[i].id; i++)
					{
						if (selection->IsSelected(testdata[i].id))
						{
							data.SetInt32('chck', true);
							data.SetString('name', testdata[i].name);
							data.SetString('bttn', "...");
							selection->Select(counter2);
							listview2.SetItem(counter2++, data);
						}
					}
					listview2.SetSelection(selection);
					listview2.DataChanged();
				}
			}
		}
			UpdateButtons();
			break;

		case GADGET_REMOVE:
		{
			if (selection && listview2.GetSelection(selection))
			{
				Int32 i, a, b;
				for (i = 0; selection->GetRange(i, LIMIT<Int32>::MAX, &a, &b); i++)
				{
					for (; a <= b; a++)
					{
						listview2.RemoveItem(a);
					}
				}
				listview2.DataChanged();
			}
		}
			UpdateButtons();
			break;
	}
	return true;
}


Int32 ListViewDialog::Message(const BaseContainer& msg, BaseContainer& result)
{
	//	switch (msg.GetId())
	{
	}
	return GeDialog::Message(msg, result);
}

class ListViewTest : public CommandData
{
private:
	ListViewDialog dlg;

public:
	virtual Bool Execute(BaseDocument* doc);
	virtual Int32 GetState(BaseDocument* doc);
	virtual Bool RestoreLayout(void* secret);
};

Int32 ListViewTest::GetState(BaseDocument* doc)
{
	return CMD_ENABLED;
}

Bool ListViewTest::Execute(BaseDocument* doc)
{
	return dlg.Open(DLG_TYPE_ASYNC, ID_LISTVIEWTEST, -1, -1);
}

Bool ListViewTest::RestoreLayout(void* secret)
{
	return dlg.RestoreLayout(ID_LISTVIEWTEST, 0, secret);
}

Bool RegisterListView(void)
{
	return RegisterCommandPlugin(ID_LISTVIEWTEST, GeLoadString(IDS_LISTVIEW), 0, nullptr, String(), NewObjClear(ListViewTest));
}
