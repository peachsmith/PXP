#ifndef PEACH_XML_PARSER_H
#define PEACH_XML_PARSER_H

#define ERR_LENGTH -1
#define ERR_TAG -2
#define ERR_OPEN -3
#define ERR_QUOTES -4
#define ERR_COMMENT -5
#define ERR_PROLOG -6
#define ERR_WHITESPACE -7
#define ERR_ATTRIBUTES -8
#define ERR_ELEMENT -9

#define TAG_OPEN 1
#define TAG_CLOSE 2
#define TAG_SINGLE 3
#define TAG_PROLOG 4

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

namespace peach
{

struct attr_t
{
	std::string name;
	std::string value;
};

struct tag_t
{
	std::string name;
	int type;
	int depth;
	std::vector<peach::attr_t*> attributes;
	std::vector<std::string> text;
};

struct elem_t
{
	std::string name;
	std::vector<peach::attr_t*> attributes;
	std::vector<std::string> text;
	std::vector<peach::elem_t*> children;
	tag_t* opening_tag;
	tag_t* closing_tag;
	int depth;
};

int isWhitespace(char c);
int allWhitespace(std::string str);
int validate(std::string source, std::stringstream& parsable);
peach::elem_t* parse(std::string parsable);

void printElements(peach::elem_t* root, int indent);
void destroyElements(peach::elem_t* root);
int parseElements(peach::elem_t* root, std::vector<peach::tag_t*>& tags, int& index);
int parseTag(std::string tag_string, peach::tag_t* tag);
int parseAttributes(std::string attr_string, std::vector<peach::attr_t*>& attributes);

}

#endif