
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_projects/tree_map.h"

static int data_memory_address;
static int instruction_memory_address;

typedef enum instruction_type{
    ADD,
    ADDI,
    SUB,
    SUBI,
    NOT,
    AND,
    OR,
    NAND,
    NOR,
    MOV,
    LI,
    LW,
    SW,
    BEQ,
    BNE,
    JUMP
} instruction_type;

typedef struct instruction{
    instruction_type opcode;
    int rs;
    int rt;
    int rd;
    int immediate;
} instruction;

int cstring_compare(void *o1, void *o2){
    return strcmp((char*)o1, (char*)o2);
}

void* labelToAddress(char* label, TreeMap *map)
{
    if(tree_map_contains(label, map)){
        return tree_map_get(label, map);
    }
}

int newDataLabel(char* label, int size, TreeMap *data_map){
    // need to make a copy of this label because it is going to get overwritten and pointer  will be null
    int* address = malloc(sizeof(int));
    *address = data_memory_address;
    char* newLabel = malloc(sizeof(char) * size);
    strncpy(newLabel, label, size);
    tree_map_put(newLabel, address, data_map);
    data_memory_address++;
    return data_memory_address-1;
}

int newInstructionLabel(char* label, int size, TreeMap *im_map){
    int* address = malloc(sizeof(int));
    *address = instruction_memory_address;
    char* newLabel = malloc(sizeof(char) * size);
    strncpy(newLabel, label, size);
    tree_map_put(newLabel, address, im_map);
    instruction_memory_address++;
    return instruction_memory_address-1;
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
instruction_type stringToOpcode(char* c){
    if(strncmp(c, "addi", 4) == 0){return ADDI;}
    else if(strncmp(c, "subi", 4) == 0){return SUBI;}
    else if(strncmp(c, "add", 3) == 0){return ADD;}
    else if(strncmp(c, "sub", 3) == 0){return SUB;}
    else if(strncmp(c, "not", 3) == 0){return NOT;}
    else if(strncmp(c, "and", 3) == 0){return AND;}
    else if(strncmp(c, "or", 3) == 0){return OR;}
    else if(strncmp(c, "nand", 3) == 0){return NAND;}
    else if(strncmp(c, "nor", 3) == 0){return NOR;}
    else if(strncmp(c, "mov", 3) == 0){return MOV;}
    else if(strncmp(c, "li", 2) == 0){return LI;}
    else if(strncmp(c, "lw", 2) == 0){return LW;}
    else if(strncmp(c, "sw", 2) == 0){return SW;}
    else if(strncmp(c, "beq", 3) == 0){return BEQ;}
    else if(strncmp(c, "bne", 3) == 0){return BNE;}
    else{return JUMP;}
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
        char immediate_string[10];
        sscanf(s, "%s %s", dummy_string, immediate_string);
        if(tree_map_contains(immediate_string, im_map)){
            i->immediate = toInt(tree_map_get(immediate_string, im_map));
        } else {
            i->immediate = newInstructionLabel(immediate_string, strlen(immediate_string), im_map);
        }
    }
    else if(i->opcode > NOR || i->opcode == ADDI || i->opcode == SUBI){
        char immediate_string[10];
        sscanf(s, "%s %d %d %s", dummy_string, &(i->rs), &(i->rt), immediate_string);
        if(tree_map_contains(immediate_string, im_map)){
            i->immediate = toInt(tree_map_get(immediate_string, im_map));
        } else {
            i->immediate = newInstructionLabel(immediate_string, strlen(immediate_string), im_map);
        }
    }
    else{
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

int main(void)
{
	int code_index;
	int data_index;
    TreeMap *im_map = tree_map_constructor(&cstring_compare);
    TreeMap *data_map = tree_map_constructor(&cstring_compare);

    int size;
    FILE* assembly = fopen("C:\\Users\\Brian\\Desktop\\code.txt", "r");
    char data_string[100];

    if(nextLineEquals(assembly, "-data", 5)){
        do{
            getline(data_string, &size, assembly);
            if(strncmp(data_string, "-code", 5) != 0){
                newDataLabel(data_string, size, data_map);
            } else {
            	data_index = ftell(assembly);
            }
        } while(strncmp(data_string, "-code", 5) != 0);
        code_index = ftell(assembly);
    }

    char instruction_string[100];
    instruction i;
    while(getline(instruction_string, &size, assembly))
    {
        setInstruction(instruction_string, &i, im_map, data_map);
    }

    fclose(assembly);

    return 1;
}
