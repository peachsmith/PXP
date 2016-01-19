/*  
    main.cpp
    a PXP dmeo application
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
#include "pxp.h"

using std::cout;
using std::endl;
	
int main()
{	
	std::ifstream in_file;
	std::stringstream source_stream;
	std::string source;
	peach::elem_t* root;
	std::vector<peach::elem_t*> speed;
	std::vector<peach::attr_t*> units;
	
	in_file.open("configuration.xml");

	if(in_file.is_open())
	{	
		source_stream << in_file.rdbuf();
		source = source_stream.str();
		
		root = peach::parse(source);

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
			cout << "could not parse file" << endl;
		}
		
		in_file.close();
	}
	else
	{
		cout << "could not open file" << endl;
	}
	
	return 0;
}
