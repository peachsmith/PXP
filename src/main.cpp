#include <stdlib.h>

#include "pxp.h"

using namespace std;

int main()
{	
	ifstream config;
	
	config.open("configuration.xml");

	if(config.is_open())
	{	
		using peach::attr_t;
		using peach::tag_t;
		using peach::elem_t;

		stringstream parsable;
		stringstream source_stream;
		string source;
		int parse_error;
		vector<elem_t*> speed;
		vector<attr_t*> units;

		source_stream << config.rdbuf();
		source = source_stream.str();
		
		parse_error = peach::validate(source, parsable);
		
		if(parse_error == 0)
		{
			peach::elem_t* root = peach::parse(parsable.str());
			if(root)
			{
				speed = peach::getElementsByName(root, "speed");

				if(speed.size() > 0)
				{
					cout << "name: " << speed[0]->name << endl;

					units = peach::getAttributesByName(speed[0], "units");

					if(units.size() > 0)
					{
						cout << "units: " << units[0]->value << endl;
					}
					if(speed[0]->text.size() > 0)
					{
						cout << "text: " << speed[0]->text[0] << endl;
					}
				}
				
				peach::destroyElements(root);
			}
			else
			{
				cout << "parsing error" << endl;
			}
		}
		else
		{
			cout << "validation error: " << parse_error << endl;
		}
		
	}
	else
	{
		cout << "could not open file" << endl;
	}
	
	return 0;
}
