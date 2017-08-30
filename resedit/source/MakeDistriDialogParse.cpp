// MakeDistriDialog.cpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globals.h"
#include "MakeDistriDialog.h"
#include <stdio.h>
#include "DistriCompressProgr.h"
#include "customgui_lensglow.h"

#define MAXREADBUF	(2000)

#define CHR_CR			(0xa)
#define CHR_LF			(0xd)
#define CHR_SPC			' '
#define CHR_TAB			(0x9)

Int32 Compare(char* pCh1, const char* pCh2);

typedef enum parser_error
{
	ERROR_NULL,
	ERROR_READ,
	ERROR_WRONG_SYMBOL,
	ERROR_ILLEGAL_MF,

	ERROR_FILEOPEN,

	E_EOF=-0x1000

} parser_error;



enum SYMBOL
{
	SYM_NULL,

	SYM_IDENT,
	SYM_STRING,
	SYM_STRINGCHR,

	SYM_NUMBER,

	SYM_OPERATOR_NEWLINE,
	SYM_OPERATOR_GESCHWEIFTAUF,
	SYM_OPERATOR_GESCHWEIFTZU,
	SYM_OPERATOR_KLAMMERAUF,
	SYM_OPERATOR_KLAMMERZU,
	SYM_OPERATOR_COMMENT,
	SYM_OPERATOR_EQUAL,
	SYM_OPERATOR_KOMMA,
	SYM_OPERATOR_SEMIKOLON,
	SYM_OPERATOR_PLUS,
	SYM_OPERATOR_TILDE,
	SYM_OPERATOR_PREPROC,
	SYM_OPERATOR_MUL,

	SYM_OPERATOR_ILLEGAL
};



//--------------------------------------------------
class SymbolParser
{
		BaseFile*     m_pFile;
		Filename			m_filename;

		parser_error	error;
		SYMBOL				cur_symbol;
		Bool					cur_leadingspace,newline;
		UChar					m_pchIdenttab[256];
		Bool					m_bUseagain;
		Bool					m_bUsesymbolagain;

		Int32					m_lTotalsize;
		Int32					m_lFilesize;
		Int32          m_lReadpos;           // the current read position
		Char					*m_pchReadbuffer;			// MAXREADBUF grosser Puffer
		Char					*m_pchReadpos;					// aktuelle Puferposition
		Int32					m_lReadrest;					// verbleibende Zeichen im Puffer
		Char					m_chLastchar;
		Char					m_chPrevchar;
		Char					m_pchBuf1[512];				// Puffer des aktuellen Symbols
		Char					m_pchBuf2[512];				// Puffer des aktuellen Symbols
		Char					*m_pchBuf;							// Puffer des aktuellen Symbols
		Bool					m_bFreebuf;
		Bool					m_bNeed_eol;					// needs CR+LF-symbols

	public:
		Bool					m_bMecker;
		Int32					m_lLine;

	public:
		SymbolParser();
		virtual ~SymbolParser();

		Int32 Init(const Filename &m_filename, Bool bMeckern, Bool bNeed_eol);

		String GetString(){return m_filename.GetString();}

		Bool nextchar(Char *chr);
		void skipspaces(Bool &chg);
		void skiplinetoend();
		SYMBOL nextsym(Bool getpoint=false);
		void   rewindsym();

		inline SYMBOL GetSymbol()	{ return cur_symbol; };
		inline Bool GetLeadingSpace() { return cur_leadingspace; }
		inline Char *GetIdent()		{ return m_pchBuf; };
		Bool GetFloat(Float &val);
		Bool GetInt32(Int32 &val);

		Int32 GetTotalFileSize() { return m_lTotalsize; }
		Int32 GetCurrentReadPosition() { return m_lReadpos; }

		Filename GetFilename(void) { return m_filename; }

		inline Bool CheckIdent(const Char *cmp)
		{
			return !Compare(m_pchBuf, cmp);
		}
		inline void SetError(parser_error num=ERROR_WRONG_SYMBOL)
		{
			if (!error) error=num;
		};

		inline void ClearError()						{ error=ERROR_NULL; };
		inline parser_error Error()					{ return error; };
		inline Int32 GetLine()								{ return m_lLine; };
		// to override
		virtual SYMBOL CompareIdent(Char *buf);
		virtual SYMBOL CompareOperator(Char c);
};


SymbolParser::SymbolParser()
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

	m_pchBuf=nullptr;

	ClearError();

	for (t=0;t<256;t++) m_pchIdenttab[t]=false;
	for (t='a';t<='z';t++) m_pchIdenttab[t]=true;
	for (t='A';t<='Z';t++) m_pchIdenttab[t]=true;
	for (t='0';t<='9';t++) m_pchIdenttab[t]=true;
	m_pchIdenttab[(UChar)'_']=true;

	m_pFile = BaseFile::Alloc();
}


SymbolParser::~SymbolParser()
{
	if (m_pFile) m_pFile->Close();
	if (m_bFreebuf && m_pchBuf) bDelete(m_pchBuf);
	bDelete(m_pchReadbuffer);

	BaseFile::Free(m_pFile);
}

Int32 SymbolParser::Init(const Filename &name,Bool meckern,Bool _need_eol_)
{
	Int32 error=ERROR_NULL;

	cur_leadingspace=newline=true;
	m_filename = name;
	m_bMecker=meckern;
	m_bNeed_eol = _need_eol_;
	m_chLastchar=0;
	m_chPrevchar=0;

	if(!m_pFile->Open(name,FILEOPEN_READ,m_bMecker?FILEDIALOG_ANY:FILEDIALOG_NONE)) error=ERROR_FILEOPEN;
	if (m_pFile->GetError()==FILEERROR_NONE)
	{
		m_lTotalsize=m_lFilesize=m_pFile->GetLength();
		if (m_lFilesize>0)
		{
			nextsym();
		}
		else { m_pFile->Close();error=ERROR_FILEOPEN; }
	}
	else
		error=ERROR_FILEOPEN;

	m_lReadpos = 0;
	return error;
}



Bool SymbolParser::nextchar(Char *chr)
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
			else
				m_chPrevchar = m_chLastchar;
		}
	}
	else
		m_chLastchar=0;

	if (chr) *chr=m_chLastchar;
	m_lReadpos++;
	return Error() ? false : true;
};



void SymbolParser::skipspaces(Bool &chg)
{
	chg=false;

	Char c;
	while (true)
	{
		if (!nextchar(&c))	return;

		if (c==CHR_CR || c==CHR_LF)
		{
			chg=true;
			if (m_bNeed_eol) break;
		}
		else if (c==CHR_SPC || c==CHR_TAB) chg=true;
		else break;
	}
	m_bUseagain = true;
};



void SymbolParser::skiplinetoend()
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


void SymbolParser::rewindsym(void)
{
	m_bUsesymbolagain = true;
}


SYMBOL SymbolParser::nextsym(Bool getpoint)
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

	skipspaces(cur_leadingspace);
	cur_leadingspace=cur_leadingspace || newline;
	newline=false;

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
		*txt=0;
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
		*txt=0;
	}
	/*else if (m_chLastchar=='"' || m_chLastchar=='\'')
	{
		Char cmpchr = m_chLastchar;
		Int32 stringlen = 50,pos = 0;
		Bool usenext=false;
		m_pchBuf = bNewDeprecatedUseArraysInstead Char[stringlen + 2];
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
				Char *newstring = bNewDeprecatedUseArraysInstead Char[2 * stringlen + 2];
				if (newstring)
				{
					CopyMem(m_pchBuf, newstring, (unsigned long)stringlen);
					bDelete(m_pchBuf);
					txt = m_pchBuf = newstring;
					stringlen = 2 * stringlen;
				}
			}
		}
	}*/
	else
	{
		// operator entdeckt
		if (!nextchar(txt)) return SYM_NULL; // Lesefehler

		cur_symbol = CompareOperator(*txt);
		if (cur_symbol==SYM_NULL) cur_symbol=SYM_OPERATOR_ILLEGAL;

		txt++;
		*txt=0;
	}

	if (cur_symbol==SYM_OPERATOR_NEWLINE)
		newline=true;

	return cur_symbol;
};

Bool SymbolParser::GetFloat(Float &val)
{
	if (cur_symbol==SYM_NUMBER)
	{
		String s_temp(m_pchBuf);
		val=s_temp.ParseToFloat();
		return true;
	}
	return false;
};



Bool SymbolParser::GetInt32(Int32 &val)
{
	if (cur_symbol==SYM_NUMBER)
	{
		String s_temp(m_pchBuf);
		val=s_temp.ParseToInt32();
		return true;
	}
	return false;
};



SYMBOL SymbolParser::CompareIdent(Char *m_pchBuf)
{
	//if (!Compare(m_pchBuf,"IDENT")) return SYM_IDENT;
	return SYM_NULL;
}


SYMBOL SymbolParser::CompareOperator(Char c)
{
	if (c=='{') return SYM_OPERATOR_GESCHWEIFTAUF;
	else if (c=='}') return SYM_OPERATOR_GESCHWEIFTZU;
	else if (c=='(') return SYM_OPERATOR_KLAMMERAUF;
	else if (c==')') return SYM_OPERATOR_KLAMMERZU;
	else if (c=='=') return SYM_OPERATOR_EQUAL;
	else if (c==',') return SYM_OPERATOR_KOMMA;
	else if (c==';') return SYM_OPERATOR_SEMIKOLON;
	else if (c=='+') return SYM_OPERATOR_PLUS;
	else if (c=='~') return SYM_OPERATOR_TILDE;
	else if (c=='#') return SYM_OPERATOR_PREPROC;
	else if (c=='/') return SYM_OPERATOR_COMMENT;
	else if (c=='*') return SYM_OPERATOR_MUL;
	else if (c==CHR_CR) return SYM_OPERATOR_NEWLINE;
	else if (c==CHR_LF) return SYM_OPERATOR_NEWLINE;

	return SYM_OPERATOR_ILLEGAL;
}

Bool CheckVal(Parser *parse, const String &assignment, Int32 *res, Float *resfloat)
{
	Int32 err = 0;
	Bool ok = false;

	if (assignment.FindFirst('.',nullptr) && !assignment.FindFirst('\'',nullptr))
		ok = parse->Eval(assignment,&err,resfloat,0,0,10);
	else
		ok = parse->EvalLong(assignment,&err,res,0,10);

	//Try to allow evaluation anyway unless the result is 0.0, then return the real value
	if (!ok && (err&NEWPARSERERROR_NUMBERERROR)) ok = true; //A number was merely out of range, evaluation still continued

	return ok;
}

// Preprocessor State
// 0 + #      -> 1
// 1 + define -> 2
// 2 : assignment

// Comment State
// 0 + /      -> 1
// 1 + /      -> 4
// 4 + newline -> 0

// 1 + *      -> 2
// 2 + *      -> 3
// 3 + /      -> 0

// Enum State
// 0     + enum   -> 1
// 1     + {      -> 2
// 2     + ident  -> 3
// 3     + =      -> 4
// 2/3   + }      -> 0

static void XOutputDirect(BaseFile *bf, Char *dest, const String &identifier, const String &output)
{
	(identifier+String("=")+output+String(",")+GeGetLineEnd()).GetCString(dest,1023);
	bf->WriteBytes(dest,(Int)strlen(dest));
}

static void XOutput(BaseFile *bf, Char *dest, const String &identifier, const String &output)
{
	if (identifier==String("NOTINDEX")) return;
	if (identifier==String("LITTLE_ENDIAN")) return;
	if (identifier==String("GE_LITTLE_ENDIAN")) return;
	if (identifier==String("C4DPL_MEMBERMULTIPLIER")) return;
	if (identifier==String("VECTOR_X")) return;
	if (identifier==String("VECTOR_Y")) return;
	if (identifier==String("VECTOR_Z")) return;
	XOutputDirect(bf,dest,identifier,output);
}

struct Identifier
{
	String str;
	Int32   l;
};

static Bool Resolve(Identifier *tab, Int32 cnt, String &str)
{
	Bool chg=false;
	Int32 i,pos,index,indexlength,indexpos=0;

	do
	{
		index=NOTOK;
		indexlength=0;

		for (i=0; i<cnt; i++)
		{
			if (tab[i].str.GetLength()>indexlength && str.FindFirst(tab[i].str,&pos))
			{
				index=i;
				indexlength=tab[i].str.GetLength();
				indexpos=pos;
				chg=true;
			}
		}

//		if (index==NOTOK && str.FindFirst("sizeof",&pos))         // seems that some sizeof constants are still unknown
//			chg=chg;

		// changed, so that the longest string like DESC_VERSION_DEMO instead of DESC_VERSION is picked
		if (index!=NOTOK)
		{
			str.Delete(indexpos,tab[index].str.GetLength());
			str.Insert(indexpos,String::IntToString(tab[index].l));
		}
	}
	while (index!=NOTOK);

	return chg;
}

static Bool DoubleCheck(Identifier *identtable, Int32 cnt)
{
	Int32 i;

	if (identtable[cnt].str==String("DC") ||
		identtable[cnt].str==String("__C4D_MEM_ALIGNMENT_MASK__") ||
		identtable[cnt].str==String("GE_LITTLE_ENDIAN")) // remove those constants
		return true;

	for (i=0; i<cnt; i++)
	{
		if (identtable[i].str==identtable[cnt].str)
		{
			GePrint(String("DOUBLE USAGE ")+identtable[cnt].str);
			return true;
		}
	}

	return false;
}

static String CalculateFourBytes(Parser *parse, String input)
{
	String tmp;
	Int32 i,num=0,multiplier=1<<24,pos,tmpvar;

	for (i=0; i<4; i++)
	{
		if (i<3)
		{
			if (!input.FindFirst(',',&pos))
				return String();
			tmp=input.SubStr(0,pos-1);
			input.Delete(0,pos+1);
		}
		else
			tmp=input;

		if (!parse->EvalLong(tmp,nullptr,&tmpvar,0,10))
			return String();

		num+=tmpvar*multiplier;
		multiplier>>=8;
	}

	return String::IntToString(num);
}

void ParseFile(Identifier *identtable, Int32 &identcnt, const Filename &fn, Int32 &cnt, BaseFile *tf, Bool second)
{
	AutoAlloc<Parser> parse;
	SymbolParser pp;
	Char dest[1024];

	if (pp.Init(fn,true,true)!=ERROR_NULL) return;

	SYMBOL sym = pp.GetSymbol();

	Bool skip;
	Int32 lastenumval=0;
	Int32 commentstate=0;
	Int32 preprocessorstate=0;
	Int32 bracketstate=0;
	Int32 enumstate=0;
	String identifier,assignment;

	if (!second && fn.GetFileString().ToLower()==String("c4d_tools.h"))
		skip=false;

	while (!pp.Error())
	{
		Bool lead=pp.GetLeadingSpace();
		skip=false;

		switch (commentstate)
		{
			case 0:
				if (sym==SYM_OPERATOR_COMMENT)
				{
					commentstate=1;
					skip=true;
				}
				break;

			case 1:
				if (sym==SYM_OPERATOR_COMMENT)
				{
					if (!lead)
						commentstate=4;
					skip=true;
				}
				else if (!lead && sym==SYM_OPERATOR_MUL)
				{
					commentstate=2;
					skip=true;
				}
				else /*if (sym==SYM_OPERATOR_NEWLINE)*/
				{
					commentstate=0;
					skip=true;
				}
				break;

			case 2:
				if (sym==SYM_OPERATOR_MUL)
				{
					commentstate=3;
				}
				skip=true;
				break;

			case 3:
				if (!lead && sym==SYM_OPERATOR_COMMENT)
					commentstate=0;
				else
					commentstate=2;

				skip=true;
				break;

			case 4:
				if (sym==SYM_OPERATOR_NEWLINE)
					commentstate=0;
				else
					skip=true;
				break;
		}

		if (!skip)
		{
			switch (preprocessorstate)
			{
				case 0:
					if (lead && sym==SYM_OPERATOR_PREPROC)
					{
						preprocessorstate=1;
						skip=true;
					}
					break;

				case 1:
					if (!lead && sym==SYM_IDENT)
					{
						if (pp.CheckIdent("define"))
						{
							preprocessorstate=2;
							skip=true;
						}
						else
						{
							pp.skiplinetoend();
							skip=true;
							preprocessorstate=0;
						}
					}
					else
					{
						pp.skiplinetoend();
						skip=true;
						preprocessorstate=0;
					}

					break;

				case 2:
					if (sym==SYM_IDENT)
					{
						identifier=String(pp.GetIdent());
						assignment=String();
						preprocessorstate=3;
						bracketstate=0;
						skip=true;
					}
					else
					{
						// error
						GePrint(String("ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR"));
						pp.skiplinetoend();
						skip=true;
						preprocessorstate=0;
					}
					break;

				case 3:
					skip=true;
					if (sym==SYM_OPERATOR_NEWLINE)
					{
						preprocessorstate=0;

						if (!second && assignment.Content())
						{
							if (assignment.SubStr(0,13)==String("C4D_FOUR_BYTE"))
							{
								assignment = CalculateFourBytes(parse,assignment.SubStr(14,assignment.GetLength()-15));
							}

							Int32 res=0;
							Float resfloat=0.0;

							Bool ok = CheckVal(parse,assignment,&res,&resfloat);
							if (!ok && Resolve(identtable,identcnt,assignment) && CheckVal(parse,assignment,&res,&resfloat))
								ok=true;

							if (ok)
							{
								if (resfloat!=0.0)
								{
									GePrint(String("REAL Constant: ")+identifier+String(" = ")+assignment+String(" : ")+String::FloatToString(resfloat));
								}
								else
								{
									identtable[identcnt].str	= identifier;
									identtable[identcnt].l		= res;

									if (!DoubleCheck(identtable,identcnt))
									{
										identcnt++;
										XOutput(tf,dest,identifier,String::IntToString(res));
										cnt++;
									}
								}
							}
							else
								GePrint(String("SYMBOL FAILED ")+identifier+String(" = ")+assignment);
						}
					}
					else
					{
						if (sym==SYM_OPERATOR_KLAMMERAUF || sym==SYM_OPERATOR_GESCHWEIFTAUF)
						{
							if (bracketstate==NOTOK)
							{
								pp.skiplinetoend();
								preprocessorstate=0;
							}

							bracketstate++;
						}
						else if (sym==SYM_OPERATOR_KLAMMERZU || sym==SYM_OPERATOR_GESCHWEIFTZU)
						{
							bracketstate--;
							if (bracketstate==0)
								bracketstate=NOTOK;
						}

						if (preprocessorstate!=0)
							assignment+=String(pp.GetIdent());
					}
					break;
			}
		}

		if (!skip )
		{
			switch (enumstate)
			{
				case 0:
					if (sym==SYM_IDENT && pp.CheckIdent("enum"))
					{
						lastenumval=0;
						enumstate=1;
						skip=true;
					}
					break;

				case 1:
					if (sym==SYM_OPERATOR_GESCHWEIFTAUF)
					{
						enumstate=2;
						skip=true;
						identifier=String();
						assignment=String();
					}
					break;

				case 2:
					if (sym==SYM_OPERATOR_GESCHWEIFTZU)
					{
						enumstate=0;
						skip=true;
					}
					else if (sym==SYM_IDENT)
					{
						identifier=String(pp.GetIdent());
						enumstate=3;
						skip=true;

						//if (identifier==String("enum"))
						//	skip=skip;
					}
					break;

				case 3:
					if ((sym==SYM_OPERATOR_NEWLINE || sym==SYM_OPERATOR_KOMMA || sym==SYM_OPERATOR_GESCHWEIFTZU) && identifier.Content())
					{
						if (second)
						{
							identtable[identcnt].str	= identifier;
							identtable[identcnt].l		= lastenumval;

							if (!DoubleCheck(identtable,identcnt))
							{
								identcnt++;

								XOutput(tf,dest,identifier,String::IntToString(lastenumval++));
								cnt++;
							}
						}
						assignment=String();
						enumstate=2;
						skip=true;
					}

					if (sym==SYM_OPERATOR_GESCHWEIFTAUF)
					{
						enumstate=2;
						skip=true;
					}
					if (sym==SYM_OPERATOR_GESCHWEIFTZU || sym==SYM_OPERATOR_SEMIKOLON) // no else if here - enumstate might be set to 0
					{
						enumstate=0;
						skip=true;
					}
					else if (sym==SYM_OPERATOR_EQUAL)
					{
						enumstate=4;
						skip=true;
					}
					break;

				case 4:
					if (sym==SYM_OPERATOR_KLAMMERAUF && assignment.Content())
					{
						// opener for constructs like C4D_FOUR_BYTE('a',0,'b','z');
						enumstate=5;
						assignment+=String("(");
					}
					else if ((sym==SYM_OPERATOR_NEWLINE || sym==SYM_OPERATOR_KOMMA || sym==SYM_OPERATOR_GESCHWEIFTZU) && assignment.Content())
					{
						if (second)
						{
							Int32 err;
							if (!parse->EvalLong(assignment,&err,&lastenumval,0,10))
							{
								if (!Resolve(identtable,identcnt,assignment) || !parse->EvalLong(assignment,&err,&lastenumval,0,10))
								{
									GePrint(String("SYMBOL FAILED ")+identifier+String(" = ")+assignment);
									enumstate=2;
									assignment=String();
									break;
								}
							}
							identtable[identcnt].str 	= identifier;
							identtable[identcnt].l		= lastenumval;

							if (!DoubleCheck(identtable,identcnt))
							{
								identcnt++;

								XOutput(tf,dest,identifier,String::IntToString(lastenumval++));
								cnt++;
							}
						}

						assignment=String();
						enumstate=sym==SYM_OPERATOR_GESCHWEIFTZU?0:2;
					}
					else if (sym!=SYM_OPERATOR_KOMMA)
						assignment+=String(pp.GetIdent());

					skip=true;
					break;

				case 5:
					if (sym==SYM_OPERATOR_NEWLINE)
					{
						assignment=String();
						enumstate=2;
					}
					else
					{
						assignment+=String(pp.GetIdent());

						if (sym==SYM_OPERATOR_KLAMMERZU)
						{
							// opener for constructs like C4D_FOUR_BYTE('a',0,'b','z');
							enumstate=4;
							if (assignment.SubStr(0,13)==String("C4D_FOUR_BYTE"))
							{
								assignment = CalculateFourBytes(parse,assignment.SubStr(14,assignment.GetLength()-15));
							}
						}
					}

					skip=true;
					break;
			}
		}

		sym = pp.nextsym();
	}
}

void FillFnList(Filename *fnlist, Int32 &fncount, const Filename &name)
{
	AutoAlloc<BrowseFiles> bf;
	if (!bf) return;
	bf->Init(name,false);
	while (bf->GetNext())
	{
		Filename loc=bf->GetFilename();
		Filename fn=name+loc;

		String fstr=loc.GetFileString().ToLower();
		if (bf->IsDir())
		{
			if (fstr==String("c4d_scaling") ||
					fstr==String("c4d_preview"))
				continue;
			FillFnList(fnlist,fncount,fn);
		}
		else if (loc.CheckSuffix("H"))
		{
			if (fstr==String("x4d_colors.h")) continue;
			if (fstr==String("c4d_gl.h")) continue; // gl defines are too complex to parse and not necessary for COFFEE

			if (fstr.SubStr(0,6)==String("ge_mac")) continue;
			fnlist[fncount++]=fn;
		}
	}
}

static Int32 GetCategory(const String &str)
{
	if (str==String("ge_win_math.h"))
		return -2;
	else if (str==String("lib_description.h"))
		return -1;

	return 0;
}

class SortFileNames : public maxon::BaseSort<SortFileNames>
{
	public:
		static Bool LessThan(const Filename& s1,const Filename& s2)
		{
			String a1=s1.GetFileString().ToLower();
			String a2=s2.GetFileString().ToLower();

			Int32 cat1=GetCategory(a1);
			Int32 cat2=GetCategory(a2);

			if (cat1==0 && cat2==0)
				return a1.LexCompare(a2) < 0;

			return cat1 < cat2;
		}
};

void MakeDistriDialog::ParseSymbols()
{
	AutoAlloc<BaseFile> tf;
	if (!tf || !ForceOpenFileWrite(tf, GeGetStartupPath()+Filename("resource")+Filename("coffeesymbols.h")))
		return;

	Char dest[256];
	(String("enum {")+GeGetLineEnd()).GetCString(dest,255);
	tf->WriteBytes(dest,(Int)strlen(dest));

	Int32 cnt=0;

	Filename fnlist[1000];
	Int32 i,fncount=0;

	FillFnList(fnlist,fncount,GeGetStartupPath()+Filename("resource")+Filename("_api"));

	// sort: always ensure same results
	SortFileNames sfn;
	sfn.Sort(fnlist,fncount);

	Identifier *identtable = bNewDeprecatedUseArraysInstead<Identifier>(50000);
	Int32 identcnt,xrun;

	// these need to be hardcoded for evaluation
	identtable[0].str = String("sizeof(Int16)");
	identtable[0].l   = 2;

	identtable[1].str = String("sizeof(PIX_F)");
	identtable[1].l   = 4;

	identcnt=2;

	// first two files: win_math + lib_description - parse enums & defines in reverse order
	for (xrun=1; xrun>=0; xrun--)
	{
		// first pass: defines
		// second pass: enums

		for (i=0; i<2; i++)
			ParseFile(identtable,identcnt,fnlist[i],cnt,tf,xrun);
	}

	for (xrun=0; xrun<2; xrun++)
	{
		// first pass: defines
		// second pass: enums

		for (i=2; i<fncount; i++)
			ParseFile(identtable,identcnt,fnlist[i],cnt,tf,xrun);
	}

	GePrint(String::IntToString(cnt)+String(" defines in files"));

	(String("COFFEE_HEADERFILE_END_")+GeGetLineEnd()+String("}")+GeGetLineEnd()).GetCString(dest,255);
	tf->WriteBytes(dest,(Int)strlen(dest));

	tf->Close();

	bDelete(identtable);
}
