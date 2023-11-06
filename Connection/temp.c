#include "headers.h"

void sendRequest(char *input) {
    char request[4096];
    strcpy(request, input);
    char req2[4096];
    int i;

    for (i = 1; request[i] != '\0'; i++) {
        req2[i - 1] = request[i];
    }
    
    req2[i-2] = '\0';
    
    printf("x-%s-x\n", req2);
}

int main()
{
	while (1)
	{
		char *input = malloc(sizeof(char) * 4096);
		if (fgets(input, 4096, stdin) == NULL)
		{
			break;
		}
		sendRequest(input);
		free(input);
	}

	return 0;
}