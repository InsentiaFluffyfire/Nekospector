#include "Introspect.h"

#include "typedefs.h"
#include "utility.h"
#include "fstring.h"

#include <stdio.h>
#include <malloc.h>

#include <windows.h>

#define typePrefix "IntroDataType"
#define tabSize 8
#define outputFileName "IntrospectionData.h"

const char* basicTypeNames[] = { "char", "memIndex", "bool", "b32", "b64", "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64", "r32", "r64"};

#define fileFilter "*.h"
#define maxFiles 256
char filesToParse[maxFiles][MAX_PATH];
u8 fileCount = 0;
u8 fileStartOffsets[maxFiles] = { 0 };

static char* readWholeFile(const char* fileName) {
	char* fileContents = nullptr;
	FILE* file = fopen(fileName, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		u64 fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		fileContents = (char*)malloc(fileSize);
		if (fileContents) {
			fread(fileContents, fileSize, 1, file);
			fileContents[fileSize] = 0;
			fclose(file);
		} else {
			printf("Failed to read: %s", fileName);
		}
	} else {
		printf("Failed to open: %s", fileName);
	}
	return(fileContents);
}

static inline u32 maxU32(u32 a, u32 b) {
	return(a > b ? a : b);
}

static inline u32 minU32(u32 a, u32 b) {
	return(a < b ? a : b);
}
static bool compareCString(const char* s1, const char* s2) {
	if (s1[0]&&s2[0]) {
		u32 c = 0;
		while (s1[c]) {
			if (s1[c]!=s2[c]) { return(false); }
			++c;
		}
		if (s2[c]=='\0') { return(true); }
	}
	return(false);
	
}
typedef enum FluffyTokenType {
	Token_Null,
	Token_ParenOpen,
	Token_ParenClose,
	Token_BracketOpen,
	Token_BracketClose,
	Token_BraceOpen,
	Token_BraceClose,
	Token_Asterisk,
	Token_Colon,
	Token_Semicolon,
	Token_Comma,
	
	Token_String,
	Token_Number,
	Token_Identifier,

	Token_FileEnd,
	Token_Unknown = -1,
} FluffyTokenType;

typedef struct Token {
	FluffyTokenType type;
	u32 length;
	char* text;
} Token;

const Token nullToken = {
	.length = 1,
	.text = "0",
	.type = Token_Null,
};

typedef struct Tokenizer {
	char* at;
}Tokenizer;

static Tokenizer tokenizer = {
	.at = nullptr,
};

#define maxMemberCount 65536u
#define maxEnumeratorValueCount 65536u
typedef enum MemberFlagBits {
	MEMBER_FLAG_BIT_NONE   = 0x0,
	MEMBER_FLAG_BIT_PTR    = 0x1,
	MEMBER_FLAG_BIT_ENUM   = 0x2,
	MEMBER_FLAG_BIT_TUPPLE = 0x4,
} MemberFlagBits;

typedef enum ModeFlagBits {
	MODE_BIT_NONE			= 0x0,
	MODE_BIT_SINGLE			= 0x1,
	MODE_BIT_TUPPLE			= 0x4,
}ModeFlagBits;

typedef struct StructData {
	Token typeToken;
	u32 start;
	u32 end;
	ModeFlagBits flags;
} StructData;

#define maxStructCount 256u
#define maxEnumeratorCount 256u

typedef struct MemberData{
	Token typeToken;
	Token identifierToken;
	Token value[3];
	bool hasTypeToken;
	bool hasIdentifierToken;
	bool hasValueToken[3];
	MemberFlagBits flags;
}MemberData;

typedef struct StructParser {
	u32 braceCount;
	FILE* outputFile;
	u16 structCount;
	u16 memberCount;
	u32 maxTypeLength;
	u32 maxIdentifierLength;
	u32 maxCombinedLength;
	u32 maxArrayCountLength;
	StructData structs[maxStructCount];
	MemberData members[maxMemberCount];
}StructParser;

StructParser* structParser;

typedef struct EnumeratorValue {
	Token identifierToken;
	s32 value;
	bool hasIdentifierToken;
}EnumeratorValue;

typedef struct EnumeratorData {
	Token typeToken;
	u32 start;
	u32 end;
} EnumeratorData;

typedef struct EnumParser {
	u32 maxIdentifierLength;
	u32 maxValueLength;
	u32 currentEnumerators;
	u32 currentEnumeratorValues;
	EnumeratorData enumerators[maxEnumeratorCount];
	EnumeratorValue enumeratorValues[maxEnumeratorValueCount];
}EnumParser;

EnumParser* enumParser;

typedef struct FileResults {
	u16 structStart;
	u16 structEnd;
	u16 enumStart;
	u16 enumEnd;
}FileResults;

FileResults parsingResults[maxFiles];

static void setOutputFile(FILE* file) {
	structParser->outputFile = file;
}

//sets which file the tokenizer will process
static inline initTokenizer(char* file) {
	tokenizer.at = file;
}

static inline bool isWhiteSpace(const char c) {
	bool result =	((c==' ')||
					(c=='\t')||
					(c=='\v')||
					(c=='\f')||
					(c=='\n')||
					(c=='\r'));
	return(result);
}

static inline bool isLetter(const char c) {
	bool result =	(((c>='a')&&(c<='z'))||
					((c>='A')&&(c<='Z')));
	return(result);
}

static inline bool isDigit(const char c) {
	bool result =	((c>= '0') && (c<='9'));
	return(result);
}

static void eatAllDeadSpace() {
	for (;;) {
		if (isWhiteSpace(tokenizer.at[0])) {
			++tokenizer.at;
		} else if ((tokenizer.at[0] == '/')&&(tokenizer.at[1] == '*')) {
			tokenizer.at += 2;
			while ((tokenizer.at[0])&& (tokenizer.at[1])&&!((tokenizer.at[0] == '*') && (tokenizer.at[1] == '/'))) {
				++tokenizer.at;
			}
			if (tokenizer.at[0] == '/') {
				++tokenizer.at;
			}
		} else if ((tokenizer.at[0] == '/')&&(tokenizer.at[1] == '/')) {
			tokenizer.at += 2;
			while ((tokenizer.at[0] != '\n') && (tokenizer.at[0] != '\r') && (tokenizer.at[0])) {
				++tokenizer.at;
			}
		} else {
			break;
		}
	}
}

static Token getToken() {
	eatAllDeadSpace();
	Token token = {
		.length = 1,
		.text = tokenizer.at,
	};
	switch (tokenizer.at[0]) {
		case '\0':{ ++tokenizer.at;token.type = Token_FileEnd; }break;
		case '(': { ++tokenizer.at;token.type = Token_ParenOpen; }break;
		case ')': { ++tokenizer.at;token.type = Token_ParenClose; }break;
		case '[': { ++tokenizer.at;token.type = Token_BracketOpen; }break;
		case ']': { ++tokenizer.at;token.type = Token_BracketClose; }break;
		case '{': { ++tokenizer.at;token.type = Token_BraceOpen; }break;
		case '}': { ++tokenizer.at;token.type = Token_BraceClose; }break;
		case '*': { ++tokenizer.at;token.type = Token_Asterisk; }break;
		case ':': { ++tokenizer.at;token.type = Token_Colon; }break;
		case ';': { ++tokenizer.at;token.type = Token_Semicolon; }break;
		case ',': { ++tokenizer.at;token.type = Token_Comma; }break;

		case '"': {
			++tokenizer.at;
			token.type = Token_String;
			token.text = tokenizer.at;
			while ((tokenizer.at[0] != '"') && (tokenizer.at[0])){
				if ((tokenizer.at[0] == '\\')&& tokenizer.at[1]) {
					++tokenizer.at;
				}
				++tokenizer.at;
			}
			token.length = tokenizer.at-token.text;
			if (tokenizer.at[0] == '"') {
				++tokenizer.at;
			}
		}break;
		default: {
			if (isLetter(tokenizer.at[0])|| (tokenizer.at[0] == '#') || ((tokenizer.at[0] == '-')&&isLetter(tokenizer.at[1]))) {
				token.type = Token_Identifier;
				token.text = tokenizer.at;
				while (isLetter(tokenizer.at[0]) || isDigit(tokenizer.at[0]) || (tokenizer.at[0] == '_')|| (tokenizer.at[0] == '#')|| (tokenizer.at[0] == '-')) {
					++tokenizer.at;
				}
				token.length = tokenizer.at-token.text;
			} else if (isDigit(tokenizer.at[0]) || ((tokenizer.at[0] == '-')&&isDigit(tokenizer.at[1]))) {
				token.type = Token_Number;
				token.text = tokenizer.at;
				while (isDigit(tokenizer.at[0]) || (tokenizer.at[0]=='.') || (tokenizer.at[0]=='f') || (tokenizer.at[0]=='e')|| (tokenizer.at[0]=='x')|| (tokenizer.at[0]=='b')|| (tokenizer.at[0]=='-')) {
					++tokenizer.at;
				}
				token.length = tokenizer.at-token.text;
			} else {
				token.type = Token_Unknown;
				++tokenizer.at;
			}
		}break;
	}
	return(token);
}

static u32 tokenToU32(Token t) {
	u32 x = 0;
	for (u32 d = 0; d < t.length; ++d) {
		x *= 10;
		x+=t.text[d]-'0';
	}
	return(x);
}

static s32 tokenToS32(Token t) {
	s32 x = 0;
	const bool negative = (t.text[0] =='-');
	for (u32 d = negative; d < t.length; ++d) {
		x *= 10;
		x += t.text[d]-'0';
	}
	return(negative ? -x : x);
}

static bool compareIdentifier(Token token, const char* match) {
	
	for (u32 c = 0; c < token.length; ++c) {
		if ((match[c] == 0) || (token.text[c] != match[c])) {
			return false;
		}
	}
	return(match[token.length]== '\0');
}

static bool compareTokens(const Token token1, const Token token2) {
	if ((token1.type != token2.type) || (token1.length == token2.length)) {
		return(false);
	}
	for (u32 c = 0; c < token1.length; ++c) {
		if ((token1.text[c] != token2.text[c])) {
			return false;
		}
	}
	return(true);
}
static inline bool requiredToken(const FluffyTokenType tokenType) {
	Token token = getToken();
	return(tokenType==token.type);
}

static bool parseModeParameter() {
	Token token = getToken();
	if (token.type == Token_String) {
		if (compareIdentifier(token, "tupple")) {
			structParser->structs[structParser->structCount].flags |= MODE_BIT_TUPPLE;
		}
		return(parseModeParameter());
	}
	if (token.type == Token_ParenClose) {
		return(false);
	}
	fprintf(stderr, "Invalid syntax in Introspection params: %.*s", token.length, token.text);
	return(false);
}
static void parseIntrospectionParams() {	
	Token token = getToken();
		switch (token.type) {
			case Token_ParenClose:
			case Token_FileEnd: {}break;
			case Token_Identifier: {
				if (compareIdentifier(token, "mode") && requiredToken(Token_Colon)) {
					if (parseModeParameter()) {
						parseIntrospectionParams();
					}
				} else {
					fprintf(stderr, "Error: Unsupported paramater or missing :");
				}
			}break;
			default: { parseIntrospectionParams(); }break;
		}
}

#define maxNameLength 127
typedef struct EnumValue {
	char name[maxNameLength+1];
} EnumValue;

#define maxEnumValues 1024u
static EnumValue enumValues[maxEnumValues] = { 0 };
static u32 currentEnumValues = 0u;
static EnumValue enumValuesE[maxEnumValues] = { 0 };
static u32 currentEnumValuesE = 0u;

static void initEnumValues() {
	for (; currentEnumValues < ArrayCount(basicTypeNames); ++currentEnumValues) {
		u32 c = 0;
		while (basicTypeNames[currentEnumValues][c]) {
			enumValues[currentEnumValues].name[c] = basicTypeNames[currentEnumValues][c];	
			++c;
		}
	}
};

static inline void writeEnumValue(FILE* file, EnumValue v) {
	fprintf(file, "\t%s_%s", typePrefix, &v.name[0]);
	fprintf(file, ",\n");
}

static void pushEnumValue(Token t, bool isEnum) {
	if (isEnum) {
		for (u32 s = 0; s< currentEnumValuesE; ++s) {
			if (compareIdentifier(t, enumValuesE[s].name)) { return; }
		}

		for (u32 c = 0; c<t.length; ++c) {
			enumValuesE[currentEnumValuesE].name[c] = t.text[c];
		}
		enumValuesE[currentEnumValuesE].name[t.length] = '\0';
		currentEnumValuesE++;
	}else {
		for (u32 s = 0; s< currentEnumValues; ++s) {
			if (compareIdentifier(t, enumValues[s].name)) { return;}
		}
		
		for (u32 c = 0; c<t.length; ++c) {
			enumValues[currentEnumValues].name[c] = t.text[c];
		}
		enumValues[currentEnumValues].name[t.length] = '\0';
		currentEnumValues++;
	}
}

static void writeEnumFile(FILE* file) {
	fprintf(file, "typedef enum IntroDataType {\n");
	for (u32 e = 0; e< currentEnumValues; ++e) {
		writeEnumValue(file, enumValues[e]);
	}
	for (u32 e = 0; e< currentEnumValuesE; ++e) {
		writeEnumValue(file, enumValuesE[e]);
	}
	fprintf(file, "} IntroDataType;\n\n");
}

static void parseMember() {
	Token token = getToken();
	switch (token.type) {
		case Token_Semicolon: {
			if (structParser->members[structParser->memberCount].hasIdentifierToken&& structParser->members[structParser->memberCount].hasTypeToken) {
				if ((structParser->members[structParser->memberCount].hasValueToken[0])&&(structParser->structs[structParser->structCount].flags&MODE_BIT_TUPPLE)) {
					structParser->members[structParser->memberCount].flags |= MEMBER_FLAG_BIT_TUPPLE;
				}
				pushEnumValue(structParser->members[structParser->memberCount].typeToken, (structParser->members[structParser->memberCount].flags & MEMBER_FLAG_BIT_ENUM) ? true : false);
				u32 arrayDisplayLength = 0;
				arrayDisplayLength += structParser->members[structParser->memberCount].hasValueToken[0] ? structParser->members[structParser->memberCount].value[0].length : nullToken.length;
				arrayDisplayLength += structParser->members[structParser->memberCount].hasValueToken[1] ? structParser->members[structParser->memberCount].value[1].length : nullToken.length;
				arrayDisplayLength += structParser->members[structParser->memberCount].hasValueToken[2] ? structParser->members[structParser->memberCount].value[2].length : nullToken.length;
				
				structParser->maxArrayCountLength = maxU32(structParser->maxArrayCountLength, arrayDisplayLength);
				structParser->maxIdentifierLength = maxU32(structParser->maxIdentifierLength, structParser->members[structParser->memberCount].identifierToken.length);
				structParser->maxTypeLength = maxU32(structParser->maxTypeLength, structParser->members[structParser->memberCount].typeToken.length);
				structParser->maxCombinedLength = maxU32(structParser->maxCombinedLength, (structParser->structs[structParser->structCount].typeToken.length+structParser->members[structParser->memberCount].identifierToken.length));
				++structParser->memberCount;
			}
		} break;
		case Token_FileEnd: {
		}break;
		case Token_BraceClose: {
			--structParser->braceCount;
			if (structParser->braceCount) {
				parseMember();
			}
		}break;
		case Token_BraceOpen: {
			++structParser->braceCount;
		}break;
		case Token_BracketOpen: {
			token = getToken();
			if ((token.type == Token_Number)||(token.type == Token_Identifier)) {
				if (!structParser->members[structParser->memberCount].hasValueToken[0]) {
					structParser->members[structParser->memberCount].value[0] = token;
					structParser->members[structParser->memberCount].hasValueToken[0] = true;
				} else if (!structParser->members[structParser->memberCount].hasValueToken[1]) {
					structParser->members[structParser->memberCount].value[1] = token;
					structParser->members[structParser->memberCount].hasValueToken[1] = true;
				} else if (!structParser->members[structParser->memberCount].hasValueToken[2]) {
					structParser->members[structParser->memberCount].value[2] = token;
					structParser->members[structParser->memberCount].hasValueToken[2] = true;
				} else {
					fprintf(stderr, "Error: Too many array dimensions.\n");
				}
			} //doesn't work with constants yet
			if (!requiredToken(Token_BracketClose)) { fprintf(stderr, "Error: Invalid Array Syntax.\n\n"); }
			parseMember();
		}break;
		case Token_Asterisk: {
			structParser->members[structParser->memberCount].flags |= MEMBER_FLAG_BIT_PTR;
			parseMember();
		}break;
		case Token_Identifier: {
			if (!compareIdentifier(token, "struct")&&!compareIdentifier(token, "union")) {
				if (structParser->members[structParser->memberCount].hasTypeToken) {
					structParser->members[structParser->memberCount].identifierToken = token; structParser->members[structParser->memberCount].hasIdentifierToken = true;
				} else {
					if (compareIdentifier(token, "enum")) {
						structParser->members[structParser->memberCount].flags |= MEMBER_FLAG_BIT_ENUM;
					} else {
						structParser->members[structParser->memberCount].typeToken = token; structParser->members[structParser->memberCount].hasTypeToken = true;
					}
				}
			}
			parseMember();
		}break;
		default: {
			parseMember();
		}break;
	}
}

static void writeMemberData(FILE* file, MemberData member, StructData structData) {
	fprintf(file, "\t{%s_%.*s,", typePrefix, member.typeToken.length, member.typeToken.text);
	for (u32 c = 0; c < structParser->maxTypeLength-member.typeToken.length; ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " \"%.*s\",", member.identifierToken.length, member.identifierToken.text);
	for (u32 c = 0; c < structParser->maxIdentifierLength-member.identifierToken.length; ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " {%.*s, %.*s, %.*s},",
		member.hasValueToken[0] ? member.value[0].length : nullToken.length,
		member.hasValueToken[0] ? member.value[0].text   : nullToken.text,
		member.hasValueToken[1] ? member.value[1].length : nullToken.length,
		member.hasValueToken[1] ? member.value[1].text   : nullToken.text,
		member.hasValueToken[2] ? member.value[2].length : nullToken.length,
		member.hasValueToken[2] ? member.value[2].text   : nullToken.text);
	u32 arrayDisplayLength = 0;
	arrayDisplayLength += member.hasValueToken[0] ? member.value[0].length : nullToken.length;
	arrayDisplayLength += member.hasValueToken[1] ? member.value[1].length : nullToken.length;
	arrayDisplayLength += member.hasValueToken[2] ? member.value[2].length : nullToken.length;
	for (u32 c = 0; c < structParser->maxArrayCountLength-arrayDisplayLength; ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " offsetof(%.*s, %.*s),", structData.typeToken.length, structData.typeToken.text, member.identifierToken.length, member.identifierToken.text);
	for (u32 c = 0; c < structParser->maxCombinedLength-(member.identifierToken.length+structData.typeToken.length); ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " 0x%x },\n", member.flags);
}

static void writeStruct(FILE* file, StructData structData) {
	fprintf(structParser->outputFile, "MemberDefinition membersOf_%.*s[] = {\n", structData.typeToken.length, structData.typeToken.text);
	for (u32 i = structData.start; i < structData.end; ++i) {
		writeMemberData(file, structParser->members[i], structData);
	}
	fprintf(structParser->outputFile, "};\n\n");
}

static void parseStruct() {
	structParser->structs[structParser->structCount].typeToken = getToken();
	structParser->structs[structParser->structCount].start = structParser->memberCount;
	pushEnumValue(structParser->structs[structParser->structCount].typeToken, false);
	if (requiredToken(Token_BraceOpen)) {
		structParser->braceCount = 1;
		while (structParser->braceCount) {
			parseMember();
		}
	}
	structParser->structs[structParser->structCount].end = structParser->memberCount;
	++structParser->structCount;

}

static void parseEnumerator() {
	Token token = getToken();
	switch (token.type) {
		case Token_Semicolon:
		case Token_BraceClose:
		case Token_FileEnd: {
		}break;
		case Token_Number: {
			const s32 value = tokenToS32(token);
			enumParser->enumeratorValues[enumParser->currentEnumeratorValues].value = value;
			enumParser->maxValueLength = maxU32(enumParser->maxValueLength, f_Log10(value < 0 ? -value : value)+(value < 0));
			if (requiredToken(Token_Comma)) {
				++enumParser->currentEnumeratorValues;
				parseEnumerator();
			} else {
				fprintf(stderr, "Error: Unexpected Token: %i", token.type);
			}
		}break;
		case Token_Comma: {
			enumParser->enumeratorValues[enumParser->currentEnumeratorValues].value = (s32)enumParser->currentEnumeratorValues - (s32)enumParser->enumerators[enumParser->currentEnumerators].start;
			enumParser->maxValueLength = maxU32(enumParser->maxValueLength, f_Log10(enumParser->currentEnumerators));
			++enumParser->currentEnumeratorValues;
			parseEnumerator();
		}break;
		case Token_Identifier: {
			if (!enumParser->enumeratorValues[enumParser->currentEnumeratorValues].hasIdentifierToken) {
				enumParser->enumeratorValues[enumParser->currentEnumeratorValues].identifierToken = token;
				enumParser->enumeratorValues[enumParser->currentEnumeratorValues].hasIdentifierToken = true;
				enumParser->maxIdentifierLength = maxU32(enumParser->maxIdentifierLength, token.length);
			} else {
				fprintf(stderr, "Error: Unexpected identifier: %.*s", token.length, token.text);
			}
			parseEnumerator();
		}break;
		default: { parseEnumerator(); }break;
	}
}
static void writeEnumerator(FILE* file, EnumeratorValue e) {
	fprintf(file, "\t{\"%.*s\",", e.identifierToken.length, e.identifierToken.text);
	for (u32 c = 0; c < maxU32(enumParser->maxIdentifierLength, tabSize)-e.identifierToken.length; ++c) {
		fprintf(file, " ");
	}
	for (u32 c = 0; c < maxU32(enumParser->maxValueLength, tabSize)-(f_Log10(e.value < 0 ? -e.value : e.value)+(e.value < 0)); ++c) {
		fprintf(file, " ");
	}
	fprintf(file, "%i", e.value);
	fprintf(file, " },\n");
}

static void parseEnum() {
	Token nameToken = getToken();
	enumParser->enumerators[enumParser->currentEnumerators].typeToken = nameToken;
	enumParser->enumerators[enumParser->currentEnumerators].start = enumParser->currentEnumeratorValues;
	pushEnumValue(nameToken, true);
	if (requiredToken(Token_BraceOpen)) {
		parseEnumerator();
	}
	enumParser->enumerators[enumParser->currentEnumerators].end = enumParser->currentEnumeratorValues;
	++enumParser->currentEnumerators;
}

static void writeEnum(FILE* file, EnumeratorData enumerator) {
	fprintf(structParser->outputFile, "EnumeratorDefinition enumeratorsOf_%.*s[] = {\n", enumerator.typeToken.length, enumerator.typeToken.text);
	for (u32 e = enumerator.start; e < enumerator.end; ++e) {
		writeEnumerator(structParser->outputFile, enumParser->enumeratorValues[e]);
	}
	fprintf(structParser->outputFile, "};\n\n");
}

static void parseIntrospectable() {
	if (requiredToken(Token_ParenOpen)) {
		parseIntrospectionParams();
		Token token = getToken();
		if (compareIdentifier(token, "typedef")) { token = getToken(); }
		if (compareIdentifier(token, "struct")) {
			parseStruct();
		} else if (compareIdentifier(token, "enum")) {
			parseEnum();
		} else if (compareIdentifier(token, "union")) {
			parseStruct();
		} else {
			fprintf(stderr, "Error: Introspectable must be a struct, union or enum!\n");
		}
	} else {
		fprintf(stderr, "Error: Expecting '(' Token!\n");
	}
}

static void writeEnumeratorHelperFunction(FILE* file) {
	fprintf(file, "static const char* findEnumString_(EnumeratorDefinition* enumDefinition, u32 size, s32* value);\n");
	fprintf(file, "#define findEnumString(type, value_ptr) findEnumString_(enumeratorsOf_##type, ArrayCount(enumeratorsOf_##type), (s32*)value_ptr)\n");
	fprintf(file, "\nstatic const char* findEnumString_(EnumeratorDefinition* enumDefinition, u32 size, s32* value) {\n");
	fprintf(file, "\tu32 v = 0;\n");
	fprintf(file, "\t\t for(u32 v = 0; v < size; ++v) {\n");
	fprintf(file, "\t\t\t if(enumDefintion[v].value == *value) {return(enumDefintion[v].identifier);}\n");
	fprintf(file, "\t\t}\n");
	fprintf(file, "\treturn(v);\n");
	fprintf(file, "}\n\n");
}

static void writeFormatingFunction(FILE* file) {
	fprintf(file, "static void DEBUG_dumpMembers_(char* buffer, u32 bufferSize, u32 currentBufferPos, MemberDefinition* memberDefinitions, u32 memberCount, void* displayable);\n");
	fprintf(file, "#define DEBUG_dumpMembers(buffer, bufferPos, type, ptr_displayable) DEBUG_dumpMembers_(buffer, ArrayCount(buffer), bufferPos, membersOf_##type, ArrayCount(membersOf_##type), ptr_displayable)\n");
	fprintf(file, "\nstatic void DEBUG_dumpMembers_(char* buffer, u32 bufferSize, u32 currentBufferPos, MemberDefinition* memberDefinitions, u32 memberCount, void* displayable) {\n");
	fprintf(file, "\tfor (u32 m = 0; m<memberCount; ++m) {\n");
	fprintf(file, "\t\tvoid* member = (void*)((u8*) displayable + memberDefinitions[m].offset);\n");
	fprintf(file, "\t\tif (memberDefinitions[m].flags & MEMBER_FLAG_BIT_ENUM) {\n");
	fprintf(file, "\t\t\tswitch(memberDefinitions[m].type); {\n");
	for (u32 i = 0; i < currentEnumValuesE; ++i) {
		fprintf(file, "\t\t\t\tcase %s_%s: {\n", typePrefix, enumValuesE[i].name);
		fprintf(file, "\t\t\t\t\tif (memberDefinitions[m].flags & MEMBER_FLAG_BIT_PTR) { AppendCString(buffer, findEnumString(%s), *member);}\n", enumValuesE[i].name);
		fprintf(file, "\t\t\t\t\telse {AppendCString(buffer, findEnumString(%s), member);}\n", enumValuesE[i].name);
		fprintf(file, "\t\t\t\t}break;\n");
	}
	fprintf(file, "\t\t\t}\n");
	fprintf(file, "\t\telse {\n");
	fprintf(file, "\t\t\tswitch(memberDefinitions[m].type); {\n");
	
	fprintf(file, "\n\t\t\t//Strings and chars:\n");
	fprintf(file, "\t\t\t\tcase %s_char: {\n", typePrefix);
	fprintf(file, "\t\t\t\t\tif (memberDefinitions[m].flags & MEMBER_FLAG_BIT_PTR) { AppendCString(&buffer[currentBufferPos], *(char**)member);}\n");
	
	fprintf(file, "\t\t\t\t}break;\n");
	
	fprintf(file, "\n\t\t\t//Basic types:\n");
	for (u32 i = 1; i< ArrayCount(basicTypeNames); ++i) {
		fprintf(file, "\t\t\t\tcase %s_%s: {\n", typePrefix, enumValues[i].name);
		fprintf(file, "\t\t\t\t\tif (memberDefinitions[m].flags & MEMBER_FLAG_BIT_PTR) { %sToChars(&buffer[currentBufferPos], **(%s**)member);}\n", enumValues[i].name, enumValues[i].name);
		fprintf(file, "\t\t\t\t\telse { %sToChars(&buffer[currentBufferPos], *(%s*)member);}\n", enumValues[i].name, enumValues[i].name);
		fprintf(file, "\t\t\t\t}break;\n");
	}
	fprintf(file, "\n\t\t\t//Custom types:\n");
	for (u32 i = ArrayCount(basicTypeNames); i <currentEnumValues; ++i) {
		fprintf(file, "\t\t\t\tcase %s_%s: {\n", typePrefix, enumValues[i].name);
		fprintf(file, "\t\t\t\t\tif (memberDefinitions[m].flags & MEMBER_FLAG_BIT_PTR) { DEBUG_dumpMembers(buffer, currentBufferPos, %s, *member); }\n", enumValues[i].name);
		fprintf(file, "\t\t\t\t\telse { DEBUG_dumpMembers(buffer, currentBufferPos, %s, member); }\n", enumValues[i].name);
		fprintf(file, "\t\t\t\t}break;\n");
	}
	fprintf(file, "\t\t\t}\n");
	fprintf(file, "\t\t}\n");
	fprintf(file, "\t}\n");
	fprintf(file, "}\n");
}

static void printFileHeader(FILE* file) {
	SYSTEMTIME sTime;
	SYSTEMTIME lTime;
	GetSystemTime(&sTime);
	GetLocalTime(&lTime);
	fprintf(file, "/*===========================================================================================================================================================================\n");
	fprintf(file, "|\n");
	fprintf(file, "|       Generated on %u/%u/%u at %u:%u:%u (system)\n",sTime.wDay, sTime.wMonth, sTime.wYear, sTime.wHour, sTime.wMinute, sTime.wSecond);
	fprintf(file, "|       Generated on %u/%u/%u at %u:%u:%u (local)\n", lTime.wDay, lTime.wMonth, lTime.wYear, lTime.wHour, lTime.wMinute, lTime.wSecond);
	fprintf(file, "|\n");
	fprintf(file, "===========================================================================================================================================================================*/\n\n");
	
	fprintf(file, "#pragma once\n\n");
	fprintf(file, "#include \"typedefs.h\"\n");
	fprintf(file, "#include \"utility.h\"\n");
	fprintf(file, "#include \"fstring.h\"\n");
	fprintf(file, "#include \"platformMemory.h\"\n\n");
};
static void printMemberDefintion(FILE* file) {
	fprintf(file, "typedef enum MemberFlagBits {\n");
	fprintf(file, "\t MEMBER_FLAG_BIT_NONE    = 0x0,\n");
	fprintf(file, "\t MEMBER_FLAG_BIT_PTR     = 0x1,\n");
	fprintf(file, "\t MEMBER_FLAG_BIT_ENUM    = 0x2,\n");
	fprintf(file, "\t MEMBER_FLAG_BIT_TUPPLE  = 0x4,\n");
	fprintf(file, "} MemberFlagBits;\n\n");

	fprintf(file, "typedef struct MemberDefinition {\n");
	fprintf(file, "\t IntroDataType  type;\n");
	fprintf(file, "\t const char*    identifier;\n");
	fprintf(file, "\t uvec3          count;\n");
	fprintf(file, "\t u64            offset;\n");
	fprintf(file, "\t MemberFlagBits flags;\n");
	fprintf(file, "} MemberDefinition;\n\n");
}
static void printEnumeratorDefintion(FILE* file) {
	fprintf(file, "typedef struct EnumeratorDefinition {\n");
	fprintf(file, "\t const char*  identifier;\n");
	fprintf(file, "\t s32          value;\n");
	fprintf(file, "} EnumeratorDefinition;\n\n");
}
static void printDataHeader(FILE* file, const char* fileName) {
	fprintf(file, "/*===========================================================================================================================================================================\n");
	fprintf(file, "|\n");
	fprintf(file, "|       %s - Introspection Data\n", fileName);
	fprintf(file, "|\n");
	fprintf(file, "===========================================================================================================================================================================*/\n\n");
}

static void writeFunctionHeader(FILE* file) {
	fprintf(file, "/*===========================================================================================================================================================================\n");
	fprintf(file, "|\n");
	fprintf(file, "|       Functions\n");
	fprintf(file, "|\n");
	fprintf(file, "===========================================================================================================================================================================*/\n\n");
}

typedef enum CurrentArgType {
	ArgType_None,
	ArgType_Path,
	ArgType_Out,
	ArgType_OutPath,
}CurrentArgType;

int main(int argCount, char** args)
{
	LARGE_INTEGER clockFrequency;
	QueryPerformanceFrequency(&clockFrequency);
	char* outFileName = outputFileName;
	char outFileDirectory[MAX_PATH];
	memset(outFileDirectory, 0, MAX_PATH);
	u32 outPathLength = 0;
	structParser = malloc(sizeof(StructParser));
	memset(structParser, 0, sizeof(StructParser));
	enumParser = malloc(sizeof(EnumParser));
	memset(enumParser, 0, sizeof(EnumParser));
	if (argCount>1) {
		CurrentArgType argType = ArgType_None;
		for (u32 a = 1; a < argCount; ++a) {
			initTokenizer(args[a]);
			Token token = getToken();
			switch (argType) {
				case ArgType_Out: { outFileName = args[a]; argType = ArgType_None; }break;
				case ArgType_Path: {
					if (args[a][0] == '-') { goto noArgType; }
					char currentDirectory[MAX_PATH];
					memset(currentDirectory, 0, MAX_PATH);
					u32 length = AppendCString(currentDirectory, args[a]);
					AppendCString(&currentDirectory[length], fileFilter);
					WIN32_FIND_DATAA fileData = { 0 };
					HANDLE searchHandle = FindFirstFileA(currentDirectory, &fileData);
					if (searchHandle != INVALID_HANDLE_VALUE) {
						do {
							memcpy(filesToParse[fileCount], token.text, length);
							if (compareCString(&fileData.cFileName, "Introspect.h")) { continue; }
							AppendCString(&filesToParse[fileCount][length], fileData.cFileName);
							++fileCount;
						} while (FindNextFileA(searchHandle, &fileData));
					}
				}break;
				case ArgType_OutPath: {
					outPathLength = AppendCString(outFileDirectory, args[a]);
					argType = ArgType_None;
				} break;
				case ArgType_None: {
					noArgType:
					if (compareIdentifier(token, "-p")||compareIdentifier(token, "-path")) {
						argType = ArgType_Path;
					}
					if (compareIdentifier(token, "-o") || compareIdentifier(token, "-out")) {
						argType = ArgType_Out;
					}
					if (compareIdentifier(token, "-op") || compareIdentifier(token, "-outpath")) {
						argType = ArgType_OutPath;
					}
				}break;
			}
		}
	};
	AppendCString(&outFileDirectory[outPathLength], outFileName);
	FILE* outputFile = fopen(outFileDirectory, "wb");
	for (u32 f = 0; f < fileCount; ++f) {
		printf("File: %s\n", filesToParse[f]);
	}
	setOutputFile(outputFile);
	initEnumValues();
	LARGE_INTEGER startMarker;
	LARGE_INTEGER endMarker;
	for (u32 f = 0; f < fileCount; ++f) {
		QueryPerformanceCounter(&startMarker);
		parsingResults[f].structStart = structParser->structCount;
		parsingResults[f].enumStart = enumParser->currentEnumerators;
		char* file = readWholeFile(filesToParse[f]);
		initTokenizer(file);
		const char* fileName = filesToParse[f];
		for (;;) {
			Token token = getToken();
			switch (token.type) {
				case Token_FileEnd: {
					goto fileEnd;
				}break;
				case Token_Unknown: {
				}break;
				case Token_Identifier: {
					if (compareIdentifier(token, "INTROSPECT")) {
						parseIntrospectable();
					}
				}break;
				default: {
				}break;
			}
		}
	fileEnd:
		parsingResults[f].structEnd = structParser->structCount;
		parsingResults[f].enumEnd = enumParser->currentEnumerators;
		u32 structCount = parsingResults[f].structEnd-parsingResults[f].structStart;
		u32 enumCount = parsingResults[f].enumEnd-parsingResults[f].enumStart;
		QueryPerformanceCounter(&endMarker);
		printf("Found %u structures and %u enums.", structCount, enumCount);
		r64 time = ((r64)endMarker.QuadPart-(r64)startMarker.QuadPart)/(r64)clockFrequency.QuadPart;
		if (time<1.0f) {
			time *= 1000.0f;
			printf(" Time taken: %.2fms\n", time);
		} else {
			printf(" Time taken: %.2fs\n", time);
		}
	}

	printFileHeader(outputFile);
	writeEnumFile(outputFile);
	printMemberDefintion(outputFile);
	printEnumeratorDefintion(outputFile);

	for (u32 f = 0; f < fileCount; ++f) {
		if ((parsingResults[f].structStart!=parsingResults[f].structEnd)||(parsingResults[f].enumStart!=parsingResults[f].enumEnd)) {
			printDataHeader(outputFile, filesToParse[f]);
		}
		for (u32 e = parsingResults[f].enumStart; e < parsingResults[f].enumEnd; ++e) {
			writeEnum(outputFile, enumParser->enumerators[e]);
		}
		for (u32 s = parsingResults[f].structStart; s < parsingResults[f].structEnd; ++s) {
			writeStruct(outputFile, structParser->structs[s]);
		}
	}
	writeFunctionHeader(outputFile);
	writeEnumeratorHelperFunction(outputFile);
	writeFormatingFunction(outputFile);
	fclose(outputFile);
}
