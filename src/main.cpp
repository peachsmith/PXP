#include <iostream>
#include <fstream>
#include <sstream>

#define ERR_LENGTH -1
#define ERR_TAG -2
#define ERR_OPEN -3
#define ERR_QUOTES -4
#define ERR_COMMENT -5
#define ERR_PROLOG -6
#define ERR_WHITESPACE -7

using namespace std;

struct attr_t
{
	string name;
	string value;
};

struct elem_t
{
	string name;
	string text;
	int child_capacity;
	int child_count;
	int attr_capacity;
	int attr_count;
	elem_t* children;
	attr_t* attributes;
};

int isWhitespace(char c);
int validate(string source, stringstream& parsable);

void resizeAttributes(elem_t* parent);
void addAttribute(elem_t* parent, attr_t* attribute);
void resizeChildElements(elem_t* parent);
void addChildElement(elem_t* parent, elem_t* child);

int main()
{	
	ifstream config;
	
	config.open("configuration.xml");

	if(config.is_open())
	{	
		stringstream parsable;
		stringstream source_stream;
		source_stream << config.rdbuf();
		string source = source_stream.str();
		
		int error_code = validate(source, parsable);
		
		if(error_code == 0)
		{
			cout << parsable.rdbuf() << endl;
		}
		else
		{
			cout << "error: " << endl;
		}
		
	}
	else
	{
		cout << "could not open file" << endl;
	}
	
	return 0;
}

int isWhitespace(char c)
{
	if(c == ' ' || c == '\t' || c == '\n')
		return 1;
	else
		return 0;
}

int validate(string source, stringstream& parsable)
{
	int tag;
	int open;
	int quotes;
	int comment;
	int prolog;
	int whitespace;
	size_t len = source.length();
		
	if(len > 0 && len < 4)
		return -1;
	
	tag = 0;
	open = 0;
	quotes = 0;
	comment = 0;
	prolog = 0;
	whitespace = 0;
	
	for(int i = 0; i < len; i++)
	{
		if(source[i] == '<' && !quotes && !comment)
		{
			if(isWhitespace(source[i + 1]))
			{
				whitespace++;
				break;
			}
			if(i < len - 4)
			{
				if(source[i + 1] == '!' && source[i + 2] == '-'
					&& source[i + 3] == '-')
				{
					comment++;
				}
			}
			if(!comment)
			{
				if(source[i + 1] == '/')
				{
					if(isWhitespace(source[i + 2]))
					{
						whitespace++;
						break;
					}
					open--;
				}
				else if(source[i + 1] == '?')
				{
					if(isWhitespace(source[i + 2]))
					{
						whitespace++;
						break;
					}
					prolog++;
				}
				else
					open++;
				tag++;
			}
		}
		else if(source[i] == '>' && !quotes && !comment)
		{
			if(source[i - 1] == '/')
				open--;
			else if(source[i - 1] == '?')
				prolog++;
			tag--;
		}
		else if(source[i] == '>' && comment)
		{
			if(i > 4)
			{
				if(source[i - 1] == '-' && source[i - 2] == '-'
					&& source[i - 3] != '!' && source[i - 4] != '!')
				{
					//i++;
					comment--;
					continue;
				}
			}
		}
		else if(source[i] == '"')
		{
			if(quotes == 2)
			{
				quotes -= 2;
			}
			else if(!quotes)
			{
				quotes += 2;
			}
			else if(quotes == 1)
			{
				// TODO stuff
			}
		}
		else if(source[i] == '\'')
		{
			if(quotes == 1)
			{
				quotes--;
			}
			else if(!quotes)
			{
				quotes++;
			}
			else if(quotes == 2)
			{
				// TODO stuff
			}
		}
		if((isWhitespace(source[i]) && !quotes && !comment && !tag && (open ^ 1)) || comment)
			continue;
		else
			parsable << source[i];
	}
	
	//cout << "tag: " << tag << endl;
	//cout << "open: " << open << endl;
	//cout << "quotes: " << quotes << endl;
	//cout << "comment: " << comment << endl;
	//cout << "prolog: " << prolog << endl;
	//cout << "whitespace: " << whitespace << endl;
	
	if(tag)
		return ERR_TAG;
	else if(open)
		return ERR_OPEN;
	else if(quotes)
		return ERR_QUOTES;
	else if(prolog && prolog != 2)
		return ERR_PROLOG;
	else
		return 0;
}

void resizeAttributes(elem_t* parent)
{
	int capacity = parent->attr_capacity + parent->attr_capacity / 2;
	attr_t* attributes = new attr_t[capacity];
	
	for(int i = 0; i < parent->attr_capacity; i++)
		attributes[i] = parent->attributes[i];
	
	delete[] parent->attributes;
	parent->attributes = attributes;
	parent->attr_capacity = capacity;
}

void addAttribute(elem_t* parent, attr_t* attribute)
{
	if(parent->attr_count == parent->attr_capacity)
		resizeAttributes(parent);
	
	parent->attributes[parent->attr_count++] = *attribute;
}

void resizeChildElements(elem_t* parent)
{
	int capacity = parent->child_capacity + parent->child_capacity / 2;
	elem_t* children = new elem_t[capacity];
	
	for(int i = 0; i < parent->child_capacity; i++)
		children[i] = parent->children[i];
	
	delete[] parent->children;
	parent->children = children;
	parent->child_capacity = capacity;
}

void addChildElement(elem_t* parent, elem_t* child)
{
	if(parent->child_count == parent->child_capacity)
		resizeChildElements(parent);
	
	parent->children[parent->child_count++] = *child;
}
