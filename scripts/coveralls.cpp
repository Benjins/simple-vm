#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* GetCoverageForFile(char* fileName);

int main(int argc, char** argv){	
	char* outBuffer = (char*)malloc(1024*1024*20);
	char* outCursor = outBuffer;
	
	char header[256];
	memset(header, 0, 256);
	sprintf(header, "{\n\
	\"repo_token\": \"%s\"\n\
	\"source_files\": \n[\n", argv[1]);
	char* footer = "\n]\n}";
	
	outCursor += sprintf(outCursor, header);
  
	for(int i = 2; i < argc - 1; i += 2){
		if(i != 2){
			outCursor += sprintf(outCursor, ",\n");
		}
		
		char* fileName = argv[i];
		char* hash = argv[i + 1];
		printf("The file is named '%s'.\n", fileName);
		
		char sourceFileFormat[256] = "{\n\
		  \"name\": \"%s\",\n\
		  \"source_digest\": \"%s\",\n\
		  \"coverage\": %s\n\
		}";
		
		char* fileCoverage = GetCoverageForFile(fileName);
		outCursor += sprintf(outCursor, sourceFileFormat, fileName, hash, fileCoverage);
		free(fileCoverage);
	}
	
	outCursor += sprintf(outCursor, footer);
	
	char buff[256];
	
	FILE* fileOut = fopen("coveralls.json", "wb");
	fwrite(outBuffer, 1, outCursor - outBuffer, fileOut);
	fclose(fileOut);
	
	return 0;
}

char* GetCoverageForFile(char* fileName){
	FILE* fileIn = fopen(fileName, "rb");
	if(fileIn == NULL){
		char* dummyRet = (char*)malloc(1);
		dummyRet[0] = '\0';
		return dummyRet;
	}
	
	fseek(fileIn, 0, SEEK_END);
	int fileSize = ftell(fileIn);
	fseek(fileIn, 0, SEEK_SET);
	
	char* buffer = (char*)malloc(fileSize);
	fread(buffer, 1, fileSize, fileIn);
	
	char* outBuffer = (char*)malloc(fileSize);
	char* outCursor = outBuffer;
	int outSize = 0;
	
	outCursor += sprintf(outCursor, "[");
	
	char* cursor = buffer;
	while(cursor != NULL && cursor - buffer < fileSize){
		cursor += strspn(cursor, " \t\n\r");
		
		char* firstColon = strstr(cursor, ":");
		if(firstColon == NULL){
			break;
		}
		
		char* secondColon = strstr(firstColon + 1, ":");
		
		if(secondColon == NULL){
			break;
		}
		
		char* beforeSecondColon = secondColon - 1;
		char* twoBeforeSecondColon = secondColon - 2;
		if(beforeSecondColon[0] != '0' || twoBeforeSecondColon[0] != ' '){
			if(outCursor - outBuffer > 2){
				outCursor += sprintf(outCursor, ", ");
			}
			
			if(cursor[0] == '#'){
				outCursor += sprintf(outCursor, "0");
			}
			else if(cursor[0] == '-'){
				outCursor += sprintf(outCursor, "null");
			}
			else{
				int executionCount = atoi(cursor);
				outCursor += sprintf(outCursor, "%d", executionCount);
			}
		}
		
		cursor = strstr(cursor, "\n");
	}
	
	outCursor += sprintf(outCursor, "]");
	
	int trimmedSize = outCursor - outBuffer + 1;
	char* trimmedOutBuffer = (char*)malloc(trimmedSize);
	memcpy(trimmedOutBuffer, outBuffer, trimmedSize);
	trimmedOutBuffer[trimmedSize - 1] = '\0';
	
	free(buffer);
	fclose(fileIn);
	
	return trimmedOutBuffer;
}