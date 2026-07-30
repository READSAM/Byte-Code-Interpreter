#include<stdlib.h>
#include<stdio.h>
#include "memory.h"
#include "chunk.h"

void initChunk(Chunk* chunk)
{
    chunk->count=0;
    chunk->capacity=0;
    chunk->code=NULL;

    chunk->line_count=0;
    chunk->line_capacity=0;
    chunk->lines=NULL;

    initValueArray(&chunk->constants);
}
void freeChunk(Chunk* chunk){
    FREE_ARRAY(uint8_t , chunk->code, chunk->capacity);
    FREE_ARRAY(rle_form , chunk->lines, chunk->line_capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte,int line)
{
    if(chunk->capacity < chunk->count+1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity=GROW_CAPACITY(oldCapacity);
        chunk->code=GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        if(oldCapacity < 8)
         chunk->lines=GROW_ARRAY(rle_form, chunk->lines, oldCapacity,1); //rle
    } 

    chunk->code[chunk->count]=byte;
    chunk->count++;
    
    //rle

    if(chunk->line_capacity < chunk->line_count+1)
    {
        int oldCapacity = chunk->line_capacity;
        chunk->line_capacity=GROW_CAPACITY(oldCapacity);
        chunk->lines=GROW_ARRAY(rle_form, chunk->lines, oldCapacity,chunk->line_capacity); //rle 
    }
    else 
    {
        if(chunk->line_count>0 && chunk->lines[chunk->line_count-1].line_num==line)
        {
            chunk->lines[chunk->line_count-1].count++;
        }
        else
        {
            chunk->lines[chunk->line_count].line_num=line;
            chunk->lines[chunk->line_count].count=1;
            chunk->line_count++;
        }
    }

}

void writeConstant(Chunk* chunk, Value value,int line)
{
    int constantIndex=addConstant(chunk,value);
    if(constantIndex <= 255)
    {
        writeChunk(chunk,OP_CONSTANT,line);
        writeChunk(chunk,constantIndex,line);
    }
    else
    {
        writeChunk(chunk,OP_CONSTANT_LONG,line);
        writeChunk(chunk,(constantIndex>>16)&0xff,line);
        writeChunk(chunk,(constantIndex>>8)&0xff,line);
        writeChunk(chunk,(constantIndex)&0xff,line);
    }
}

int addConstant(Chunk* chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count-1;
}
