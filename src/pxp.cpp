#include "pxp.h"

namespace peach
{

int isWhitespace(char c)
{
	if(c == ' ' || c == '\t' || c == '\n')
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int allWhitespace(std::string str)
{
	for(int i = 0; i < str.length(); i++)
	{
		if(!peach::isWhitespace(str[i]))
		{
			return 0;
		}
	}
	return 1;
}

int validate(std::string source, std::stringstream& parsable)
{
	int tag;
	int open;
	int quotes;
	int comment;
	int prolog;
	int whitespace;
	size_t len = source.length();
		
	std::stringstream tag_builder;

	if(len < 4)
	{
		return -1;
	}

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
			if(peach::isWhitespace(source[i + 1]))
			{
				// whitespace error
				whitespace++;
				break;
			}
			if(i < len - 4)
			{
				if(source[i + 1] == '!' && source[i + 2] == '-'
					&& source[i + 3] == '-')
				{
					// found the beginning of a comment
					comment++;
				}
			}
			if(!comment)
			{
				if(source[i + 1] == '/')
				{
					if(peach::isWhitespace(source[i + 2]))
					{
						// whitespace error
						whitespace++;
						break;
					}
					open--;
				}
				else if(source[i + 1] == '?')
				{
					// found the beginning of a prolog
					if(peach::isWhitespace(source[i + 2]))
					{
						// whitespace error
						whitespace++;
						break;
					}
					if(parsable.tellp() > 0)
					{
						// the prolog isn't the first thing in the file
						prolog = 3;
						break;
					}
					prolog++;
				}
				else
				{
					open++;
				}
				tag++;
			}
		}
		else if(source[i] == '>' && !quotes && !comment)
		{
			if(source[i - 1] == '/')
			{
				// found a closing tag
				open--;
			}
			else if(source[i - 1] == '?')
			{
				// found the end of the prolog
				prolog++;
			}
			tag--;
		}
		else if(source[i] == '>' && comment)
		{
			if(i > 4)
			{
				if(source[i - 1] == '-' && source[i - 2] == '-'
					&& source[i - 3] != '!' && source[i - 4] != '!')
				{
					// found the end of a comment
					comment--;
					continue;
				}
			}
		}
		else if(source[i] == '"')
		{
			// enter or exit double quotes
			if(quotes == 2)
			{
				quotes -= 2;
			}
			else if(!quotes)
			{
				quotes += 2;
			}
		}
		else if(source[i] == '\'')
		{
			// enter or exit single quotes
			if(quotes == 1)
			{
				quotes--;
			}
			else if(!quotes)
			{
				quotes++;
			}
		}

		if(comment)
		{
			// ignore comments
			continue;
		}
		else
		{
			parsable << source[i];
		}
	}
	
	if(tag)
	{
		return ERR_TAG;
	}
	else if(open)
	{
		return ERR_OPEN;
	}
	else if(quotes)
	{
		return ERR_QUOTES;
	}
	else if(prolog && prolog != 2)
	{
		return ERR_PROLOG;
	}
	else if(whitespace)
	{
		return ERR_WHITESPACE;
	}
	else
	{
		return 0;
	}
}

peach::elem_t* parse(std::string source)
{
	int tag;
	int open;
	int close; // unnecessary
	int quotes;
	int comment;
	int prolog;
	int whitespace;
	int error;
	int depth;
	int single; // unnecessary
	int root_element;
	size_t len = source.length();
	
	tag = 0;
	open = 0;
	close = 0;
	quotes = 0;
	comment = 0;
	prolog = 0;
	whitespace = 0;
	error = 0;
	depth = 0;
	single = 0;
	root_element = 0;
	std::stringstream tag_builder;
	std::stringstream text_builder;
	
	std::vector<tag_t*> tags;
	
	for(int i = 0; i < len; i++)
	{
		// stuff outside of tags is considered text content of the
		// previous open tag
		if(tag)
		{
			tag_builder << source[i];
		}
		else if(source[i] != '<' && open)
		{
			text_builder << source[i];
		}
		
		if(source[i] == '<' && !quotes)
		{
			if(text_builder.tellp() > 0)
			{
				int ti = tags.size() - 1;
				
				// find the most recent open tag that does not yet have a closing tag
				while(ti > -1)
				{
					if(tags[ti]->type == TAG_OPEN && tags[ti]->depth == open - 1)
					{
						break;
					}
					else
					{
						ti--;
					}
				}

				if(tags[ti]->type == TAG_OPEN)
				{
					// add the text content to the tag
					tags[ti]->text.push_back(text_builder.str());
				}
				else
				{
					std::cout << "could not find an open tag for the text data" << std::endl;
				}
				
				text_builder.str(std::string());
				text_builder.clear();
			}
			tag++;
			tag_builder << source[i];
		}
		else if(source[i] == '>' && !quotes)
		{
			tag--;
			// parse the tag
			peach::tag_t* tag_node = new tag_t;
			tag_node->type = 0;
			tag_node->depth = open;

			if(peach::parseTag(tag_builder.str(), tag_node))
			{
				tag_builder.str(std::string());
				tag_builder.clear();
				tags.push_back(tag_node);
				
				if(tag_node->type == TAG_OPEN)
				{
					if(tag_node->depth == 0)
					{
						if(!root_element)
						{
							root_element++;
						}
						else
						{
							// if there's another root element, destroy the tags and exit
							std::cout << "found more than one root element" << std::endl;
							for(int i = 0; i < tags.size(); i++)
							{
								if(tags[i]->attributes.size() > 0)
								{
									for(int j = 0; j < tags[i]->attributes.size(); j++)
										delete tags[i]->attributes[j];
								}
								delete tags[i];
							}
							return 0;
						}
					}
					open++;
				}
				else if(tag_node->type == TAG_CLOSE)
				{
					open--;
					close++; // unnecessary
				}
				else if(tag_node->type == TAG_SINGLE || tag_node->type == TAG_PROLOG)
				{
					single++; // unnecessary
				}
			}
			else
			{
				delete tag_node;
				tag = 1;
				break;
			}
		}
		else if(source[i] == '\'')
		{
			// enter or exit single quotes
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
			// enter or exit double quotes
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
	
	int tags_size = tags.size();
	
	peach::elem_t* root = 0;

	if(tags_size > 0)
	{
		root = new peach::elem_t;
		root->opening_tag = 0;
		root->closing_tag = 0;
		root->depth = 1;
		
		int index = 0;
		int element_parse = peach::parseElements(root, tags, index);
		
		if(!element_parse)
		{
			peach::destroyElements(root);
			error = ERR_ELEMENT;
		}

	}
	
	// destroy the prolog here since it will not be in the resulting element tree
	for(int i = 0; i < tags.size(); i++)
	{
		if(tags[i]->type == TAG_PROLOG)
		{
			if(tags[i]->attributes.size() > 0)
			{
				for(int j = 0; j < tags[i]->attributes.size(); j++)
					delete tags[i]->attributes[j];
			}
			delete tags[i];	
		}
	}
	
	if(tag || open || quotes || (prolog && prolog != 2) || whitespace || error)
	{
		return 0;
	}
	else
	{
		return root;
	}
}

std::vector<peach::elem_t*> getElementsByName(peach::elem_t* root, std::string name)
{
	std::vector<peach::elem_t*> elements;
	for(int i = 0; i < root->children.size(); i++)
	{
		if(root->children[i]->name == name)
			elements.push_back(root->children[i]);
	}
	return elements;
}

std::vector<peach::attr_t*> getAttributesByName(peach::elem_t* root, std::string name)
{
	std::vector<peach::attr_t*> attributes;
	for(int i = 0; i < root->attributes.size(); i++)
	{
		if(root->attributes[i]->name == name)
			attributes.push_back(root->attributes[i]);
	}
	return attributes;
}

void printElements(peach::elem_t* root, int indent)
{
	std::cout << std::setw(indent) << std::setfill('.') << "";
	std::cout << root->opening_tag->name << " ";
	int attr_size = root->opening_tag->attributes.size();

	if(attr_size > 0)
	{
		// print attributes
		std::cout << "[";
		for(int i = 0; i < attr_size; i++)
		{
			std::cout << root->opening_tag->attributes[i]->name;
			std::cout << " = ";
			std::cout << root->opening_tag->attributes[i]->value;
			if(i < attr_size - 1)
			{
				std::cout << ", ";
			}
		}
		std::cout << "]";
	}

	std::cout << std::endl;

	if(root->text.size() > 0)
	{
		// print text content
		for(int i = 0; i < root->text.size(); i++)
		{
			std::cout << std::setw(indent + 2) << std::setfill(' ') << "";
			std::cout << "text " << i << ": ";
			if(peach::allWhitespace(root->text[i]))
			{
				std::cout << "[whitespace]" << std::endl;
			}
			else
			{
				std::cout << root->text[i] << std::endl;
			}
		}
	}

	if(root->children.size() > 0)
	{
		// print child elements
		for(int i = 0; i < root->children.size(); i++)
		{
			peach::printElements(root->children[i], indent + 2);
		}
	}
}

void destroyElements(peach::elem_t* root)
{
	if(root->attributes.size() > 0)
	{
		// delete the attributes
		for(int i = 0; i < root->attributes.size(); i++)
		{
			delete root->attributes[i];
		}
	}

	if(root->opening_tag->type == TAG_OPEN)
	{
		// delete opening and closing tags if the opening
		// tag is actually an opening tag
		delete root->opening_tag;
		delete root->closing_tag;
	}
	else if(root->opening_tag->type == TAG_SINGLE)
	{
		// only delete the opening tag if it's a 
		// single tage, since opening and closing tags
		// point to the same memory location
		delete root->opening_tag;
	}

	if(root->children.size() == 0)
	{
		delete root;
	}
	else
	{
		// destroy the child elements
		for(int i = 0; i < root->children.size(); i++)
		{
			peach::destroyElements(root->children[i]);
		}
	}
}

int parseElements(peach::elem_t* root, std::vector<peach::tag_t*>& tags, int& index)
{
	while(index < tags.size())
	{
		if(tags[index]->type == TAG_OPEN)
		{
			if(!root->opening_tag)
			{
				root->name = tags[index]->name;
				root->attributes = tags[index]->attributes;
				root->text = tags[index]->text;
				root->opening_tag = tags[index];
			}
			else
			{
				// parse child elements
				while(index < tags.size() && tags[index]->name != root->opening_tag->name && tags[index]->type != TAG_CLOSE)
				{
					peach::elem_t* child = new peach::elem_t;
					child->opening_tag = 0;
					child->closing_tag = 0;
					child->depth = root->depth + 1;
					int child_parse = peach::parseElements(child, tags, index);
					
					if(child_parse)
					{
						root->children.push_back(child);
					}
					else
					{
						peach::destroyElements(child);
						return 0;
					}
				}
				continue;
			}
		}
		else if(tags[index]->type == TAG_CLOSE)
		{
			if(!root->closing_tag)
			{
				if(!root->opening_tag)
				{
					std::cout << "found closing tag without matching opening tag" << std::endl;
					return 0;
				}
				else if(root->opening_tag->name != tags[index]->name)
				{
					if(!root->closing_tag)
					{
						std::cout << "opening and closing tag names do not match" << std::endl;
						return 0;
					}
					else
					{
						return 1;
					}
				}
				
				root->closing_tag = tags[index++];
				return 1;
			}
		}
		else if(tags[index]->type == TAG_SINGLE)
		{
			if(!root->opening_tag && !root->closing_tag)
			{
				// for single-tag elements, the opening and
				// closing tag are the same
				root->name = tags[index]->name;
				root->attributes = tags[index]->attributes;
				root->text = tags[index]->text;
				root->opening_tag = tags[index];
				root->closing_tag = tags[index];
				index++;
				return 1;
			}
			else
			{
				// parse child elements
				while(index < tags.size() && tags[index]->name != root->opening_tag->name && tags[index]->type != TAG_CLOSE)
				{
					peach::elem_t* child = new peach::elem_t;
					child->opening_tag = 0;
					child->closing_tag = 0;
					child->depth = root->depth + 1;
					int child_parse = peach::parseElements(child, tags, index);
					
					if(child_parse)
					{
						root->children.push_back(child);
					}
					else
					{
						peach::destroyElements(child);
						return 0;
					}
				}
				continue;
			}
		}
		index++;
	}
	
	if(root->opening_tag && root->closing_tag && root->opening_tag->name == root->closing_tag->name)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int parseTag(std::string tag_string, peach::tag_t* tag)
{
	std::stringstream name_builder;
	std::stringstream attr_builder;
	std::string tag_name;
	int tag_type;
	size_t len = tag_string.length();
	std::vector<peach::attr_t*> attributes;
	std::string attr_string;
	
	if(tag_string[1] == '/')
	{
		// found a closing tag
		tag_type = TAG_CLOSE;
		int i = 2;
		//acquire the tag's name
		while(tag_string[i] != ' ' && tag_string[i] != '>')
		{
			name_builder << tag_string[i++];
		}
	}
	else if(tag_string[1] != '?')
	{
		if(tag_string[len - 2] == '/')
		{
			// found a single tag
			tag_type = TAG_SINGLE;
		}
		else
		{
			// found an opening tag
			tag_type = TAG_OPEN;
		}

		// acquire the tag's name
		int i = 1;
		while(tag_string[i] != ' ' && tag_string[i] != '>' && tag_string[i] != '/')
		{
			if(tag_string[i] == '=' || tag_string[i] == '\'' || tag_string[i] == '"')
			{
				return 0;
			}
			name_builder << tag_string[i++];
		}
		
		// skip whitespace
		while(peach::isWhitespace(tag_string[i]))
		{
			i++;
		}

		if(tag_string[i] != '>')
		{
			if(tag_string[i] == '/')
			{
				i++;
			}

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
		
		while(peach::isWhitespace(tag_string[i]))
		{
			i++;
		}

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
		int attr_parse = peach::parseAttributes(attr_string, attributes);
		if(!attr_parse)
		{
			std::cout << "error while parsing attributes" << std::endl;
			return 0;
		}
		else
		{
			tag->attributes = attributes;
		}
	}
	
	return 1;
}

int parseAttributes(std::string attr_string, std::vector<peach::attr_t*>& attributes)
{
	int nam = 0;
	int equ = 0;
	int val = 0;
	int error = 0;
	int i = 0;
	
	std::stringstream name_builder;
	std::stringstream value_builder;
	
	size_t len = attr_string.length();
	
	if(len < 4)
	{
		return 0;
	}

	while(i < len)
	{
		while(peach::isWhitespace(attr_string[i]) && i < len - 1)
		{
			i++;
		}
		
		if(i < len - 1)
		{
			if(nam == equ && equ == val)
			{
				// get the name
				while(!peach::isWhitespace(attr_string[i]) && attr_string[i] != '=' 
					&& attr_string[i] != '\'' && attr_string[i] != '"' && i < len - 1)
				{
					name_builder << attr_string[i++];
				}
				
				if(name_builder.tellp() > 0)
				{
					nam++;
				}
				
				while(peach::isWhitespace(attr_string[i]) && i < len - 1)
				{
					i++;
				}

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
				
				while(peach::isWhitespace(attr_string[i]) && i < len - 1)
				{
					i++;
				}

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
				peach::attr_t* attr = new peach::attr_t;
				attr->name = name_builder.str();
				attr->value = value_builder.str();

				attributes.push_back(attr);
				
				nam = 0;
				equ = 0;
				val = 0;
				name_builder.str(std::string());
				name_builder.clear();
				value_builder.str(std::string());
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
	{
		return 0;
	}
	else if(error)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

}