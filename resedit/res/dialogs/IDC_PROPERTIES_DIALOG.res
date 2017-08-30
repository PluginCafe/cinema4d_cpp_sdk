// C4D-DialogResource
DIALOG IDC_PROPERTIES_DIALOG
{
  NAME IDS_DIALOG; SCALE_V; SCALE_H; 
  
  GROUP IDC_STATIC
  {
    SCALE_V; SCALE_H; 
    BORDERSIZE 4, 4, 4, 4; 
    COLUMNS 1;
    SPACE 4, 4;
    
    GROUP IDC_STATIC
    {
      NAME IDS_STATIC1; ALIGN_TOP; FIT_H; 
      BORDERSIZE 0, 0, 0, 0; 
      COLUMNS 2;
      SPACE 4, 4;
      
      GROUP IDC_STATIC
      {
        NAME IDS_STATIC4; ALIGN_TOP; SCALE_H; 
        BORDERSTYLE BORDER_GROUP_IN; BORDERSIZE 4, 4, 4, 4; 
        COLUMNS 2;
        SPACE 4, 4;
        
        STATICTEXT IDC_STATIC { NAME IDS_STATIC8; CENTER_V; ALIGN_LEFT; }
        EDITTEXT IDC_ID_EDIT
        { CENTER_V; SCALE_H; SIZE 200, 0; }
        STATICTEXT IDC_STATIC { NAME IDS_STATIC6; CENTER_V; ALIGN_LEFT; }
        EDITTEXT IDC_NAME_EDIT
        { CENTER_V; FIT_H; SIZE 70, 0; }
        STATICTEXT IDC_STATIC { NAME IDS_STATIC7; CENTER_V; ALIGN_LEFT; }
        STATICTEXT IDC_NAME_ID_STAT { NAME IDS_STATIC5; CENTER_V; FIT_H; }
      }
      GROUP IDC_STATIC
      {
        NAME IDS_STATIC77; ALIGN_TOP; ALIGN_LEFT; 
        BORDERSIZE 0, 0, 0, 0; 
        COLUMNS 1;
        SPACE 4, 4;
        
        GROUP IDC_STATIC
        {
          NAME IDS_STATIC3; SCALE_V; ALIGN_LEFT; 
          BORDERSTYLE BORDER_GROUP_IN; BORDERSIZE 4, 4, 4, 4; 
          COLUMNS 4;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC9; CENTER_V; ALIGN_LEFT; }
          COMBOBOX IDC_HORZ_ALIGN_COMBO
          {
            ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
            CHILDS
            {
              0, IDS_ALIGN_CENTER; 
              1, IDS_ALIGN_LEFT; 
              2, IDS_ALIGN_RIGHT; 
              3, IDS_ALIGN_FIT; 
              5, IDS_ALIGN_SCALEFIT; 
            }
          }
          STATICTEXT IDC_STATIC { NAME IDS_STATIC10; CENTER_V; ALIGN_LEFT; }
          EDITNUMBERARROWS IDC_WIDTH_EDIT
          { CENTER_V; CENTER_H; SIZE 70, 0; }
          STATICTEXT IDC_STATIC { NAME IDS_STATIC11; CENTER_V; ALIGN_LEFT; }
          COMBOBOX IDC_VERT_ALIGN_COMBO
          {
            ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
            CHILDS
            {
              0, IDS_ALIGN_CENTER; 
              1, IDS_ALIGN_TOP; 
              2, IDS_ALIGN_BOTTOM; 
              3, IDS_ALIGN_FIT; 
              5, IDS_ALIGN_SCALEFIT; 
            }
          }
          STATICTEXT IDC_STATIC { NAME IDS_STATIC12; CENTER_V; ALIGN_LEFT; }
          EDITNUMBERARROWS IDC_HEIGHT_EDIT
          { CENTER_V; CENTER_H; SIZE 70, 0; }
        }
        BUTTON IDC_UPDATE_BTN { NAME IDS_BUTTON; ALIGN_TOP; ALIGN_LEFT; }
      }
    }
    GROUP IDC_STATIC
    {
      NAME IDS_STATIC2; SCALE_V; SCALE_H; 
      BORDERSTYLE BORDER_GROUP_IN; BORDERSIZE 4, 4, 4, 4; 
      COLUMNS 1;
      SPACE 4, 4;
      
      TAB IDC_MAIN_TAB
      {
        NAME IDS_STATIC13; SCALE_V; SCALE_H; 
        SELECTION_NONE; 
        
        GROUP IDC_DIALOG_SETTINGS_TAB
        {
          NAME IDS_STATIC79; ALIGN_TOP; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC80; CENTER_V; ALIGN_LEFT; }
        }
        GROUP IDC_STATIC_SETTINGS_TAB
        {
          NAME IDS_STATIC14; SCALE_V; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC15; ALIGN_TOP; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 2;
            SPACE 4, 4;
            
            STATICTEXT IDC_STATIC { NAME IDS_STATIC16; CENTER_V; ALIGN_LEFT; }
            COMBOBOX IDC_BORDER_COMBO
            {
              ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
              CHILDS
              {
                0, IDS_BORDER_NONE; 
                1, IDS_BORDER_THIN_IN; 
                2, IDS_BORDER_THIN_OUT; 
                3, IDS_BORDER_IN; 
                4, IDS_BORDER_OUT; 
                5, IDS_BORDER_GROUP_IN; 
                6, IDS_BORDER_GROUP_OUT; 
                7, IDS_BORDER_OUT2; 
                8, IDS_BORDER_OUT3; 
                9, IDS_BORDER_BLACK; 
                10, IDS_BORDER_ACTIVE_1; 
                11, IDS_BORDER_ACTIVE_2; 
                12, IDS_BORDER_GROUP_TOP; 
              }
            }
          }
        }
        GROUP IDC_EDIT_SETTINGS_TAB
        {
          NAME IDS_STATIC17; FIT_V; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 2;
          SPACE 4, 4;
          
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC18; ALIGN_TOP; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 2;
            SPACE 4, 4;
            
            STATICTEXT IDC_STATIC { NAME IDS_STATIC20; CENTER_V; ALIGN_LEFT; }
            COMBOBOX IDC_TYPE_SELECT_COMBO
            {
              ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
              CHILDS
              {
                0, IDS_TEXT_TYPE_NUMBER; 
                1, IDS_TEXT_TYPE_TEXT; 
              }
            }
          }
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC19; ALIGN_TOP; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 2;
            SPACE 4, 4;
            
            GROUP IDC_STATIC
            {
              NAME IDS_STATIC21; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 1;
              SPACE 4, 4;
              
              CHECKBOX IDC_HAS_SLIDER_CHK { NAME IDS_CHECK; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_HAS_ARROWS_CHK { NAME IDS_CHECK1; ALIGN_TOP; ALIGN_LEFT;  }
            }
            GROUP IDC_STATIC
            {
              NAME IDS_STATIC22; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 2;
              SPACE 4, 4;
              
              CHECKBOX IDC_TEXT_MULTILINE_CHK { NAME IDS_CHECK2; ALIGN_TOP; ALIGN_LEFT;  }
              GROUP IDC_STATIC
              {
                NAME IDS_STATIC23; ALIGN_TOP; ALIGN_LEFT; 
                BORDERSIZE 4, 0, 4, 4; 
                COLUMNS 1;
                SPACE 4, 4;
                
                CHECKBOX IDC_TEXT_MULTILINE_MONOSPACED_CHK { NAME IDS_CHECK3; ALIGN_TOP; ALIGN_LEFT;  }
                CHECKBOX IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK { NAME IDS_CHECK4; ALIGN_TOP; ALIGN_LEFT;  }
                CHECKBOX IDC_TEXT_MULTILINE_STATUSBAR_CHK { NAME IDS_CHECK5; ALIGN_TOP; ALIGN_LEFT;  }
                CHECKBOX IDC_TEXT_MULTILINE_WORDWRAP_CHK { NAME IDS_CHECK_WORDWRAP; ALIGN_TOP; ALIGN_LEFT;  }
                CHECKBOX IDC_TEXT_MULTILINE_PYTHON_CHK { NAME IDS_CHECK_PYTHON; ALIGN_TOP; ALIGN_LEFT;  }
              }
            }
          }
        }
        GROUP IDC_BUTTON_SETTINGS_TAB
        {
          NAME IDS_STATIC24; FIT_V; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC25; CENTER_V; ALIGN_LEFT; }
        }
        GROUP IDC_ARROW_SETTINGS_TAB
        {
          NAME IDS_STATIC26; SCALE_V; SCALE_H; 
          BORDERSIZE 4, 4, 4, 4; 
          COLUMNS 3;
          SPACE 4, 4;
          
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC27; SCALE_V; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 2;
            SPACE 4, 4;
            
            CHECKBOX IDC_IS_POPUP_CHK { NAME IDS_CHECK6; ALIGN_TOP; ALIGN_LEFT;  }
            STATICTEXT IDC_STATIC { NAME IDS_STATIC78; CENTER_V; ALIGN_LEFT; }
            STATICTEXT IDC_STATIC { NAME IDS_STATIC28; CENTER_V; ALIGN_LEFT; }
            COMBOBOX IDS_ARROW_TYPE_COMBO
            {
              ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
              CHILDS
              {
                0, IDS_LEFT; 
                1, IDS_RIGHT; 
                2, IDS_UP; 
                3, IDS_DOWN; 
              }
            }
            STATICTEXT IDC_STATIC { NAME IDS_STATIC29; CENTER_V; ALIGN_LEFT; }
            CHECKBOX IDS_ARROW_SMALL_CHK { NAME IDS_CHECK7; ALIGN_TOP; ALIGN_LEFT;  }
          }
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC30; SCALE_V; SCALE_H; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 1;
            SPACE 4, 4;
            
            STATICTEXT IDC_STATIC { NAME IDS_STATIC31; ALIGN_TOP; ALIGN_LEFT; }
            MULTILINEEDIT IDC_ARROW_CHILD_ITEMS
            { SCALE_V; ALIGN_LEFT; SIZE 300, 0; }
          }
        }
        GROUP IDC_CHECKBOX_SETTINGS_TAB
        {
          NAME IDS_STATIC32; FIT_V; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC33; CENTER_V; ALIGN_LEFT; }
        }
        GROUP IDC_RADIO_BTN_SETTINGS_TAB
        {
          NAME IDS_STATIC34; FIT_V; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          CHECKBOX IDC_RADIO_BTN_IS_TEXT_CHK { NAME IDS_CHECK8; ALIGN_TOP; ALIGN_LEFT;  }
        }
        GROUP IDC_LIST_VIEW_SETTINGS_TAB
        {
          NAME IDS_STATIC35; FIT_V; ALIGN_LEFT; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC36; ALIGN_TOP; ALIGN_LEFT; }
        }
        GROUP IDC_COMBO_BOX_SETTINGS_TAB
        {
          NAME IDS_STATIC37; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC1 { NAME IDS_STATIC38; ALIGN_TOP; ALIGN_LEFT; }
          MULTILINEEDIT IDC_COMBO_CHILD_ITEMS
          { SCALE_V; SCALE_H; SIZE 300, 0; }
        }
        GROUP IDC_COMBO_BUTTON_SETTINGS_TAB
        {
          NAME IDS_STATIC37; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC1 { NAME IDS_STATIC38; ALIGN_TOP; ALIGN_LEFT; }
          MULTILINEEDIT IDC_COMBOBUTTON_CHILD_ITEMS
          { SCALE_V; SCALE_H; SIZE 300, 0; }
        }
        GROUP IDC_GROUP_SETTINGS_TAB
        {
          NAME IDS_STATIC39; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          TAB IDC_GROUP_TYPE_TAB
          {
            NAME IDS_STATIC41; ALIGN_TOP; ALIGN_LEFT; 
            SELECTION_CYCLE; 
            
            GROUP 0
            {
              NAME IDS_STATIC43; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 3;
              SPACE 4, 4;
              
              STATICTEXT IDC_STATIC { NAME IDS_STATIC47; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_GROUP_COLS_EDIT
              { ALIGN_BOTTOM; ALIGN_LEFT; SIZE 70, 0; }
              CHECKBOX IDC_GROUP_EQUALCOLS_CHK { NAME IDS_CHECK9; CENTER_V; ALIGN_LEFT;  }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC48; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_GROUP_ROWS_EDIT
              { CENTER_V; ALIGN_LEFT; SIZE 70, 0; }
              CHECKBOX IDC_GROUP_EQUALROWS_CHK { NAME IDS_CHECK10; CENTER_V; ALIGN_LEFT;  }
              CHECKBOX IDC_GROUP_ALLOW_WEIGHTS_CHK { NAME IDS_CHECK28; ALIGN_TOP; ALIGN_LEFT;  }
            }
            GROUP 1
            {
              NAME IDS_STATIC44; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 2;
              SPACE 4, 4;
              
              STATICTEXT IDC_STATIC { NAME IDS_STATIC49; CENTER_V; ALIGN_LEFT; }
              COMBOBOX IDC_TAB_GROUP_TYPE_COMBO
              {
                ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
                CHILDS
                {
                  0, IDS_TAB_TYPE_TABS; 
                  1, IDS_TAB_TYPE_NO_SELECT; 
                  2, IDS_TAB_TYPE_CYCLE; 
                  3, IDS_TAB_TYPE_RADIO; 
                  4, IDS_TAB_TYPE_VLTABS; 
                  5, IDS_TAB_TYPE_VRTABS; 
                }
              }
            }
            GROUP 2
            {
              NAME IDS_STATIC45; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              ROWS 3;
              SPACE 4, 4;
              
              CHECKBOX IDC_SCROLL_GROUP_HORZ_CHK { NAME IDS_CHECK15; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_VERT_CHK { NAME IDS_CHECK11; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_BORDERIN_CHK { NAME IDS_CHECK14; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_AUTOHORIZ_CHK { NAME IDS_CHECK16; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_AUTOVERT_CHK { NAME IDS_CHECK12; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_STATUSBAR_CHK { NAME IDS_CHECK17; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_LEFT_CHK { NAME IDS_CHECK18; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_SCROLL_GROUP_NOBLIT_CHK { NAME IDS_CHECK13; ALIGN_TOP; ALIGN_LEFT;  }
            }
            GROUP 3
            {
              NAME IDS_STATIC46; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 2;
              SPACE 4, 4;
              
              STATICTEXT IDC_STATIC { NAME IDS_STATIC50; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_GROUP_COLS_EDIT1
              { ALIGN_TOP; ALIGN_LEFT; SIZE 70, 0; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC51; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_GROUP_ROWS_EDIT1
              { ALIGN_TOP; ALIGN_LEFT; SIZE 70, 0; }
            }
          }
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC40; ALIGN_TOP; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            ROWS 3;
            SPACE 4, 4;
            
            GROUP IDC_STATIC
            {
              NAME IDS_STATIC52; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 4;
              SPACE 4, 4;
              
              CHECKBOX IDC_SIMPLE_GROUP_BORDER_CHK { NAME IDS_CHECK19; CENTER_V; ALIGN_LEFT;  }
              COMBOBOX IDC_SIMPLE_GROUP_BORDER_COMBO
              {
                CENTER_V; ALIGN_LEFT; SIZE 150, 0; 
                CHILDS
                {
                  0, IDS_BORDER_NONE1; 
                  1, IDS_BORDER_THIN_IN1; 
                  2, IDS_BORDER_THIN_OUT1; 
                  3, IDS_BORDER_IN1; 
                  4, IDS_BORDER_OUT4; 
                  5, IDS_BORDER_GROUP_IN1; 
                  6, IDS_BORDER_GROUP_OUT1; 
                  7, IDS_BORDER_OUT5; 
                  8, IDS_BORDER_OUT6; 
                  9, IDS_BORDER_BLACK1; 
                  10, IDS_BORDER_ACTIVE_3; 
                  11, IDS_BORDER_ACTIVE_4; 
                  12, IDS_BORDER_GROUP_TOP1; 
                }
              }
              CHECKBOX IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK { NAME IDS_CHECK20; ALIGN_TOP; ALIGN_LEFT;  }
              CHECKBOX IDC_HAS_BORDER_CHECKBOX { NAME IDS_CHECK21; ALIGN_TOP; ALIGN_LEFT;  }
            }
            GROUP IDC_STATIC
            {
              NAME IDS_STATIC55; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              COLUMNS 9;
              SPACE 4, 4;
              
              STATICTEXT IDC_STATIC { NAME IDS_STATIC56; CENTER_V; ALIGN_LEFT; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC57; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_BW_LEFT_EDIT
              { ALIGN_TOP; ALIGN_LEFT; SIZE 30, 0; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC58; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_BW_RIGHT_EDIT
              { ALIGN_TOP; ALIGN_LEFT; SIZE 30, 0; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC59; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_BW_TOP_EDIT
              { ALIGN_TOP; ALIGN_LEFT; SIZE 30, 0; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC60; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_BW_BOTTOM_EDIT
              { ALIGN_TOP; ALIGN_LEFT; SIZE 30, 0; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC61; CENTER_V; ALIGN_LEFT; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC62; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_SPACE_X_EDIT
              { ALIGN_TOP; ALIGN_LEFT; SIZE 30, 0; }
              STATICTEXT IDC_STATIC { NAME IDS_STATIC63; CENTER_V; ALIGN_LEFT; }
              EDITNUMBERARROWS IDC_SPACE_Y_EDIT
              { ALIGN_TOP; ALIGN_LEFT; SIZE 30, 0; }
            }
          }
        }
        GROUP IDC_DIALOG_GROUP_SETTINGS_TAB
        {
          NAME IDS_STATIC64; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          CHECKBOX IDC_DLG_GROUP_OK_CHK { NAME IDS_CHECK22; ALIGN_TOP; ALIGN_LEFT;  }
          CHECKBOX IDC_DLG_GROUP_CANCEL_CHK { NAME IDS_CHECK23; ALIGN_TOP; ALIGN_LEFT;  }
          CHECKBOX IDC_DLG_GROUP_HELP_CHK { NAME IDS_CHECK24; ALIGN_TOP; ALIGN_LEFT;  }
        }
        GROUP IDC_SEPARATOR_SETTINGS_TAB
        {
          NAME IDS_STATIC65; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 2;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC66; CENTER_V; ALIGN_LEFT; }
          COMBOBOX IDC_SEPARATOR_TYPE_COMBO
          {
            CENTER_V; ALIGN_LEFT; SIZE 150, 0; 
            CHILDS
            {
              0, IDS_HORIZ; 
              1, IDS_VERT; 
            }
          }
        }
        GROUP IDC_USER_AREA_SETTINGS_TAB
        {
          NAME IDS_STATIC67; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC68; CENTER_V; ALIGN_LEFT; }
        }
        GROUP IDC_SLIDER_SETTINGS_TAB
        {
          NAME IDS_STATIC69; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC { NAME IDS_STATIC70; CENTER_V; ALIGN_LEFT; }
        }
        GROUP IDC_COLOR_SETTINGS_TAB
        {
          NAME IDS_STATIC71; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 2;
          SPACE 4, 4;
          
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC73; ALIGN_TOP; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 2;
            SPACE 4, 4;
            
            STATICTEXT IDC_STATIC { NAME IDS_STATIC72; CENTER_V; ALIGN_LEFT; }
            COMBOBOX IDC_COLOR_TYPE_COMBO
            {
              ALIGN_TOP; ALIGN_LEFT; SIZE 150, 0; 
              CHILDS
              {
                0, IDS_COLOR_TYPE_FIELD; 
                1, IDS_COLOR_TYPE_CHOOSER; 
              }
            }
          }
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC74; ALIGN_TOP; ALIGN_LEFT; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 1;
            SPACE 4, 4;
            
            CHECKBOX IDC_COLORFIELD_NO_BRIGHTNESS_CHK { NAME IDS_CHECK26; ALIGN_TOP; ALIGN_LEFT;  }
            CHECKBOX IDC_COLORFIELD_NO_COLOR_CHK { NAME IDS_CHECK27; ALIGN_TOP; ALIGN_LEFT;  }
          }
        }
        GROUP IDC_SUB_DLG_SETTINGS_TAB
        {
          NAME IDS_STATIC75; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          STATICTEXT IDC_STATIC3 { NAME IDS_STATIC76; CENTER_V; ALIGN_LEFT; }
        }
        GROUP IDC_CUSTOM_ELEMENT_SETTINGS_TAB
        {
          NAME IDS_STATIC81; SCALE_V; SCALE_H; 
          BORDERSIZE 0, 0, 0, 0; 
          COLUMNS 1;
          SPACE 4, 4;
          
          GROUP IDC_STATIC
          {
            NAME IDS_STATIC83; SCALE_V; SCALE_H; 
            BORDERSIZE 0, 0, 0, 0; 
            COLUMNS 2;
            EQUAL_ROWS; 
            SPACE 4, 4;
            
            GROUP IDC_STATIC
            {
              NAME IDS_STATIC87; ALIGN_TOP; ALIGN_LEFT; 
              BORDERSIZE 0, 0, 0, 0; 
              ROWS 1;
              SPACE 4, 4;
              
              STATICTEXT IDC_STATIC { NAME IDS_STATIC82; CENTER_V; ALIGN_LEFT; }
              POPUPBUTTON IDC_CUSTOM_RESET_ARROW
              {
                ALIGN_TOP; ALIGN_LEFT; 
                CHILDS
                {
                  IDS_CUSTOM_RESET, IDS_CUSTOM_RESET; 
                  IDS_CUSTOM_RESET_ALL, IDS_CUSTOM_RESET_ALL; 
                }
              }
            }
            STATICTEXT IDC_STATIC { NAME IDS_STATIC85; CENTER_V; ALIGN_LEFT; }
            LISTVIEW IDC_CUSTOM_GUI_LIST { SCALE_V; SCALE_H; SIZE 150, 50; }
            SCROLLGROUP IDC_STATIC
            {
              NAME IDS_STATIC84; SCALE_V; SCALE_H; 
              NAME IDS_STATIC84; SCALE_V; SCALE_H; SCROLL_V; SCROLL_H; 
              
              SUBDIALOG IDC_CUSTOM_ELEMENT_SUBDLG { SCALE_V; SCALE_H; }
            }
          }
        }
      }
    }
  }
}