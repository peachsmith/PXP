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

const unsigned int TAG_OPEN = 1;
const unsigned int TAG_CLOSE = 2;
const unsigned int TAG_SINGLE = 3;
const unsigned int TAG_PROLOG = 4;

using namespace std;

struct attr_t
{
	string name;
	string value;
};

struct tag_t
{
	string name;
	int type;
	vector<attr_t*> attributes;
};

struct elem_t
{
	tag_t opening_tag;
	tag_t closing_tag;
	vector<string> text_content;
	vector<elem_t*> children;
};

int isWhitespace(char c);
int validate(string source, stringstream& parsable);
int parse(string parsable);

int parseTag(string tag_string, tag_t* tag);
int parseAttributes(string attr_string, vector<attr_t*>& attributes);

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
	stringstream text_builder;
	
	vector<string> text;
	
	for(int i = 0; i < len; i++)
	{
		if(tag)
			tag_builder << source[i];
		else if(source[i] != '<')
			text_builder << source[i];
		
		if(source[i] == '<' && !quotes)
		{
			text.push_back(text_builder.str());
			text_builder.str(string());
			text_builder.clear();
			tag++;
			tag_builder << source[i];
		}
		else if(source[i] == '>' && !quotes)
		{
			tag--;
			// parse the tag
			tag_t* tag_node = new tag_t;
			if(parseTag(tag_builder.str(), tag_node))
			{
				tag_builder.str(string());
				tag_builder.clear();
				
				cout << "tag: " << endl;
				cout << "    name: " << tag_node->name << endl;
				cout << "    type: ";
				
				if(tag_node->type == TAG_OPEN)
					cout << "OPEN" << endl;
				else if(tag_node->type == TAG_CLOSE)
					cout << "CLOSE" << endl;
				else if(tag_node->type == TAG_SINGLE)
					cout << "SINGLE" << endl;
				else if(tag_node->type == TAG_PROLOG)
					cout << "PROLOG" << endl;
				
				if(tag_node->attributes.size() > 0)
				{
					cout << "    attributes:" << endl;
					for(int i = 0; i < tag_node->attributes.size(); i++)
					{
						cout << "        attribute:" << endl;
						cout << "            name: " << tag_node->attributes[i]->name << endl;
						cout << "            value: " << tag_node->attributes[i]->value << endl;
						delete tag_node->attributes[i];
					}
				}
				delete tag_node;
			}
			else
			{
				delete tag_node;
				return 1;
			}
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
	
	if(text.size() > 0)
	{
		cout << "text:" << endl;
		for(int i = 0; i < text.size(); i++)
			cout << text[i] << endl;
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

int parseTag(string tag_string, tag_t* tag)
{
	stringstream name_builder;
	stringstream attr_builder;
	string tag_name;
	int tag_type;
	size_t len = tag_string.length();
	vector<attr_t*> attributes;
	string attr_string;
	
	if(tag_string[1] == '/')
	{
		tag_type = TAG_CLOSE;
		int i = 2;
		while(tag_string[i] != ' ' && tag_string[i] != '>')
		{
			name_builder << tag_string[i++];
		}
	}
	else if(tag_string[1] != '?')
	{
		if(tag_string[len - 2] == '/')
			tag_type = TAG_SINGLE;
		else
			tag_type = TAG_OPEN;
		int i = 1;
		while(tag_string[i] != ' ' && tag_string[i] != '>' && tag_string[i] != '/')
		{
			if(tag_string[i] == '=' || tag_string[i] == '\'' || tag_string[i] == '"')
				return 0;
			name_builder << tag_string[i++];
		}
		
		while(isWhitespace(tag_string[i]))
			i++;
		
		if(tag_string[i] != '>')
		{
			if(tag_string[i] == '/')
				i++;
			while(tag_string[i] != '>' && tag_string[i] != '/')
			{
				attr_builder << tag_string[i++];
			}
		}
	}
	else if(tag_string[1] == '?')
	{
		tag_type = TAG_PROLOG;
		int i = 2;
		while(tag_string[i] != ' ' && tag_string[i] != '?')
		{
			name_builder << tag_string[i++];
		}
		
		while(isWhitespace(tag_string[i]))
			i++;
		
		if(tag_string[i] != '?')
		{
			if(tag_string[i] == '?')
				i++;
			while(tag_string[i] != '>' && tag_string[i] != '?')
			{
				attr_builder << tag_string[i++];
			}
		}
	}
	
	tag_name = name_builder.str();
	
	tag->name = tag_name;
	tag->type = tag_type;
	
	if(attr_builder.tellp() > 0)
	{
		attr_string = attr_builder.str();
		int attr_parse = parseAttributes(attr_string, attributes);
		if(!attr_parse)
		{
			cout << "error while parsing attributes" << endl;
			return 0;
		}
		else
		{
			tag->attributes = attributes;
		}
	}
	
	return 1;
}

int parseAttributes(string attr_string, vector<attr_t*>& attributes)
{
	int nam = 0;
	int equ = 0;
	int val = 0;
	int error = 0;
	int i = 0;
	
	stringstream name_builder;
	stringstream value_builder;
	
	size_t len = attr_string.length();
	
	if(len < 4)
		return 0;
	
	while(i < len)
	{
		while(isWhitespace(attr_string[i]) && i < len - 1)
		{
			i++;
		}
		
		if(i < len - 1)
		{
			if(nam == equ && equ == val)
			{
				// get the name
				while(!isWhitespace(attr_string[i]) && attr_string[i] != '=' 
					&& attr_string[i] != '\'' && attr_string[i] != '"' && i < len - 1)
				{
					name_builder << attr_string[i++];
				}
				
				if(name_builder.tellp() > 0)
				{
					nam++;
				}
				
				while(isWhitespace(attr_string[i]) && i < len - 1)
					i++;
				
				if(attr_string[i] == '=')
				{
					if(!nam)
					{
						error++;
						break;
					}
					equ++;
					i++;
				}
				else if(attr_string[i] == '\'' || attr_string[i] == '"')
				{
					error++;
					break;
				}
				
				while(isWhitespace(attr_string[i]) && i < len - 1)
					i++;
				
				if(attr_string[i] == '\'')
				{
					i++;
					while(attr_string[i] != '\'' && i < len - 1)
					{
						value_builder << attr_string[i++];
					}
					if(attr_string[i] == '\'')
					{
						val++;
					}
					else
					{
						error++;
						break;
					}
				}
				else if(attr_string[i] == '"')
				{
					i++;
					while(attr_string[i] != '"' && i < len - 1)
					{
						value_builder << attr_string[i++];
					}
					if(attr_string[i] == '"')
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
				attr_t* attr = new attr_t;
				attr->name = name_builder.str();
				attr->value = value_builder.str();

				attributes.push_back(attr);
				
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
		return 1;
}
