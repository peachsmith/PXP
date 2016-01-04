#include <stdlib.h>

#include "pxp.h"

using namespace std;

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
		
		int error_code = peach::validate(source, parsable);
		
		if(error_code == 0)
		{
			peach::elem_t* root = peach::parse(parsable.str());
			if(root)
			{
				//peach::printElements(root, 0);
				vector<peach::elem_t*> speed = peach::getElementsByName(root, "speed");
				if(speed.size() > 0)
				{
					cout << "name: " << speed[0]->name << endl;
					vector<peach::attr_t*> units = peach::getAttributesByName(speed[0], "units");
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
			cout << "validation error: " << error_code << endl;
		}
		
	}
	else
	{
		cout << "could not open file" << endl;
	}
	
	return 0;
}
