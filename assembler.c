
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_projects/tree_map.h"

typedef enum Opcode{
    ADD,
    ADDI,
    SUB,
    SUBI,
    NOT,
    AND,
    OR,
	LA,
	SA,
    MOV,
    LI,
    LW,
    SW,
    BEQ,
    BNE,
    JUMP,
	NONE
} Opcode;

typedef enum InstructionType{
    RType,
	IType,
	JType
} InstructionType;

typedef struct instruction{
    Opcode opcode;
    InstructionType type;
    int rs;
    int rt;
    int rd;
    int immediate;
} instruction;

int cstring_compare(void *o1, void *o2){
    return strcmp((char*)o1, (char*)o2);
}

int labelToAddress(char* label, TreeMap *map)
{
    if(tree_map_contains(label, map)){
        return toInt(tree_map_get(label, map));
    }
    else{
    	assert(0);
    	return -1;
    }
}

// combine these two into 1 function.
void newDataLabel(char* label, int size, TreeMap *data_map, int data_address){
    // need to make a copy of this label because it is going to get overwritten and pointer  will be null
    int* address = malloc(sizeof(int));
    *address = data_address;
    char* newLabel = malloc(sizeof(char) * size);
    strcpy(newLabel, label);
    tree_map_put(newLabel, address, data_map);
    assert(tree_map_contains(label, data_map) == 1);
}
void newInstructionLabel(char* label, int size, TreeMap *im_map, int instruction_address){
	assert(label[0] == ':'); // make sure we are passed the full label.
	// we need to handle this correctly.
	label = label + 1;
	// increment the label so that when we call get ... it dosnt have to deal with it
	// the question is whether this shud be handled by this function or not...
	// seems kinda tightly coupled to do it this way.
	int* address = malloc(sizeof(int));
    *address = instruction_address;
    char* newLabel = malloc(sizeof(char) * size);
    strcpy(newLabel, label);
    tree_map_put(newLabel, address, im_map);
    assert(tree_map_contains(label, im_map) == 1);
}

int getline(char* buffer, int* size, FILE* file)
{
    int c;
    int index = 0;
    while((c = fgetc(file)) != 10 && c != -1)
    {
        buffer[index] = c;
        index++;
    }
    buffer[index] = 0;
    if(c != -1)
    {
        *size = index;
        return 1;
    }
    *size = 0;
    return 0;
}

// will have the change this to take only the number of characters on the right.
Opcode stringToOpcode(char* c){
    if(strncmp(c, "addi", 4) == 0){return ADDI;}
    else if(strncmp(c, "subi", 4) == 0){return SUBI;}
    else if(strncmp(c, "add", 3) == 0){return ADD;}
    else if(strncmp(c, "sub", 3) == 0){return SUB;}
    else if(strncmp(c, "not", 3) == 0){return NOT;}
    else if(strncmp(c, "and", 3) == 0){return AND;}
    else if(strncmp(c, "or", 3) == 0){return OR;}
    else if(strncmp(c, "la", 2) == 0){return LA;}
    else if(strncmp(c, "sa", 2) == 0){return SA;}
    else if(strncmp(c, "mov", 3) == 0){return MOV;}
    else if(strncmp(c, "li", 2) == 0){return LI;}
    else if(strncmp(c, "lw", 2) == 0){return LW;}
    else if(strncmp(c, "sw", 2) == 0){return SW;}
    else if(strncmp(c, "beq", 3) == 0){return BEQ;}
    else if(strncmp(c, "bne", 3) == 0){return BNE;}
    else if(strncmp(c, "jump", 4) == 0){return JUMP;}
    else
    {
    	assert(0);
    	return NONE;
    }
}

int toInt(void *data){
    return *((int*)data);
}

void setInstruction(char* s, instruction *i, TreeMap *im_map, TreeMap *data_map)
{
    // ah we have to figure out which ones need the inst map or data map.
    char dummy_string[10];
    i->opcode = stringToOpcode(s);
    if(i->opcode == JUMP){
    	i->type = JType;
        char immediate_string[10];
        sscanf(s, "%s %s", dummy_string, immediate_string);
        if(tree_map_contains(immediate_string, im_map)){
            i->immediate = toInt(tree_map_get(immediate_string, im_map));
        } else {
        	printf("%s", immediate_string);
            assert(0);
        }
    }
    else if(i->opcode == ADDI || i->opcode == SUBI){
		i->type = IType;
		sscanf(s, "%s %d %d %d", dummy_string, &(i->rs), &(i->rt), &(i->immediate));
    }
    else if(i->opcode == LI){
    	i->type = IType;
        sscanf(s, "%s %d %d", dummy_string, &(i->rt), &(i->immediate));
    }
    else if(i->opcode == SW || i->opcode == LW || i->opcode == MOV){
    	i->type = IType;
    	sscanf(s, "%s %d %d", dummy_string, &(i->rs), &(i->rt));
    }
    else if(i->opcode == BEQ || i->opcode == BNE){
    	i->type = IType;
		char immediate_string[10];
		sscanf(s, "%s %d %d %s", dummy_string, &(i->rs), &(i->rt), immediate_string);
		if(tree_map_contains(immediate_string, im_map)){
			i->immediate = toInt(tree_map_get(immediate_string, im_map));
		} else {
			// we dont have this label mapped!
			assert(0);
		}
    }
    else if(i->opcode == LA || i->opcode == SA){
    	i->type = IType;
		char immediate_string[10];
		sscanf(s, "%s %d %s", dummy_string, &(i->rt), immediate_string);
		if(tree_map_contains(immediate_string, data_map)){
			i->immediate = toInt(tree_map_get(immediate_string, data_map));
		} else {
			// we dont have this label mapped!
			assert(0);
		}
    }
    else{
    	i->type = RType;
        sscanf(s, "%s %d %d %d", dummy_string, &(i->rs), &(i->rt), &(i->rd));
    }
}

//we gonna need to use a map for memory location lookups. shud not be hard, but i do want to use java...
// but if we use c then we can use all our shit...

int nextLineEquals(FILE* file, const char* nextLine, int length){
    char* buffer = malloc(sizeof(char) * length);
    int size;
    while(getline(buffer, &size, file)){
        if(size){
            if(strncmp(nextLine, buffer, length) == 0){
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

// so going to use fprintf and print the shit as "%x" ...ezpz
short instructionToHexString(instruction *i){
	unsigned short hexString = 0;
	if(i->type == JType)
	{
		hexString = i->opcode;
		hexString = hexString << 12;
		hexString = hexString | i->immediate;
	}
	else if(i->type == IType){
		hexString = i->opcode;
		hexString  = hexString << 3;
		hexString = hexString | i->rs;
		hexString  = hexString << 3;
		hexString = hexString | i->rt;
		hexString  = hexString << 6;
		hexString = hexString | i->immediate;
	}
	else if(i->type == RType){
		hexString = i->opcode;
		hexString  = hexString << 3;
		hexString = hexString | i->rs;
		hexString  = hexString << 3;
		hexString = hexString | i->rt;
		hexString = hexString << 3;
		hexString = hexString | i->rd;
		hexString = hexString << 3;
	}
	else{
		printf("%d %d %d %d", i->type, RType, IType, JType);
		assert(0);
		hexString = 0xFFFF;
	}
	return hexString;
}

int main(void)
{
	// we shud really use the instruction type as a field in the struct
	// R I J
	// wud make switch statements real nice ... no if( blah blah blah weird shit)
	// for long term use, this shud really be done in java
	// but still writing in c and java will be similar ... can switch it to java once
	// written in c.
	int code_index;
	int data_index;

    TreeMap *im_map = tree_map_constructor(&cstring_compare);
    TreeMap *data_map = tree_map_constructor(&cstring_compare);

    int size;
    FILE* assembly = fopen("C:\\Users\\Brian\\Desktop\\code.txt", "r");
    FILE* hex_code = fopen("C:\\Users\\Brian\\Desktop\\hex_code.hex", "w");
    char data_string[100];
    char code_string[100];

    int code_address = 0;
    int data_address = 0;

    if(nextLineEquals(assembly, "-data", 5))
    {
    	data_index = ftell(assembly);
        do
        {
            getline(data_string, &size, assembly);
            if(strncmp(data_string, "-code", 5) != 0){
                newDataLabel(data_string, size, data_map, data_address);
                data_address++;
            } else {
            	code_index = ftell(assembly);
            }
        }
        while(strncmp(data_string, "-code", 5) != 0);

        // our get line function shud ignore blank lines with whitespace
        while(getline(code_string, &size, assembly)){
        	// this shud not be contains ... but rather is the first character in the only
        	// string equal to ':'
        	if(isLabel(code_string)){
        		if(tree_map_contains(code_string, im_map) == 0){
        			newInstructionLabel(code_string, strlen(code_string), im_map, code_address);
				}
        	}
        	code_address++;
        }
    }

    // no also need to detect if it is an integer or not
    // so addi and subi shud be treated differently than the rest
    // and lw sw shud also be treated differently, maybe getting a new intruction type.
    instruction i;
    int hex;
    fseek(assembly, code_index, SEEK_SET);
    while(getline(code_string, &size, assembly))
    {
    	if(isLabel(code_string) == 0){
    		setInstruction(code_string, &i, im_map, data_map);
    		printf("%d\n", i.opcode);
			hex = instructionToHexString(&i);
			writeHexString(hex_code, hex);
    	}

    }

    fclose(hex_code);
    fclose(assembly);
    return 1;
}

void writeHexString(FILE* file, short hex)
{
	fprintf(file, "%x", ((hex & 0xF000) >> 12));
	fprintf(file, "%x", ((hex & 0x0F00) >> 8));
	fprintf(file, "%x", ((hex & 0x00F0) >> 4));
	fprintf(file, "%x\n", hex & 0x000F);
}

int isLabel(char* s){
	return s[0] == ':';
}
