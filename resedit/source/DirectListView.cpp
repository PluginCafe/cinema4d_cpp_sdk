/*********************************************************************\
	File name        : DirectListView.cpp
	Description      :
	Created at       : 27.11.01, @ 17:26:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/


#include "stdafx.h"
#include "globals.h"
#include "DirectListView.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
#if 0
CDirectListView::CDirectListView()
{

}

CDirectListView::~CDirectListView()
{

}

void CDirectListView::LvCallBack(Int32 &res_type,void *&result,void *secret,Int32 cmd,Int32 line,Int32 col,void *data1)
{
	switch (cmd)
	{
		case LV_INITCACHE:
			//if (dlg) dlg->iGetData();
			//sbs = dlg->sbs;
			return;

		case LV_DESTROYLISTVIEW:
			//sbs = nullptr;
			return;

		case LV_SIMPLE_DOUBLECLICK:
		case LV_SIMPLE_SELECTIONCHANGED:
		case LV_SIMPLE_CHECKBOXCHANGED:
		case LV_SIMPLE_FOCUSITEM:
		case LV_SIMPLE_BUTTONCLICK:
			{
				BaseContainer m(BFM_ACTION);
				m.SetInt32(BFM_ACTION_ID,GetId());
				m.SetInt32(BFM_ACTION_VALUE,cmd);
				m.SetInt32(LV_SIMPLE_ITEM_ID,line);
				m.SetInt32(LV_SIMPLE_COL_ID ,col);
				m.SetInt32(LV_SIMPLE_DATA, Int32(data1));
				SendParentMessage(m);
			}
			return;

		case LV_GETLINECOUNT:
			{
				Int32 num = 0;
				/*if (sbs)
				{
					switch (m_mode)
					{
						case LIST_SELECTION:
							{
								SelectionSet *set = (SelectionSet*)sbs->selectionsets.GetFirst();
								for (; set; set = set->GetNext())
								{
									num++;
								}
							}
							break;

						case LIST_SPRINGS:
							num = sbs->scnt;
							break;
					}
				}*/
				ReturnLong(num);
			}
			break;

		case LV_GETCOLUMNCOUNT:
			{
				switch (m_mode)
				{
					case LIST_SELECTION: ReturnLong(1);
					case LIST_SPRINGS:   ReturnLong(2);
				}
			}
			ReturnLong(0);
			break;

		case LV_GETCOLSPACE:			ReturnLong(4); // horiz. space in pixel
		case LV_GETLINESPACE:			ReturnLong(0); // vert. space in pixel
		case LV_GETCOLUMTYPE:			ReturnLong(LV_COLUMN_TEXT);

		case LV_GETCOLUMDATA:
			{
				if (line==-999)
				{
					String txt;
					Int32 n=0;
					if (col==1)
					{
						txt = String("188 %");
					}
					else if (col==0)
					{
						/*for (; set; set = set->GetNext())
						{
							n = MMax(n,set->GetName().GetLength());
						}	*/
						txt = String(n,'A');
					}
					ReturnString(txt);
				}

				if (col == 0) ReturnString(String::IntToString(line));
				/*if (col == 1 && sbs)
				{
					Bool first = true;
					String str;
					SelectionSet *set = (SelectionSet*)sbs->selectionsets.GetFirst();
					for (; set; set = set->GetNext())
					{
						if (set->Get()->IsSelected(line))
						{
							if (!first) str += ", ";
							str += set->GetName();
							first = false;
						}
					}
					ReturnString(str);
				}*/
				ReturnString(String());
			}
			break;

		case LV_GETLINESELECTED:
			ReturnLong(0/*sbs->selection->IsSelected(line)*/);
			break;

		case LV_LMOUSEDOWN:
			{
				MouseDownInfo mouseinfo;
				if (ExtractMouseInfo(secret,mouseinfo,sizeof(mouseinfo)))
				{
					BaseContainer m(BFM_ACTION);
					m.SetInt32(BFM_ACTION_ID,GetId());
					m.SetInt32(BFM_ACTION_VALUE,cmd);
					m.SetInt32(LV_SIMPLE_ITEM_ID,line);
					m.SetInt32(LV_SIMPLE_COL_ID ,col);
					m.SetInt32(LV_SIMPLE_DATA, mouseinfo.msg.GetInt32(BFM_INPUT_QUALIFIER));
					SendParentMessage(m);
				}
			}
			ReturnNull();

		case LV_ACTION:
			{
			}
			ReturnNull();

		case LV_GETFIXEDLAYOUT:
			ReturnLong(true);
			break;

		case LV_GETCOLUMNWIDTH:
			LvSuperCall(res_type,result,secret,LV_GETCOLUMNWIDTH,-999,col);
			return;
	}

	LvSuperCall(res_type,result,secret,cmd,line,col);
}
#endif
