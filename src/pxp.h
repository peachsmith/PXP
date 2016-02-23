/*  
    pxp.h
	Peach XML Parser (PXP)
    Copyright (C) 2016 John E. Powell

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef PXP_H_
#define PXP_H_

/* error codes */
#define ERR_LENGTH -1
#define ERR_TAG -2
#define ERR_OPEN -3
#define ERR_QUOTES -4
#define ERR_COMMENT -5
#define ERR_PROLOG -6
#define ERR_WHITESPACE -7
#define ERR_ATTRIBUTES -8
#define ERR_ELEMENT -9

/* tag types */
#define TAG_OPEN 1
#define TAG_CLOSE 2
#define TAG_SINGLE 3
#define TAG_PROLOG 4

/* escape codes */
#define ESCAPE_AMPERSAND 5
#define ESCAPE_LESS_THAN 6
#define ESCAPE_GREATER_THAN 7
#define ESCAPE_QUOTE 8
#define ESCAPE_APOSTROPHE 9

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
};

int isWhitespace(char c);
int allWhitespace(std::string str);
int validate(std::string input, std::stringstream& parsable);
peach::elem_t* parse(std::string input);
std::vector<peach::elem_t*> getElementsByName(peach::elem_t* root, std::string name);
std::vector<peach::attr_t*> getAttributesByName(peach::elem_t* root, std::string name);

void printElements(peach::elem_t* root, int indent);
void destroyElements(peach::elem_t* root);
int parseElements(peach::elem_t* root, std::vector<peach::tag_t*>& tags, int& index);
int parseTag(std::string tag_string, peach::tag_t* tag);
int parseAttributes(std::string attr_string, std::vector<peach::attr_t*>& attributes);

} /* namespace peach */

#endif