#include "pxp.h"

using namespace std;
using peach::attr_t;
using peach::tag_t;
using peach::elem_t;
	
int main()
{	
	ifstream in_file;
	stringstream parse_stream;
	stringstream source_stream;
	string source;
	int parse_error;
	vector<elem_t*> speed;
	vector<attr_t*> units;
	
	in_file.open("configuration.xml");

	if(in_file.is_open())
	{	
		source_stream << in_file.rdbuf();
		source = source_stream.str();
		
		parse_error = peach::validate(source, parse_stream);
		
		if(parse_error == 0)
		{
			peach::elem_t* root = peach::parse(parse_stream.str());
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
		
		in_file.close();
	}
	else
	{
		cout << "could not open file" << endl;
	}
	
	return 0;
}
