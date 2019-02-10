#include <cstdlib>
#include "Threaded_TCPListener.h"
#include "Test.h"

int main()
{
	Threaded_TCPListener listener("127.0.0.1", 54000);

	int response = listener.Init();

	if(response == 0)
	{
		std::cout << "Running\n";
		while(true)
		{
			listener.Run();
		}
	}
	else
		std::cerr << "Error:" << response << std::endl;

	system("pause");
}