#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define ERR_LENGTH -1
#define ERR_TAG -2
#define ERR_OPEN -3
#define ERR_QUOTES -4
#define ERR_COMMENT -5
#define ERR_PROLOG -6
#define ERR_WHITESPACE -7
#define ERR_ATTRIBUTES -8

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
int parse(string parsable);

int parseTag(string tag);
int parseAttributes(string attributes);

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
			if(!parse(parsable.str()))
			{
				cout << "\n----------\nSUCCESS\n----------" << endl;
			}
			else
			{
				cout << "\n----------\nERROR\n----------" << endl;
			}
		}
		else
		{
			cout << "\n----------\nerror: " << error_code << "\n----------" << endl;
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
		
	stringstream tag_builder;
	if(len < 4)
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
					if(parsable.tellp() > 0)
					{
						prolog = 3;
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
		if(comment)
			continue;
		else
			parsable << source[i];
	}
	
	cout << "tag: " << tag << endl;
	cout << "open: " << open << endl;
	cout << "quotes: " << quotes << endl;
	cout << "comment: " << comment << endl;
	cout << "prolog: " << prolog << endl;
	cout << "whitespace: " << whitespace << endl;
	
	if(tag)
		return ERR_TAG;
	else if(open)
		return ERR_OPEN;
	else if(quotes)
		return ERR_QUOTES;
	else if(prolog && prolog != 2)
		return ERR_PROLOG;
	else if(whitespace)
		return ERR_WHITESPACE;
	else
		return 0;
}

int parse(string source)
{
	int tag;
	int open;
	int quotes;
	int comment;
	int prolog;
	int whitespace;
	size_t len = source.length();
	
	tag = 0;
	open = 0;
	quotes = 0;
	comment = 0;
	prolog = 0;
	whitespace = 0;
	
	stringstream tag_builder;
	
	for(int i = 0; i < len; i++)
	{
		if(tag)
			tag_builder << source[i];
		if(source[i] == '<' && !quotes)
		{
			tag++;
			tag_builder << source[i];
		}
		else if(source[i] == '>' && !quotes)
		{
			tag--;
			if(parseTag(tag_builder.str()))
			{
				tag_builder.str(string());
				tag_builder.clear();
			}
			else
				return 1;
		}
		else if(source[i] == '\'')
		{
			if(!quotes)
			{
				quotes++;
			}
			else if(quotes == 1)
			{
				quotes--;
			}
		}
		else if(source[i] == '"')
		{
			if(!quotes)
			{
				quotes += 2;
			}
			else if(quotes == 2)
			{
				quotes -= 2;
			}
		}
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
	else if(whitespace)
		return ERR_WHITESPACE;
	else
		return 0;
}

int parseTag(string tag)
{
	stringstream name_builder;
	stringstream attr_builder;
	string tag_name;
	string tag_type;
	size_t len = tag.length();
	
	if(tag[1] == '/')
	{
		tag_type = "closing";
		int i = 2;
		while(tag[i] != ' ' && tag[i] != '>')
		{
			name_builder << tag[i++];
		}
	}
	else if(tag[1] != '?')
	{
		if(tag[len - 2] == '/')
			tag_type = "single";
		else
			tag_type = "opening";
		int i = 1;
		while(tag[i] != ' ' && tag[i] != '>' && tag[i] != '/')
		{
			if(tag[i] == '=' || tag[i] == '\'' || tag[i] == '"')
				return 0;
			name_builder << tag[i++];
		}
		
		while(isWhitespace(tag[i]))
			i++;
		
		if(tag[i] != '>')
		{
			if(tag[i] == '/')
				i++;
			while(tag[i] != '>' && tag[i] != '/')
			{
				attr_builder << tag[i++];
			}
		}
	}
	else if(tag[1] == '?')
	{
		tag_type = "prolog";
		int i = 2;
		while(tag[i] != ' ' && tag[i] != '?')
		{
			name_builder << tag[i++];
		}
		
		while(isWhitespace(tag[i]))
			i++;
		
		if(tag[i] != '?')
		{
			if(tag[i] == '?')
				i++;
			while(tag[i] != '>' && tag[i] != '?')
			{
				attr_builder << tag[i++];
			}
		}
	}
	
	cout << "tag:" << endl;
	
	tag_name = name_builder.str();
	cout << "    type: " << tag_type << endl;
	cout << "    name: " << name_builder.str() << endl;
	if(attr_builder.tellp() > 0)
	{
		cout << "    attributes:" << endl;
		string attributes = attr_builder.str();
		int attr_parse = parseAttributes(attributes);
		if(!attr_parse)
		{
			cout << "error while parsing attributes" << endl;
			return 0;
		}
	}
	
	return 1;
}

int parseAttributes(string attributes)
{
	int nam = 0;
	int equ = 0;
	int val = 0;
	
	stringstream name_builder;
	stringstream value_builder;
	
	vector<attr_t> attrs;
	
	size_t len = attributes.length();
	
	int error = 0;
	int i = 0;
	if(len < 4)
		return 0;
	while(i < len)
	{
		while(isWhitespace(attributes[i]) && i < len - 1)
		{
			i++;
		}
		
		if(i < len - 1)
		{
			if(nam == equ && equ == val)
			{
				// get the name
				while(!isWhitespace(attributes[i]) && attributes[i] != '=' 
					&& attributes[i] != '\'' && attributes[i] != '"' && i < len - 1)
				{
					name_builder << attributes[i++];
				}
				
				if(name_builder.tellp() > 0)
				{
					nam++;
				}
				
				while(isWhitespace(attributes[i]) && i < len - 1)
					i++;
				
				if(attributes[i] == '=')
				{
					if(!nam)
					{
						error++;
						break;
					}
					equ++;
					i++;
				}
				else if(attributes[i] == '\'' || attributes[i] == '"')
				{
					error++;
					break;
				}
				
				while(isWhitespace(attributes[i]) && i < len - 1)
					i++;
				
				if(attributes[i] == '\'')
				{
					i++;
					while(attributes[i] != '\'' && i < len - 1)
					{
						value_builder << attributes[i++];
					}
					if(attributes[i] == '\'')
					{
						val++;
					}
					else
					{
						error++;
						break;
					}
				}
				else if(attributes[i] == '"')
				{
					i++;
					while(attributes[i] != '"' && i < len - 1)
					{
						value_builder << attributes[i++];
					}
					if(attributes[i] == '"')
					{
						val++;
					}
					else
					{
						error++;
						break;
					}
				}
			}
		}
		
		if(nam == equ && equ == val)
		{
			if(nam)
			{
				cout << "        attribute:" << endl;
				cout << "            name: " << name_builder.str() << endl;
				cout << "            value: " << value_builder.str() << endl;
				nam = 0;
				equ = 0;
				val = 0;
				name_builder.str(string());
				name_builder.clear();
				value_builder.str(string());
				value_builder.clear();
			}
		}
		else
		{
			error++;
			break;
		}
		
		i++;
	}
	
	if(nam != equ || equ != val)
		return 0;
	else if(error)
		return 0;
	else
	{
		
		return 1;
	}
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
