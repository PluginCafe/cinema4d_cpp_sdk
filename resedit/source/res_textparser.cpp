/*********************************************************************\
	File name        : res_textparser.cpp
	Description      :
	Created at       : 16.08.01, @ 18:16:55
	Created by       : MAXON
	Modified by      : Thomas Kunert
\*********************************************************************/

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdafx.h"
#include "globals.h"

#include "res_textparser.h"
//#include "ge_cstr.h"

#include <stdio.h>
#include <string.h>

void StrToReal(char* pChString, Float &val)
{
	Float32 tmp=0.0;
	sscanf(pChString, "%f", &tmp);
	val=(Float)tmp;
}

void StrToVal(char* pChString, Int32 &val)
{
	int n;
	sscanf(pChString, "%d", &n);
	val = n;
}

Int32 Compare(char* pCh1, const char* pCh2)
{
	return strcmp(pCh1, pCh2);
}



#define MAXREADBUF	(2000)

#define CHR_CR			(0xa)
#define CHR_LF			(0xd)
#define CHR_SPC			' '
#define CHR_TAB			(0x9)
#define CHR_COMMENT1	'/'
#define CHR_COMMENT2	'#'






ASCIIParser::ASCIIParser()
{
	Int32 t;

	m_bUseagain = false;
	m_bFreebuf = false;
	m_bUsesymbolagain = false;

	m_pchReadbuffer = bNewDeprecatedUseArraysInstead<Char>(MAXREADBUF + 2);
	m_pchReadpos=0;
	m_lFilesize=0;
	m_lTotalsize=0;
	m_lReadrest=0;

	m_chLastchar=0;
	m_chPrevchar=0;
	m_lLine=1;

	ClearError();

	for (t=0;t<256;t++) m_pchIdenttab[t]=false;
	for (t='a';t<='z';t++) m_pchIdenttab[t]=true;
	for (t='A';t<='Z';t++) m_pchIdenttab[t]=true;
	for (t='0';t<='9';t++) m_pchIdenttab[t]=true;
	m_pchIdenttab[(UChar)'_']=true;

	m_pFile = BaseFile::Alloc();
}



ASCIIParser::~ASCIIParser()
{
	m_pFile->Close();
	if (m_bFreebuf && m_pchBuf) bDelete(m_pchBuf);
	bDelete(m_pchReadbuffer);

	BaseFile::Free(m_pFile);
}



Int32 ASCIIParser::Init(const Filename &name,Bool meckern,Bool _need_eol_)
{
	Int32 error=ERROR_NULL;

	m_filename = name;
	m_bMecker=meckern;
	m_bNeed_eol = _need_eol_;
	m_chLastchar=0;
	m_chPrevchar=0;

	m_pFile->Open(name,FILEOPEN_READ,m_bMecker?FILEDIALOG_ANY:FILEDIALOG_NONE);
	if (m_pFile->GetError()==FILEERROR_NONE)
	{
		m_lTotalsize=m_lFilesize=m_pFile->GetLength();
		if (m_lFilesize>0)
		{
			nextsym();
		}
		else { m_pFile->Close();error=ERROR_FILEOPEN; }
	}
	else error=ERROR_FILEOPEN;

	m_lReadpos = 0;
	return error;
}

void ASCIIParser::Close()
{
	m_pFile->Close();
}



Bool ASCIIParser::nextchar(Char *chr)
{
	if (!Error())
	{
		if (m_bUseagain) m_bUseagain=false;
		else
		{
			if (m_lReadrest<=0)
			{
				//SetProgress(win,(Float)(m_lTotalsize-m_lFilesize)/(Float)m_lTotalsize*PROZENT_ANTEIL_LADEN,progress);
				if (m_lFilesize>0)
				{
					m_lReadrest= MAXREADBUF<m_lFilesize ? MAXREADBUF : m_lFilesize;
					m_lFilesize-=m_lReadrest;
					m_pchReadpos=m_pchReadbuffer;
					if (!m_pFile->ReadBytes(m_pchReadbuffer,m_lReadrest))
						SetError(ERROR_READ);
				}
				else
					error=E_EOF;
			}
			if (m_lReadrest>0)
			{
				m_chLastchar=*m_pchReadpos++;
				m_lReadrest--;
			}

			if (m_chLastchar==CHR_CR)
			{
				if (m_chPrevchar==CHR_LF) m_chPrevchar = 0;
				else
				{
					m_lLine++;
					m_chPrevchar = m_chLastchar;
				}
			}
			else if (m_chLastchar==CHR_LF)
			{
				if (m_chPrevchar==CHR_CR) m_chPrevchar = 0;
				else
				{
					m_lLine++;
					m_chPrevchar = m_chLastchar;
				}
			}
			else m_chPrevchar = m_chLastchar;
		}
	}
	else
		m_chLastchar = 0;

	if (chr) *chr=m_chLastchar;
	m_lReadpos++;
	return Error() ? false : true;
};



void ASCIIParser::skipspaces()
{
	Char c;
	Bool linecomment=false;
	while (true)
	{
		if (!nextchar(&c))	return;

		if (c==CHR_CR || c==CHR_LF)
		{
			if (m_bNeed_eol) break;
			linecomment=false;
		}
		else if (c == CHR_COMMENT1 || c == CHR_COMMENT2) linecomment=true;
		else if (linecomment || c==CHR_SPC || c==CHR_TAB) ;
		else break;
	}
	m_bUseagain = true;
};



void ASCIIParser::skiplinetoend()
{
	Char c;
	Bool linecomment=true;

	while (true)
	{
		if (!nextchar(&c))	return;

		if (c==CHR_CR || c==CHR_LF)
		{
			if (m_bNeed_eol) break;
			linecomment=false;
		}
		else if (linecomment || c==CHR_SPC || c==CHR_TAB) ;
		else break;
	}
	m_bUseagain = true;
}


void ASCIIParser::rewindsym(void)
{
	m_bUsesymbolagain = true;
}


SYMBOL ASCIIParser::nextsym(Bool getpoint)
{
	Char *txt;

	if (m_bUsesymbolagain)
	{
		m_bUsesymbolagain=false;
		return cur_symbol;
	}


	if (m_bFreebuf && m_pchBuf) { m_bFreebuf=false; bDelete(m_pchBuf); }

	if (m_pchBuf==m_pchBuf2) m_pchBuf=m_pchBuf1;
	else m_pchBuf=m_pchBuf2;
	txt=m_pchBuf;

	cur_symbol=SYM_NULL;
	skipspaces();

	if (!getpoint && ((m_chLastchar>='0' && m_chLastchar<='9') || m_chLastchar=='.' || m_chLastchar=='-'))
	{
		// hier folgt nun eine Zahl
		while (!Error())
		{
			if (!nextchar(txt)) return SYM_NULL; // Lesefehler
			if (!(*txt>='0' && *txt<='9') && !(*txt>='a' && *txt<='f') && !(*txt>='A' && *txt<='F') && *txt!='.' && *txt!='e' && *txt!='E' && *txt!='-' && *txt!='+' && *txt!='x' && *txt!='X')
			{
				// ende des bezeichners
				cur_symbol=SYM_NUMBER;
				m_bUseagain=true;
				break;
			}
			txt++;
		}
		*txt = 0;
	}
	else if (m_pchIdenttab[(UChar)m_chLastchar])
	{
		// bezeichner entdeckt
		while (!Error())
		{
			if (!nextchar(txt)) return SYM_NULL; // Lesefehler
			if (!m_pchIdenttab[(UChar)*txt])
			{
				// ende des bezeichners
				*txt=0;

				cur_symbol = CompareIdent(m_pchBuf);
				if (cur_symbol==SYM_NULL) cur_symbol=SYM_IDENT;

				m_bUseagain=true;
				break;
			}
			txt++;
		}
		*txt = 0;
	}
	else if (m_chLastchar=='"' || m_chLastchar=='\'')
	{
		Char cmpchr = m_chLastchar;
		Int32 stringlen = 50,pos = 0;
		Bool usenext=false;
		m_pchBuf = bNewDeprecatedUseArraysInstead<Char>(stringlen + 2);
		m_bFreebuf=true;
		txt=m_pchBuf;
		m_bUseagain = false;

		while (!Error())
		{
			if (!nextchar(&txt[pos])) return SYM_NULL; // Lesefehler
			if (!usenext && txt[pos]==cmpchr)
			{
				// ende des bezeichners
				cur_symbol = cmpchr=='"' ? SYM_STRING : SYM_STRINGCHR;
				txt[pos]=0;
				break;
			}
			else if (cmpchr=='"' && txt[pos]=='$')
			{
				Char chr;
				if (!nextchar(&chr)) return SYM_NULL; // Lesefehler
				m_bUseagain=true;

				if (chr==cmpchr)
				{
					if (!usenext)
					{
						usenext=true;
						continue;
					}
				}
			}
			usenext=false;
			pos++;
			if (pos >= stringlen)
			{
				Char *newstring = bNewDeprecatedUseArraysInstead<Char>(2 * stringlen + 2);
				if (newstring)
				{
					CopyMemType(m_pchBuf, newstring, stringlen);
					bDelete(m_pchBuf);
					txt = m_pchBuf = newstring;
					stringlen = 2 * stringlen;
				}
			}
		}
	}
	else
	{
		// operator entdeckt
		if (!nextchar(txt)) return SYM_NULL; // Lesefehler

		cur_symbol = CompareOperator(*txt);
		if (cur_symbol==SYM_NULL) cur_symbol=SYM_OPERATOR_ILLEGAL;

		txt++;
		*txt = 0;
	}
	return cur_symbol;
};



Bool ASCIIParser::GetFloat(Float &val)
{
	if (cur_symbol==SYM_NUMBER)
	{
		StrToReal(m_pchBuf,val);
		return true;
	}
	return false;
};



Bool ASCIIParser::GetInt32(Int32 &val)
{
	if (cur_symbol==SYM_NUMBER)
	{
		StrToVal(m_pchBuf,val);
		return true;
	}
	return false;
};



SYMBOL ASCIIParser::CompareIdent(Char *m_pchBuf)
{
	//if (!Compare(m_pchBuf,"IDENT")) return SYM_IDENT;
	return SYM_NULL;
}


SYMBOL ASCIIParser::CompareOperator(Char c)
{
	if (c=='{') return SYM_OPERATOR_GESCHWEIFTAUF;
	else if (c=='}') return SYM_OPERATOR_GESCHWEIFTZU;
	else if (c=='=') return SYM_OPERATOR_EQUAL;
	else if (c==',') return SYM_OPERATOR_KOMMA;
	else if (c==';') return SYM_OPERATOR_SEMIKOLON;
	else if (c=='+') return SYM_OPERATOR_PLUS;
	else if (c=='~') return SYM_OPERATOR_TILDE;
	else if (c==CHR_CR) return SYM_OPERATOR_NEWLINE;
	else if (c==CHR_LF) return SYM_OPERATOR_NEWLINE;
	else if (c=='[') return SYM_OPERATOR_ECKIGAUF;
	else if (c==']') return SYM_OPERATOR_ECKIGZU;

	return SYM_OPERATOR_ILLEGAL;
}















