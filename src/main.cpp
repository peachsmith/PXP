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
				peach::printElements(root, 0);
				peach::destroyElements(root);
				cout << "======="
					<< endl << "success"
					<< endl << "=======" << endl;
			}
			else
			{
				cout << "=============" << endl 
					<< "parsing error" << endl 
					<< "=============" << endl;
			}
		}
		else
		{
			cout << "=====================" << endl 
				<< "validation error: " << error_code 
				<< endl << "=====================" << endl;
		}
		
	}
	else
	{
		cout << "could not open file" << endl;
	}
	
	return 0;
}
