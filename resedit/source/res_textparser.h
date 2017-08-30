#ifndef RES_TEXTPARSER_H__
#define RES_TEXTPARSER_H__

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
	SYM_OPERATOR_COMMENT,
	SYM_OPERATOR_EQUAL,
	SYM_OPERATOR_KOMMA,
	SYM_OPERATOR_SEMIKOLON,
	SYM_OPERATOR_PLUS,
	SYM_OPERATOR_TILDE,
	SYM_OPERATOR_ECKIGAUF,
	SYM_OPERATOR_ECKIGZU,

	SYM_OPERATOR_ILLEGAL
};



//--------------------------------------------------
class ASCIIParser
{
		BaseFile*     m_pFile;
		Filename			m_filename;

		parser_error	error;
		SYMBOL				cur_symbol;
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
		ASCIIParser();
		virtual ~ASCIIParser();

		Int32 Init(const Filename &m_filename, Bool bMeckern, Bool bNeed_eol);
		void Close();

		Bool nextchar(Char *chr);
		void skipspaces();
		void skiplinetoend();
		SYMBOL nextsym(Bool getpoint=false);
		void   rewindsym();

		inline SYMBOL GetSymbol()	{ return cur_symbol; };
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




#endif	// RES_TEXTPARSER_H__
