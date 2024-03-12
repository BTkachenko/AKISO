#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main (void)
{
	setuid(0);

	char* argv[] = {"/bin/bash", NULL};
	
	int status = execv(argv[0], argv);
	
	if(status == -1)
	{

	 printf("Status -1, Błąd!");
	}

	return 0;
}
