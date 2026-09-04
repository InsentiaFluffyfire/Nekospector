#include "Introspect.h"

#include "typedefs.h"
#include "utility.h"
#include "fstring.h"

#include <stdio.h>
#include <malloc.h>

#include <windows.h>

#define typePrefix "IntroDataType"
#define tabSize 8
#define enumFileName "IntroDataType.h"
#define outputFileName "IntrospectionData.h"

const char* basicTypeNames[] = { "char", "bool", "memIndex", "b32", "b64", "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64", "r32", "r64"};

#define fileFilter "*.h"
#define maxFiles 256
char filesToParse[maxFiles][MAX_PATH];
u8 fileCount = 0;
u8 fileStartOffsets[maxFiles] = { 0 };

static inline u32 maxU32(u32 a, u32 b) {
	return(a > b ? a : b);
}

static inline u32 minU32(u32 a, u32 b) {
	return(a < b ? a : b);
}

char* readWholeFile(const char* fileName) {
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

typedef enum FluffyTokenType {
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

typedef struct Tokenizer {
	char* at;
}Tokenizer;

static Tokenizer tokenizer = {
	.at = nullptr,
};

#define maxMemberCount 65536u
#define maxEnumeratorCount 256u

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

typedef struct MemberData{
	Token typeToken;
	Token identifierToken;
	u32 arraySize;
	MemberFlagBits flags;
}MemberData;

typedef struct StructParser {
	u32 braceCount;
	u32 arraySize;
	Token typeToken;
	Token identifierToken;
	FILE* outputFile;
	bool hasTypeToken;
	bool hasIdentifierToken;
	MemberFlagBits flags;
	u8 memberCount;
	u32 maxTypeLength;
	u32 maxIdentifierLength;
	u32 maxArrayCountLength;
	Token structToken;
	ModeFlagBits modeFlags;
	MemberData members[maxMemberCount];
}StructParser;

StructParser* structParser;

typedef struct EnumeratorValue {
	Token identifierToken;
	s32 value;
	bool hasIdentifierToken;
	bool hasValue;
}EnumeratorValue;

typedef struct EnumParser {
	u32 maxIdentifierLength;
	u32 maxValueLength;
	u32 currentEnumerators;
	EnumeratorValue enumerators[maxEnumeratorCount];
}EnumParser;

EnumParser enumParser;

static void initStructParser() {
	structParser->braceCount = 1;
	structParser->arraySize = 1;
	structParser->hasTypeToken = false;
	structParser->typeToken = (Token){ 0 };
	structParser->hasIdentifierToken = false;
	structParser->identifierToken = (Token){0};
	structParser->flags = MEMBER_FLAG_BIT_NONE;
	structParser->modeFlags = MODE_BIT_NONE;
}

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

//gets rid of white Space and comments
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
			structParser->modeFlags |= MODE_BIT_TUPPLE;
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
	fprintf(file, "#pragma once\n\n");

	fprintf(file, "typedef enum IntroDataType {\n");
	for (u32 e = 0; e< currentEnumValues; ++e) {
		writeEnumValue(file, enumValues[e]);
	}
	for (u32 e = 0; e< currentEnumValuesE; ++e) {
		writeEnumValue(file, enumValuesE[e]);
	}
	fprintf(file, "} IntroDataType;\n");
	fclose(file);
}

static void parseMember() {
	Token token = getToken();
	switch (token.type) {
		case Token_Semicolon: {
			if (structParser->hasIdentifierToken&& structParser->hasTypeToken) {
				if ((structParser->arraySize > 1)&&(structParser->modeFlags&MODE_BIT_TUPPLE)) {
					structParser->flags |= MEMBER_FLAG_BIT_TUPPLE;
				}
				structParser->members[structParser->memberCount] = (MemberData){
				  .arraySize			= structParser->arraySize,
				  .flags				= structParser->flags,
				  .typeToken			= structParser->typeToken,
				  .identifierToken		= structParser->identifierToken,
				};
				pushEnumValue(structParser->typeToken, (structParser->flags & MEMBER_FLAG_BIT_ENUM) ? true : false);
				structParser->flags = MEMBER_FLAG_BIT_NONE;
				++structParser->memberCount;
				structParser->maxArrayCountLength = maxU32(structParser->maxArrayCountLength, f_Log10(structParser->arraySize));
				structParser->maxIdentifierLength = maxU32(structParser->maxIdentifierLength, structParser->identifierToken.length);
				structParser->maxTypeLength = maxU32(structParser->maxTypeLength, structParser->typeToken.length);
				structParser->arraySize = 1;
				structParser->hasTypeToken = false;
				structParser->hasIdentifierToken = false;
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
			if (token.type == Token_Number) { structParser->arraySize *= tokenToU32(token); } //doesn't work with constants yet
			if (!requiredToken(Token_BracketClose)) { fprintf(stderr, "Error: Invalid Array Syntax!\n\n"); }
			parseMember();
		}break;
		case Token_Asterisk: {
			structParser->flags |= MEMBER_FLAG_BIT_PTR;
			parseMember();
		}break;
		case Token_Identifier: {
			if (!compareIdentifier(token, "struct")&&!compareIdentifier(token, "union")) {
				if (structParser->hasTypeToken) {
					structParser->identifierToken = token; structParser->hasIdentifierToken = true;
				} else {
					if (compareIdentifier(token, "enum")) {
						structParser->flags |= MEMBER_FLAG_BIT_ENUM;
					} else {
						structParser->typeToken = token; structParser->hasTypeToken = true;
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

static void writeMemberData(FILE* file, MemberData member) {
	fprintf(file, "\t{%s_%.*s,", typePrefix, member.typeToken.length, member.typeToken.text);
	for (u32 c = 0; c < maxU32(structParser->maxTypeLength, tabSize)-member.typeToken.length; ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " \"%.*s\",", member.identifierToken.length, member.identifierToken.text);
	for (u32 c = 0; c < maxU32(structParser->maxIdentifierLength, tabSize)-member.identifierToken.length; ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " %u,", member.arraySize);
	for (u32 c = 0; c < maxU32(structParser->maxArrayCountLength, tabSize)-f_Log10(member.arraySize); ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " offsetof(%.*s, %.*s),", structParser->structToken.length, structParser->structToken.text, member.identifierToken.length, member.identifierToken.text );
	for (u32 c = 0; c < maxU32(structParser->maxIdentifierLength, tabSize)-member.identifierToken.length; ++c) {
		fprintf(file, " ");
	}
	fprintf(file, " 0x%x },\n", member.flags);
}

static void parseStruct() {
	structParser->structToken = getToken();
	pushEnumValue(structParser->structToken, false);
	fprintf(structParser->outputFile,"MemberDefinition membersOf_%.*s[] = {\n", structParser->structToken.length, structParser->structToken.text);
	if (requiredToken(Token_BraceOpen)) {
		structParser->braceCount = 1;
		while (structParser->braceCount) {
			parseMember();
		}
		
	}
	for (u32 m = 0; m<structParser->memberCount; ++m) {
		writeMemberData(structParser->outputFile, structParser->members[m]);
	}
	initStructParser();
	structParser->maxArrayCountLength = 1;
	structParser->maxIdentifierLength = 0;
	structParser->maxTypeLength = 0;
	structParser->memberCount = 0;
	fprintf(structParser->outputFile, "};\n\n");
}

static void parseEnumerator() {
	Token token = getToken();
	switch (token.type) {
		case Token_Semicolon:
		case Token_FileEnd: {
		}break;
		case Token_Number: {
			const s32 value = tokenToS32(token);
			enumParser.enumerators[enumParser.currentEnumerators].value = value;
			enumParser.maxValueLength = maxU32(enumParser.maxValueLength, f_Log10(value < 0 ? -value : value)+(value < 0));
			if (requiredToken(Token_Comma)) {
				++enumParser.currentEnumerators;
				enumParser.enumerators[enumParser.currentEnumerators].hasIdentifierToken = false;
				parseEnumerator();
			} else {
				fprintf(stderr, "Error: Unexpected Token: %i", token.type);
			}
		}break;
		case Token_Comma: {
			enumParser.enumerators[enumParser.currentEnumerators].value = (s32)enumParser.currentEnumerators;
			enumParser.maxValueLength = maxU32(enumParser.maxValueLength, f_Log10(enumParser.currentEnumerators));
			++enumParser.currentEnumerators;
			enumParser.enumerators[enumParser.currentEnumerators].hasIdentifierToken = false;
			parseEnumerator();
		}break;
		case Token_Identifier: {
			if (!enumParser.enumerators[enumParser.currentEnumerators].hasIdentifierToken) {
				enumParser.enumerators[enumParser.currentEnumerators].identifierToken = token;
				enumParser.enumerators[enumParser.currentEnumerators].hasIdentifierToken = true;
				enumParser.maxIdentifierLength = maxU32(enumParser.maxIdentifierLength, token.length);
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
	for (u32 c = 0; c < maxU32(enumParser.maxIdentifierLength, tabSize)-e.identifierToken.length; ++c) {
		fprintf(file, " ");
	}
	for (u32 c = 0; c < maxU32(enumParser.maxValueLength, tabSize)-(f_Log10(e.value < 0 ? -e.value : e.value)+(e.value < 0)); ++c) {
		fprintf(file, " ");
	}
	fprintf(file, "%i", e.value);
	fprintf(file, " },\n");
}

static void parseEnum() {
	enumParser.maxIdentifierLength = 0;
	enumParser.maxValueLength = 1;
	enumParser.currentEnumerators = 0;
	Token nameToken = getToken();
	pushEnumValue(nameToken, true);
	fprintf(structParser->outputFile, "EnumeratorDefinition enumeratorsOf_%.*s[] = {\n", nameToken.length, nameToken.text);
	if (requiredToken(Token_BraceOpen)) {
		parseEnumerator();
	}
	for (u32 e = 0; e < enumParser.currentEnumerators; ++e) {
		writeEnumerator(structParser->outputFile, enumParser.enumerators[e]);
	}
	enumParser.currentEnumerators = 0;
	enumParser.enumerators[0].hasIdentifierToken = false;
	fprintf(structParser->outputFile, "};\n\n");
}

static void parseIntrospectable() {
	if (requiredToken(Token_ParenOpen)) {
		parseIntrospectionParams();
		structParser->memberCount = 0;
		structParser->maxArrayCountLength = 1;
		structParser->maxIdentifierLength = 0;
		structParser->maxTypeLength = 0;
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

//TODO:
// - proper handling of arrays,
// - proper handling of strings,

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
	fprintf(file, "\n\t\t\t//Basic types:\n");
	for (u32 i = 0; i< ArrayCount(basicTypeNames); ++i) {
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
	fprintf(file, "#pragma once\n\n");
	fprintf(file, "#include \"typedefs.h\"\n");
	fprintf(file, "#include \"utility.h\"\n");
	fprintf(file, "#include \"fstring.h\"\n");
	fprintf(file, "#include \"platformMemory.h\"\n\n");
	fprintf(file, "#include \"%s\"\n\n", enumFileName);
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
	fprintf(file, "\t u32            count;\n");
	fprintf(file, "\t const char*    identifier;\n");
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
	char* outEnumFileName = enumFileName;
	char outFileDirectory[MAX_PATH];
	memset(outFileDirectory, 0, MAX_PATH);
	u32 outPathLength = 0;
	structParser = malloc(sizeof(StructParser));
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
						memcpy(filesToParse[fileCount], token.text, length);
						AppendCString(&filesToParse[fileCount][length], fileData.cFileName);
						++fileCount;
						while (FindNextFileA(searchHandle, &fileData)) {
							memcpy(filesToParse[fileCount], token.text, length);
							AppendCString(&filesToParse[fileCount][length], fileData.cFileName);
							++fileCount;
						}
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
	AppendCString(&outFileDirectory[outPathLength], outEnumFileName);
	FILE* outEnumFile = fopen(outFileDirectory, "wb");
	initStructParser();
	setOutputFile(outputFile);
	initEnumValues();
	printFileHeader(outputFile);
	printMemberDefintion(outputFile);
	printEnumeratorDefintion(outputFile);
	LARGE_INTEGER startMarker;
	LARGE_INTEGER endMarker;
	QueryPerformanceCounter(&startMarker);
	for (u32 f = 0; f < fileCount; ++f) {
		fileStartOffsets[f] = structParser->memberCount;
		char* file = readWholeFile(filesToParse[f]);
		initTokenizer(file);
		const char* fileName = filesToParse[f];
		printDataHeader(outputFile, fileName);
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
						//printf("Match found!\n");
						parseIntrospectable();
					}
				}break;
				default: {
				}break;
			}
		}
	fileEnd:
		printf("End of file reached!\n");
	}
	writeEnumeratorHelperFunction(outputFile);
	writeFormatingFunction(outputFile);
	writeEnumFile(outEnumFile);
	fclose(outputFile);
	QueryPerformanceCounter(&endMarker);
	r64 time = ((r64)endMarker.QuadPart-(r64)startMarker.QuadPart)/(r64)clockFrequency.QuadPart;
	if (time<1.0f) {
		time *= 1000.0f;
		printf("Time taken: %.2fms\n\n", time);
	} else {
		printf("Time taken: %.2fs\n\n", time);
	}
}
