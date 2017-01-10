// exceptions

//#include <iostream>
//using namespace std;
 void throw_critical_error_exception (char *err_str);

#pragma warning(disable: 4297) //disable warning for exeption throwing

void throw_critical_error_exception (char *err_str)
{
	while(1);
	//throw std::runtime_error(err_str);
}
