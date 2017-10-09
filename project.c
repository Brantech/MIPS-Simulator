#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
     switch(ALUControl) {
     // Add
     case 0:
          *ALUresult = A + B;
          break;
     // Subtract
     case 1:
          *ALUresult = A - B;
          break;
     // Set less than
     case 2:
          *ALUresult = (int) A < (int) B ? 1 : 0;
          break;
     // Set less than unsigned
     case 3:
          *ALUresult = A < B ? 1 : 0;
          break;
     // And
     case 4:
          *ALUresult = A & B;
          break;
     // Or
     case 5:
          *ALUresult = A | B;
          break;
     // Shift B left 16
     case 6:
          *ALUresult = B << 16;
          break;
     // Not A
     case 7:
          *ALUresult = ~A;
          break;
     }
     *Zero = *ALUresult == 0 ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
     if(PC % 4 != 0) return 1;
     *instruction = Mem[PC / 4];
     return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
     unsigned *binary = (unsigned *) calloc(32, sizeof(unsigned));

     int i = 0, j = 1;
     while(instruction != 0) {
          binary[i++] = instruction % 2;
          instruction /= 2;
     }

     *op = *r1 = *r2 = *r3 = *funct = *offset = *jsec = 0;

     // Bits 31-26
     for(i = 26; i < 32; i++) {
          *op += binary[i] * j;
          j *= 2;
     }
     j = 1;

     // Bits 21-25
     for(i = 21; i < 26; i++) {
          *r1 += binary[i] * j;
          j *= 2;
     }
     j = 1;

     // Bits 16-20
     for(i = 16; i < 21; i++) {
          *r2 += binary[i] * j;
          j *= 2;
     }
     j = 1;

     // Bits 11-15
     for(i = 11; i < 16; i++) {
          *r3 += binary[i] * j;
          j *= 2;
     }
     j = 1;

     // Bits 0-5
     for(i = 0; i < 6; i++) {
          *funct += binary[i] * j;
          j *= 2;
     }
     j = 1;

     // Bits 0-15
     for(i = 0; i < 16; i++) {
          *offset += binary[i] * j;
          j *= 2;
     }
     j = 1;

     // Bits 0-25
     for(i = 0; i < 26; i++) {
          *jsec += binary[i] * j;
          j *= 2;
     }
     free(binary);
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
     switch(op) {
     // R - Type
     case 0:
          *controls = (struct_controls) {1,0,0,0,0,7,0,0,1};
          return 0;
     // Jump
     case 2:
          *controls = (struct_controls) {2,1,0,0,0,0,0,0,0};
          return 0;
     // Branch on equal
     case 4:
          *controls = (struct_controls) {0,0,1,0,0,1,0,0,0};
          return 0;
     // Add immediate
     case 8:
          *controls = (struct_controls) {0,0,0,0,0,0,0,1,1};
          return 0;
     // Set less than immediate
     case 10:
          *controls = (struct_controls) {0,0,0,0,0,2,0,1,1};
          return 0;
     // Set less than immediate unsigned
     case 11:
          *controls = (struct_controls) {0,0,0,0,0,3,0,1,1};
          return 0;
     // Load upper immediate
     case 15:
          *controls = (struct_controls) {0,0,0,0,0,6,0,1,1};
          return 0;
     // Load word
     case 35:
          *controls = (struct_controls) {0,0,0,1,1,0,0,1,1};
          return 0;
     // Store word
     case 43:
          *controls = (struct_controls) {0,0,0,0,0,0,1,1,0};
          return 0;
     default :
          return 1;
     }
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
     *data1 = Reg[r1];
     *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
     int sign = offset >> 15;
     if(sign == 1)
          *extended_value = offset | 0xffff0000;
     else
          *extended_value = offset & 0x0000ffff;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
     switch(ALUOp) {
     case 0:
     case 1:
     case 2:
     case 3:
     case 4:
     case 5:
     case 6:
          break;
     case 7:
          switch(funct) {
          // Add
          case 32:
               ALUOp = 0;
               break;
          // And
          case 36:
               ALUOp = 4;
               break;
          // Or
          case 37:
               ALUOp = 5;
               break;
          // Not
          case 39:
               ALUOp = 7;
               break;
          // Set less than
          case 42:
               ALUOp = 2;
               break;
          // Set less than unsigned
          case 43:
               ALUOp = 3;
               break;
          default :
               return 0;
          }
          break;
     default :
          return 1;
     }

     ALU(data1, (ALUSrc == 0) ? data2 : extended_value, ALUOp, ALUresult, Zero);
     return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
     if(MemWrite == 1) {
          if(ALUresult % 4 == 0 && ALUresult < 65536)
               Mem[ALUresult / 4] = data2;
          else return 1;
     }
     if(MemRead == 1) {
          if(ALUresult % 4 == 0 && ALUresult < 65536)
               *memdata = Mem[ALUresult / 4];
          else return 1;
     }
     return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
     if(RegWrite == 1) {
          if(RegDst == 1)
               Reg[r3] = ALUresult;
          else if(MemtoReg == 1)
               Reg[r2] = memdata;
          else
               Reg[r2] = ALUresult;
     }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
     *PC += 4;
     if(Branch == 1 && Zero == 1)
          *PC += extended_value * 4;
     else if(Jump == 1)
          *PC = (*PC & 0xf0000000) | jsec * 4;
}

