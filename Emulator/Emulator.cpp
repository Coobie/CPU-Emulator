/*
* Author: Coobie (Jacob)
* Created: 06/10/2016
* Revised: 23/11/2016
* Description: Emulates the Chimera-2016-A CPU
* User advice: None
*/


/*
*		Week 1 - ADDRESSING MODES
			   - Practical: LDAA, STORA, MVI, LODS
		       - Own  Time: LDAB, STORB, MOVE, LDX, STOX, LDY, STOY,CAY,
					        MYA, STOS, CSA, LX, ABA, SBA, AAB, SAB, MVI

*		Week 2 - FLAGS
               - Practical: ADC, CMP, CSA
			   - Own  Time: ADD, MYA, CLC, STC, CLI, STI, SBC, SUB,
					  		AND, BIT, OR, XOR

*		Week 3 - STACK
			   - Practical: PUSH, POP, JMP, JSR, RET
		       - Own  Time: JCC, JCS, JNE, JEQ, JMI, JPL, JHI, JLE, CCC,
					  		CCS, CNE, CEQ, CMI, CPL, CHI, CLE, SBIA, SBIB,
							CPIA, CPIB, ORIA, ORIB, HLT

*		Week 4 - INC & LOGIC
			   - Practical: INCA, INCX
		       - Own  Time: INC, INCB, DECX, INCX, DEY, INCY, DEC, DECA, DECB

*		Week 5 - ARITHMETIC
	 	       - Practical: COMA
     		   - Own  Time: SAL, SALB, LSR, ADCP, SBCP, XCHG, SWI,
				            RTI, SALA, RLC, RLCA, RLCB, RR,RRA, RRB, RRC, RRCA, RRCB, RTI,
						    LSRA, LSRB, COMA, COMB, ROL, ROLA, ROLB, NOP

*	    Week 6 - ROTATE
	 	 	   - Practical: SARA
			   - Own  Time: SAR, SARB
*/


#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "" //PROTECTED

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x10
#define FLAG_Z  0x04
#define FLAG_N  0x02
#define FLAG_C  0x01
#define REGISTER_M	4
#define REGISTER_A	3
#define REGISTER_B	2
#define REGISTER_H	1
#define REGISTER_L	0
#define REGISTER_X 0
#define REGISTER_Y 1
BYTE Index_Registers[2];

BYTE Registers[5];
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"ILLEGAL     ",
"DECX impl    ",
"INCX impl    ",
"DEY impl     ",
"INCY impl    ",
"CLC impl     ",
"STC impl     ",
"CLI impl     ",
"STI impl     ",
"ILLEGAL     ",
"LDAA  #      ",
"LDAB  #      ",
"LX  #,L      ",
"LX  #,L      ",
"LDX  #       ",
"LDY  #       ",

"JMP abs      ",
"JCC abs      ",
"JCS abs      ",
"JNE abs      ",
"JEQ abs      ",
"JMI abs      ",
"JPL abs      ",
"JHI abs      ",
"JLE abs      ",
"ILLEGAL     ",
"LDAA abs     ",
"LDAB abs     ",
"MVI  #,L     ",
"MVI  #,H     ",
"LDX abs      ",
"LDY abs      ",

"LODS  #      ",
"JSR abs      ",
"CCC abs      ",
"CCS abs      ",
"CNE abs      ",
"CEQ abs      ",
"CMI abs      ",
"CPL abs      ",
"CHI abs      ",
"CLE abs      ",
"LDAA abs,X   ",
"LDAB abs,X   ",
"NOP impl     ",
"HLT impl     ",
"LDX abs,X    ",
"LDY abs,X    ",

"LODS abs     ",
"ADC A,L      ",
"SBC A,L      ",
"ADD A,L      ",
"SUB A,L      ",
"CMP A,L      ",
"OR A,L       ",
"AND A,L      ",
"XOR A,L      ",
"BIT A,L      ",
"LDAA abs,Y   ",
"LDAB abs,Y   ",
"ILLEGAL     ",
"ILLEGAL     ",
"LDX abs,Y    ",
"LDY abs,Y    ",

"LODS abs,X   ",
"ADC A,H      ",
"SBC A,H      ",
"ADD A,H      ",
"SUB A,H      ",
"CMP A,H      ",
"OR A,H       ",
"AND A,H      ",
"XOR A,H      ",
"BIT A,H      ",
"LDAA (ind)   ",
"LDAB (ind)   ",
"RET impl     ",
"ILLEGAL     ",
"LDX (ind)    ",
"LDY (ind)    ",

"LODS abs,Y   ",
"ADC A,M      ",
"SBC A,M      ",
"ADD A,M      ",
"SUB A,M      ",
"CMP A,M      ",
"OR A,M       ",
"AND A,M      ",
"XOR A,M      ",
"BIT A,M      ",
"LDAA (ind,X) ",
"LDAB (ind,X) ",
"SWI impl     ",
"RTI impl     ",
"LDX (ind,X)  ",
"LDY (ind,X)  ",

"LODS (ind)   ",
"ADC B,L      ",
"SBC B,L      ",
"ADD B,L      ",
"SUB B,L      ",
"CMP B,L      ",
"OR B,L       ",
"AND B,L      ",
"XOR B,L      ",
"BIT B,L      ",
"STOS abs     ",
"MOVE A,A     ",
"MOVE B,A     ",
"MOVE L,A     ",
"MOVE H,A     ",
"MOVE M,A     ",

"LODS (ind,X) ",
"ADC B,H      ",
"SBC B,H      ",
"ADD B,H      ",
"SUB B,H      ",
"CMP B,H      ",
"OR B,H       ",
"AND B,H      ",
"XOR B,H      ",
"BIT B,H      ",
"STOS abs,X   ",
"MOVE A,B     ",
"MOVE B,B     ",
"MOVE L,B     ",
"MOVE H,B     ",
"MOVE M,B     ",

"ILLEGAL     ",
"ADC B,M      ",
"SBC B,M      ",
"ADD B,M      ",
"SUB B,M      ",
"CMP B,M      ",
"OR B,M       ",
"AND B,M      ",
"XOR B,M      ",
"BIT B,M      ",
"STOS abs,Y   ",
"MOVE A,L     ",
"MOVE B,L     ",
"MOVE L,L     ",
"MOVE H,L     ",
"MOVE M,L     ",

"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"SBIA  #      ",
"SBIB  #      ",
"CPIA  #      ",
"CPIB  #      ",
"ORIA  #      ",
"ORIB  #      ",
"ILLEGAL     ",
"STOS (ind)   ",
"MOVE A,H     ",
"MOVE B,H     ",
"MOVE L,H     ",
"MOVE H,H     ",
"MOVE M,H     ",

"INC abs      ",
"DEC abs      ",
"RRC abs      ",
"RLC abs      ",
"SAL abs      ",
"SAR abs      ",
"LSR abs      ",
"COM abs      ",
"ROL abs      ",
"RR abs       ",
"STOS (ind,X) ",
"MOVE A,M     ",
"MOVE B,M     ",
"MOVE L,M     ",
"MOVE H,M     ",
"MOVE -,-     ",

"INC abs,X    ",
"DEC abs,X    ",
"RRC abs,X    ",
"RLC abs,X    ",
"SAL abs,X    ",
"SAR abs,X    ",
"LSR abs,X    ",
"COM abs,X    ",
"ROL abs,X    ",
"RR abs,X     ",
"STORA abs    ",
"STORB abs    ",
"STOX abs     ",
"STOY abs     ",
"PUSH  ,A     ",
"POP A,       ",

"INC abs,Y    ",
"DEC abs,Y    ",
"RRC abs,Y    ",
"RLC abs,Y    ",
"SAL abs,Y    ",
"SAR abs,Y    ",
"LSR abs,Y    ",
"COM abs,Y    ",
"ROL abs,Y    ",
"RR abs,Y     ",
"STORA abs,X  ",
"STORB abs,X  ",
"STOX abs,X   ",
"STOY abs,X   ",
"PUSH  ,B     ",
"POP B,       ",

"INCA A,A     ",
"DECA A,A     ",
"RRCA A,A     ",
"RLCA A,A     ",
"SALA A,A     ",
"SARA A,A     ",
"LSRA A,A     ",
"COMA A,A     ",
"ROLA A,A     ",
"RRA A,A      ",
"STORA abs,Y  ",
"STORB abs,Y  ",
"STOX abs,Y   ",
"STOY abs,Y   ",
"PUSH  ,s     ",
"POP s,       ",

"INCB B,B     ",
"DECB B,B     ",
"RRCB B,B     ",
"RLCB B,B     ",
"SALB B,B     ",
"SARB B,B     ",
"LSRB B,B     ",
"COMB B,B     ",
"ROLB B,B     ",
"RRB B,B      ",
"STORA (ind)  ",
"STORB (ind)  ",
"STOX (ind)   ",
"STOY (ind)   ",
"PUSH  ,L     ",
"POP L,       ",

"CAY impl     ",
"MYA impl     ",
"CSA impl     ",
"ABA impl     ",
"SBA impl     ",
"AAB impl     ",
"SAB impl     ",
"ADCP A,L     ",
"SBCP A,L     ",
"XCHG A,L     ",
"STORA (ind,X)",
"STORB (ind,X)",
"STOX (ind,X) ",
"STOY (ind,X) ",
"PUSH  ,H     ",
"POP H,       ",

};





////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

///////////////////////
/// Flag Functions ////
///////////////////////

/*
* Function: set_flag_n
* Description: Sets the negative/sign bit
* Parameters: inReg (BYTE) - the register that will be used to set the flag
* Returns: none (void)
* Warnings: none
*/
void set_flag_n(BYTE inReg)
 {
	BYTE reg;
	reg = inReg;

	if ((reg & 0x80) != 0) // msbit set
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

/*
* Function: set_flag_z
* Description: Sets the zero flag if value is zero
* Parameters: inReg (BYTE) - the register that will be used to set the flag
* Returns: none (void)
* Warnings: none
*/
void set_flag_z(BYTE inReg)
 {
	BYTE reg;
	reg = inReg;

	if (reg == 0) // msbit set
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

/*
* Function: set_flag_c
* Description: Sets the carry flag if the answer is larger than 8 bits
* Parameters: inReg (WORD) - the register that will be used to set the flag
* Returns: none (void)
* Warnings: none
*/
void set_flag_c(WORD inReg)
 {
	WORD reg;
	reg = inReg;

	if (reg >= 0x100)
	{
		//set carry flag
		Flags = Flags | FLAG_C;
	}
	else
	{
		//clear carry flag
		Flags = Flags & (0xFF - FLAG_C);
	}
}

/*
* Function: set_flag_cnz
* Description: Sets c, n and z flags by calling the other functions
* Parameters: inReg (WORD) - the register that will be used to set the flags
* Returns: none (void)
* Warnings: none
*/
void set_flag_cnz(WORD inReg)
{
	set_flag_c(inReg);
	set_flag_z((BYTE)inReg);
	set_flag_n((BYTE)inReg);
}

/*
* Function: carry_test_ADC
* Description: Checks to see if the carry flag is set for ADC
* Parameters: inReg (WORD) - the register that will be used to set the flag
* Returns: temp (WORD)
* Warnings: none
*/
WORD carry_test_ADC(WORD temp)
{
	if ((Flags & FLAG_C) != 0)
			{
				//If the carry flag is set then the value of the sum will be incremented by one
				//If the carry flag is not set then this will be skipped and will just return the previous value
				temp++;
			}
	//returns the value back to the program
	return temp;
}

/*
* Function: carry_test_SBC
* Description: Checks to see if the carry flag is set for SBC
* Parameters: inReg (WORD) - the register that will be used to set the flag
* Returns: temp (WORD)
* Warnings: none
*/
WORD carry_test_SBC(WORD temp)
{
	if ((Flags & FLAG_C) != 0)
			{
				//If the carry flag is set then the value of the sum will be incremented by one
				//If the carry flag is not set then this will be skipped and will just return the previous value
				temp--;
			}
	//returns the value back to the program
	return temp;
}

///////////////////////////
// Addressing Functions //
/////////////////////////

/*
* Variables for addressing function
* Description: Sets values for the addressing types to be used the next function
*/
// Absolute addressing
int abs_a = 0;

// Absolute with register X addressing
int absX_a = 1;

// Absolute with register Y addressing
int absY_a = 2;

// Indirect addressing
int ind_a = 3;

// Indexed Indirect addressing
int indX_a = 4;

/*
* Function: get_address
* Description: Sets value for type of address to be used the next function
* Parameters: address_type integer (provided by previous addressing functions)
* Returns: address (WORD)
* Warnings: none
*/
WORD get_address(int address_type)
 {
	WORD address = 0;
	BYTE HB;
	BYTE LB;

	switch (address_type)
	{
	case 0: //abs (Absolute addressing)
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		break;

	case 1: //absX (Indexed Absolute addresing with X)
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		break;

	case 2: //absY (Indexed Absolute addresing with Y)
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		break;

	case 3: //ind (Indirect addressing)
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		break;

	case 4: //indX (Indexed indirect addresssing)
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		break;
	}
	return address;
}

/////////////////////////
// REST OF FUNCTIONS  //
////////////////////////

/*
* Function: F_regM
* Description: Sets the contents of Register M
* Parameters: None
* Returns: none (void)
* Warnings: Run before calling regsiter M
*/
void F_regM()
{
	WORD address = 0;
	address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
	Registers[REGISTER_M] = Memory[address];
}

/*
* Function: F_LOAD
* Description: Loads memory into register
* Parameters: reg (BYTE), address (WORD)
* Parameters info: reg is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_LOAD(BYTE reg, WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Puts memory using address into register
		Registers[reg] = Memory[address];
	}
}

/*
* Function: F_LOAD_I
* Description: Loads memory into index register
* Parameters: reg (BYTE), address (WORD)
* Parameters info: reg is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_LOAD_I(BYTE reg, WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Puts memory using address into index register
		Index_Registers[reg] = Memory[address];
	}
}

/*
* Function: F_STORE
* Description: Stores registers into memory
* Parameters: reg (BYTE), address (WORD)
* Parameters info: reg is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_STORE(BYTE reg, WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Puts register into memory address
		Memory[address] = Registers[reg];
	}
}

/*
* Function: F_STORE_I
* Description: Stores index registers into memory
* Parameters: reg (BYTE), address (WORD)
* Parameters info: reg is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_STORE_I(BYTE reg,WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Puts index register into memory address
		Memory[address] = Index_Registers[reg];
	}
}

/*
* Function: F_MVI
* Description: Loads memory into register
* Parameters: reg (BYTE) - the register you want memory to be stored in.
* Parameters info: reg is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_MVI(BYTE reg)
{
	WORD data = 0;
	data = fetch();
	//Puts fetched memory into register
	Registers[reg] = data;
}

/*
* Function: F_LOAD_S
* Description: Loads memory into StackPointer
* Parameters: address (WORD) - the address to load into StackPointer
* Returns: none (void)
* Warnings: none
*/
void F_LOAD_S(WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE - 1)
	{
		//Puts memory using address into StackPointer
		StackPointer = (WORD)Memory[address] << 8;
		StackPointer += Memory[address + 1];
	}
}

/*
* Function: F_STORE_S
* Description: Stores StackPointer into memory
* Parameters: address (WORD)
* Returns: none (void)
* Warnings: none
*/
void F_STORE_S(WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Puts StackPointer into memory using address
		Memory[address] = (BYTE)(StackPointer >> 8);
		Memory[address + 1] += (BYTE)StackPointer;
	}
}

/*
* Function: F_ADD
* Description: Adds registers
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_ADD(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] + (WORD)Registers[reg2];
	//Sets carry, negative and zero flags
	set_flag_cnz(temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_SUB
* Description: Subtracts registers
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_SUB(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] - (WORD)Registers[reg2];
	//Sets carry, negative and zero flags
	set_flag_cnz(temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_ADC
* Description: Adds registers with carry
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_ADC(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] + (WORD)Registers[reg2];
	//Tests whether carry flag is set (adding if true)
	temp_word = carry_test_ADC(temp_word);
	//Sets carry, negative and zero flags
	set_flag_cnz(temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_CMP
* Description: Compares registers
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_CMP(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Compares the two registers by subtraction
	temp_word = ((WORD)Registers[reg1] - (WORD)Registers[reg2]);
	//Sets carry, negative and zero flags
	set_flag_cnz(temp_word);
}

/*
* Function: F_SBC
* Description: Subtracts registers with carry
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_SBC(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] - (WORD)Registers[reg2];
	//Tests to see whether carry flag is set
	temp_word = carry_test_SBC(temp_word);
	set_flag_cnz(temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_AND
* Description: Bitwise ands registers
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_AND(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] & (WORD)Registers[reg2];
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_OR
* Description: Bitwise or registers
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_OR(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] | (WORD)Registers[reg2];
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_XOR
* Description: Bitwise xor registers
* Parameters: reg1 (BYTE), reg2 (BYTE) - reg1 is the register you want to store the result in (accumulator)
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_XOR(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] ^ (WORD)Registers[reg2];
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_BIT
* Description: Register bit tested with accumulator
* Parameters: reg1 (BYTE), reg2 (BYTE) - the two registers to compare
* Parameters info: regs are given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_BIT(BYTE reg1, BYTE reg2)
{
	WORD temp_word;
	//registers are compared by bitwise and
	temp_word = (WORD)Registers[reg1] & (WORD)Registers[reg2];
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
}

/*
* Function: F_PUSH
* Description: Pushes registers onto the stack
* Parameters: reg1 (BYTE) - the register to push onto the stack
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_PUSH(BYTE reg1)
{
	//Check address held in StackPointer is valid
	if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
	{
		//Put register values onto the stack
		Memory[StackPointer] = Registers[reg1];
		StackPointer--;
	}
}

/*
* Function: F_PUSH_FL
* Description: Pushes flags onto the stack
* Parameters: none
* Returns: none (void)
* Warnings: none
*/
void F_PUSH_FL()
{
	//Check the location of the StackPointer is valid for push
	if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
	{
		//Puts status register onto stack
		Memory[StackPointer] = Flags;
		StackPointer--;
	}
}

/*
* Function: F_POP
* Description: Pops the stack into registers
* Parameters: reg1 (BYTE) - the location to pop the stack to
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_POP(BYTE reg1)
{
	//Check the location of the StackPointer is valid for pop
	if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
	{
		StackPointer++;
		//Puts the memory location in the stack into the register
		Registers[reg1] = Memory[StackPointer];
	}
}

/*
* Function: F_POP_FL
* Description: Pops the stack into flags
* Parameters: none
* Returns: none (void)
* Warnings: none
*/
void F_POP_FL()
{
	//Check the location of the StackPointer is valid for pop
	if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
	{
		StackPointer++;
		//Puts the memory location in the stack into the status register
		Flags = Memory[StackPointer];
	}
}

/*
* Function: F_INC_R
* Description: Increments the register
* Parameters: reg1 (BYTE) - the register to be incremented
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_INC_R(BYTE reg1)
{
	//Increments the register
	Registers[reg1]++;
	//Sets zero and negative flags
	set_flag_z(Registers[reg1]);
	set_flag_n(Registers[reg1]);
}

/*
* Function: F_INC_I
* Description: Increments the index register
* Parameters: reg1 (BYTE) - the indec register to be incremented
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_INC_I(BYTE reg1)
{
	//Increments the index register
	Index_Registers[reg1]++;
	//Sets the zero flags
	set_flag_z(Index_Registers[reg1]);
}

/*
* Function: F_DEC_R
* Description: Decrements regisers
* Parameters: reg1 (BYTE) - the register to be decremented
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_DEC_R(BYTE reg1)
{
	//Decrements the register
	Registers[reg1]--;
	//Sets zero and negative flags
	set_flag_z(Registers[reg1]);
	set_flag_n(Registers[reg1]);
}

/*
* Function: F_DEC_I
* Description: Decrements the index register
* Parameters: reg1 (BYTE) - the index register to be decremented
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_DEC_I(BYTE reg1)
{
	//Decrements the index register
	Index_Registers[reg1]--;
	//Sets the zero flag
	set_flag_z(Index_Registers[reg1]);
}

/*
* Function: F_INC_M
* Description: Increments memory
* Parameters: address (WORD) - the memory address to be incremented
* Returns: none (void)
* Warnings: none
*/
void F_INC_M(WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Increments memory
		Memory[address]++;
	}
	//Sets negative and zero flags
	set_flag_n(Memory[address]);
	set_flag_z(Memory[address]);
}

/*
* Function: F_DEC_M
* Description: Decrements memory
* Parameters: address (WORD) - the memory address to be decremented
* Returns: none (void)
* Warnings: none
*/
void F_DEC_M(WORD address)
{
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Decrements memory
		Memory[address]--;
	}
	//Sets negative and zero flags
	set_flag_n(Memory[address]);
	set_flag_z(Memory[address]);
}

/*
* Function: F_SBI
* Description: Data subtracted to accumulator with carry
* Parameters: reg1 (BYTE) - the target accumulator
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_SBI(BYTE reg1)
{
	//Fetches the data from memory
	WORD data = 0;
	data = fetch();
	//The data is then subtracted to accumulator with carry
	WORD temp_word;
	temp_word = (WORD)data - (WORD)Registers[reg1];
	//Tests to see whether carry flag is set
	temp_word = carry_test_SBC(temp_word);
	//Sets carry, negative and zero flags
	set_flag_cnz(temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_CPI
* Description: Data compared to accumulator
* Parameters: reg1 (BYTE) - the accumulator to be compared with the data
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_CPI(BYTE reg1)
{
	//Fetches the data from memory
	WORD data;
	data = fetch();
	//Data is then compared to accumulator by subtraction
	WORD temp_word;
	temp_word = (WORD)data - (WORD)Registers[reg1];
	//Sets carry, negative and zero flags
	set_flag_cnz(temp_word);
}

/*
* Function: F_ORI
* Description: Data bitwise inclusive or with accumulator
* Parameters: reg1 (BYTE) - target accumulator
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_ORI(BYTE reg1)
{
	//Fetches the data from memory
	WORD data = 0;
	data = fetch();
	WORD temp_word;
	//Stores the result of the sum in temp_word
	temp_word = (WORD)Registers[reg1] | data;
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_CALL
* Description: For use with call opcodes
* Parameters: address (WORD) - the address to set the ProgramCounter to
* Returns: none (void)
* Warnings: none
*/
void F_CALL(WORD address)
{
	//Check address held in StackPointer is valid
	if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
	{
		//Puts program counter into stack pointer
		Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
		StackPointer--;
		Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		StackPointer--;
		//Sets program counter to the next address
		ProgramCounter = address;
	}
}

/*
* Function: F_COM_M
* Description: Negate memory
* Parameters: address (WORD) - the address to Negate
* Returns: none (void)
* Warnings: none
*/
void F_COM_M(WORD address)
{
	WORD temp_word;
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Negates the address
		temp_word = ~(Memory[address]);
		//Sets carry, negative and zero flags
		set_flag_cnz(temp_word);
		//Puts the value from the calculation back into address
		Memory[address] = temp_word;
	}
}

/*
* Function: F_COM_R
* Description: Negate register
* Parameters: reg1 (BYTE) - the register to Negate
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_COM_R(BYTE reg1)
{
	WORD temp_word;
	//Negates the value of the regsiter
	temp_word = ~(Registers[reg1]);
	//Sets the carry, negative and zero flags
	set_flag_cnz(temp_word);
	//Puts the value from the calculation back into reg1
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_SL_M
* Description: Arithmetic shift left memory
* Parameters: address (WORD) - the address to shift
* Returns: none (void)
* Warnings: none
*/
void F_SL_M(WORD address)
{
	WORD temp_word;
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Sets carry flag
		if ((Memory[address] & 0x80) == 1)
		{
			Flags |= FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		//Stores the address as temp_word left shifted one
		temp_word = Memory[address] << 1;
		//Sets carry, negative and zero flags
		set_flag_cnz(temp_word);
		//Puts the shifted address back into memory
		Memory[address] = temp_word;
	}
}

/*
* Function: F_SL_R
* Description: Arithmetic shift left accumulator
* Parameters: reg1 (BYTE) - the accumulator to shift
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_SL_R(BYTE reg1)
{
	WORD temp_word;
	//Sets carry flag
	if ((Registers[reg1] & 0x80) == 1)
	{
		Flags |= FLAG_C;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_C);
	}
	//Stores the register as temp_word left shifted one
	temp_word = Registers[reg1] << 1;
	//Sets the carry, negative and zero flags
	set_flag_cnz(temp_word);
	//Puts the shifted value back into the register
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_SR_M
* Description: Arithmetic shift right memory
* Parameters: address (WORD) - the address to shift
* Returns: none (void)
* Warnings: none
*/
void F_SR_M(WORD address)
{
	WORD temp_word;
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		temp_word = Memory[address];
		//Sets carry, negative and zero flags before shift
		set_flag_cnz(temp_word);

		//Sets the carry flag
		if ((temp_word & 0x01) != 0)
		{
			Flags |= FLAG_C;
		}
		//Shifts temp_word one place right
		temp_word >>= 1;
		//Sets the negative flag
		if ((Flags & FLAG_N) != 0)
		{
			temp_word |= 0x80;
		}
		//Puts the shifted address back into memory
		Memory[address] = (BYTE)temp_word;
	}
}

/*
* Function: F_SR_R
* Description: Arithmetic shift right accumulator
* Parameters: reg1 (BYTE) - the address to shift
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_SR_R(BYTE reg1)
{
	WORD temp_word;
	temp_word = Registers[reg1];
	//Sets carry, negative and zero flags before shift
	set_flag_cnz(temp_word);
	//Sets carry flag
	if ((temp_word & 0x01) != 0)
	{
		Flags |= FLAG_C;
	}
	temp_word >>= 1;
	//Sets negative flag
	if ((Flags & FLAG_N) != 0)
	{
		temp_word |= 0x80;
	}
	//Puts the shifted address back into memory
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_ROL_M
* Description: Rotate memory left without carry
* Parameters: address (WORD) - the address to rotate
* Returns: none (void)
* Warnings: none
*/
void F_ROL_M(WORD address)
{
	WORD temp_word;
	temp_word = Memory[address];
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Rotates value held
		if ((temp_word & 0x80) == 0x80)
		{
			temp_word = (temp_word << 1) + 0x01;
		}
		else
		{
			temp_word = (temp_word << 1);
		}
		//Sets the zero and negative flags
		set_flag_z((BYTE)temp_word);
		set_flag_n((BYTE)temp_word);
		//Puts the rotated address back into memory
		Memory[address] = temp_word;
	}
}

/*
* Function: F_ROL_R
* Description: Rotate accumulator left without carry
* Parameters: reg1 (BYTE) - the accumulator to rotate
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_ROL_R(BYTE reg1)
{
	WORD temp_word;
	temp_word = Registers[reg1];
	//Rotates value held
	if ((temp_word & 0x80) == 0x80)
	{
		temp_word = (temp_word << 1) + 0x01;
	}
	else
	{
		temp_word = (temp_word << 1);
	}
	//Sets the negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the rotated value back into the register
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_RR_M
* Description: Rotate memory right without carry
* Parameters: addresss (WORD) - the address to rotate
* Returns: none (void)
* Warnings: none
*/
void F_RR_M(WORD address)
{
	WORD temp_word;
	temp_word = Memory[address];
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Rotates value held
		if ((temp_word & 0x01) == 0x01)
		{
			temp_word = (temp_word >> 1) + 0x80;
		}
		else
		{
			temp_word = (temp_word >> 1);
		}
		//Sets zero and negative flags
		set_flag_z((BYTE)temp_word);
		set_flag_n((BYTE)temp_word);
		//Puts the rotated address back into memory
		Memory[address] = temp_word;
	}
}

/*
* Function: F_RR_R
* Description: Rotate accumulator right without carry
* Parameters: reg1 (BYTE) - the accumulator to rotate
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_RR_R(BYTE reg1)
{
	WORD temp_word;
	temp_word = Registers[reg1];
	//Rotates value held
	if ((temp_word & 0x01) == 0x01)
	{
		temp_word = (temp_word >> 1) + 0x80;
	}
	else
	{
		temp_word = (temp_word >> 1);
	}
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the rotated value back into the register
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: F_RLC_R
* Description: Rotate register left with carry
* Parameters: reg1 (BYTE) - the register to rotate
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_RLC_R(BYTE reg1)
{
	//Saves current flags
	BYTE saved_flags = Flags;
	//Carry flag set
	if((Registers[reg1] & 0x80) == 0x80)
	{
		Flags = Flags | FLAG_C;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_C);
	}
	//Rotates register through carry
	Registers[reg1] = (Registers[reg1] << 1) & 0xFE;
	if ((saved_flags & FLAG_C) == FLAG_C)
	{
		Registers[reg1] = Registers[reg1] | 0x01;
	}
	//Sets zero and negative flags
	set_flag_z(Registers[reg1]);
	set_flag_n(Registers[reg1]);
}

/*
* Function: F_RLC_M
* Description: Rotate memory left with carry
* Parameters: addresss (WORD) - the address to rotate
* Returns: none (void)
* Warnings: none
*/
void F_RLC_M(WORD address)
{
	//Saves current flags
	BYTE saved_flags = Flags;
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Sets carry flag
		if((Memory[address] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		//Rotates address through carry
		Memory[address] = (Memory[address] << 1) & 0xFE;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		//Sets zero and negative flags
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
	}
}

/*
* Function: F_RRC_R
* Description: Rotate register right with carry
* Parameters: reg1 (BYTE) - the register to rotate
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_RRC_R(BYTE reg1)
{
	//Saves current flags
	BYTE saved_flags = Flags;
	//Sets carry flag
	if((Registers[reg1] & 0x01) == 0x01)
	{
		Flags = Flags | FLAG_C;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_C);
	}
	//Rotates register through carry
	Registers[reg1] = (Registers[reg1] >> 1) & 0x7F;
	if ((saved_flags & FLAG_C) == FLAG_C)
	{
		Registers[reg1] = Registers[reg1] | 0x80;
	}
	//Sets zero and negative flags
	set_flag_z(Registers[reg1]);
	set_flag_n(Registers[reg1]);
}

/*
* Function: F_RRC_M
* Description: Rotate memory right with carry
* Parameters: addresss (WORD) - the address to rotate
* Returns: none (void)
* Warnings: none
*/
void F_RRC_M(WORD address)
{
	//Saves current flags
	BYTE saved_flags = Flags;
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Sets carry flag
		if((Memory[address] & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		//Rotates address through carry
		Memory[address] = (Memory[address] >> 1) & 0x7F;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x80;
		}
		//Sets zero and negative flags
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
	}
}

/*
* Function: F_LSR_M
* Description: Shift right memory
* Parameters: addresss (WORD) - the address to shift
* Returns: none (void)
* Warnings: none
*/
void F_LSR_M(WORD address)
{
	WORD temp_word;
	temp_word = Memory[address];
	//Check if address held is valid
	if (address >= 0 && address < MEMORY_SIZE)
	{
		//Sets carry flag
		if ((temp_word & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		//Shifts addresss
		temp_word = (temp_word >> 1) & 0x7F;
		//Sets negative and zero flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		//Puts the shifted address back into memory
		Memory[address] = temp_word;
	}
}

/*
* Function: F_LSR_R
* Description: Shift right register
* Parameters: reg1 (BYTE) - the register to shift
* Parameters info: reg1 is given by REGISTER_#
* Returns: none (void)
* Warnings: none
*/
void F_LSR_R(BYTE reg1)
{
	WORD temp_word;
	temp_word = Registers[reg1];
	//Sets carry flag
	if ((temp_word & 0x01) == 0x01)
	{
		Flags = Flags | FLAG_C;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_C);
	}
	//Shifts register value
	temp_word = (temp_word >> 1) & 0x7F;
	//Sets negative and zero flags
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);
	//Puts the shifted address back into memory
	Registers[reg1] = (BYTE)temp_word;
}

/*
* Function: Group_1
* Description: Function to decode group 1 opcodes
* Parameters: opcode (BYTE)
* Returns: None (VOID)
* Warnings: none
*/
void Group_1(BYTE opcode)
{
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	WORD temp_word;
	WORD temp_word2;

	switch (opcode) 
	{

		/*
		* Opcode name: LDAA
		* Description: Loads Memory into Accumulator A
		*/
	case 0x0A: //LDAA (#)
		data = fetch();
		Registers[REGISTER_A] = data;
		break;

	case 0x1A: //LDAA (abs)
		address = get_address(abs_a);
		F_LOAD(REGISTER_A, address);
		break;

	case 0x2A: //LDAA (abs, X)
		address = get_address(absX_a);
		F_LOAD(REGISTER_A, address);
		break;

	case 0x3A: //LDAA (abs, Y)
		address = get_address(absY_a);
		F_LOAD(REGISTER_A, address);
		break;

	case 0x4A: //LDAA (ind)
		address = get_address(ind_a);
		F_LOAD(REGISTER_A, address);
		break;

	case 0x5A: //LDAA (ind, X)
		address = get_address(indX_a);
		F_LOAD(REGISTER_A, address);
		break;

		/*
		* Opcode name: LDAB
		* Description: Loads Memory into Accumulator B
		*/
	case 0x0B: //LDAB (#)
		data = fetch();
		Registers[REGISTER_B] = data;
		break;

	case 0x1B: //LDAB (abs)
		address = get_address(abs_a);
		F_LOAD(REGISTER_B, address);
		break;

	case 0x2B: //LDAB (abs, X)
		address = get_address(absX_a);
		F_LOAD(REGISTER_B, address);
		break;

	case 0x3B: //LDDB (abs, Y)
		address = get_address(absY_a);
		F_LOAD(REGISTER_B, address);
		break;

	case 0x4B: //LDAB (ind)
		address = get_address(ind_a);
		F_LOAD(REGISTER_B, address);
		break;

	case 0x5B: //LDAB (ind, X)
		address = get_address(indX_a);
		F_LOAD(REGISTER_B, address);
		break;

		/*
		* Opcode name: STORA
		* Description: Stores Accumulator A into Memory
		*/
	case 0xBA: //STORA (abs)
		address = get_address(abs_a);
		F_STORE(REGISTER_A, address);
		break;

	case 0xCA: //STORA (abs, X)
		address = get_address(absX_a);
		F_STORE(REGISTER_A, address);
		break;

	case 0xDA: //STORA (abs, Y)
		address = get_address(absY_a);
		F_STORE(REGISTER_A, address);
		break;

	case 0xEA: //STORA (ind)
		address = get_address(ind_a);
		F_STORE(REGISTER_A, address);
		break;

	case  0xFA: //STORA (ind, X)
		address = get_address(indX_a);
		F_STORE(REGISTER_A, address);
		break;

		/*
		* Opcode name: STORB
		* Description: Stores Accumulator B into Memory
		*/
	case 0xBB: //STORB (abs)
		address = get_address(abs_a);
		F_STORE(REGISTER_B, address);
		break;

	case 0xCB: //STORB (abs, X)
		address = get_address(absX_a);
		F_STORE(REGISTER_B, address);
		break;

	case 0xDB: //STORB (abs, Y)
		address = get_address(absY_a);
		F_STORE(REGISTER_B, address);
		break;

	case 0xEB: //STORB (ind)
		address = get_address(ind_a);
		F_STORE(REGISTER_B, address);
		break;

	case  0xFB: //STORB (ind, X)
		address = get_address(indX_a);
		F_STORE(REGISTER_B, address);
		break;

		/*
		* Opcode name: LDX
		* Description: Loads Memory into register X
		*/
	case 0x0E: //LDX (#)
		data = fetch();
		Index_Registers[REGISTER_X] = data;
		break;

	case 0x1E: //LDX (abs)
		address = get_address(abs_a);
		F_LOAD_I(REGISTER_X, address);
		break;

	case 0x2E: //LDX (abs, X)
		address = get_address(absX_a);
		F_LOAD_I(REGISTER_X, address);
		break;

	case 0x3E: //LDX (abs, Y)
		address = get_address(absY_a);
		F_LOAD_I(REGISTER_X, address);
		break;

	case 0x4E: //LDX (ind)
		address = get_address(ind_a);
		F_LOAD_I(REGISTER_X, address);
		break;

	case 0x5E: //LDX (ind, X)
		address = get_address(indX_a);
		F_LOAD_I(REGISTER_X, address);
		break;

		/*
		* Opcode name: STOX
		* Description: Stores register X into Memory
		*/
	case 0xBC: //STOX (abs)
		address = get_address(abs_a);
		F_STORE_I(REGISTER_X, address);
		break;

	case 0xCC: //STOX  (abs, X)
		address = get_address(absX_a);
		F_STORE_I(REGISTER_X, address);
		break;

	case 0xDC: //STOX  (abs, Y)
		address = get_address(absY_a);
		F_STORE_I(REGISTER_X, address);
		break;

	case 0xEC: //STOX  (ind)
		address = get_address(ind_a);
		F_STORE_I(REGISTER_X, address);
		break;

	case  0xFC: //STOX  (ind, X)
		address = get_address(indX_a);
		F_STORE_I(REGISTER_X, address);
		break;

		/*
		* Opcode name: LDY
		* Description: Loads Memory into register Y
		*/
	case 0x0F: //LDY (#)
		data = fetch();
		Index_Registers[REGISTER_Y] = data;
		break;

	case 0x1F: //LDY (abs)
		address = get_address(abs_a);
		F_LOAD_I(REGISTER_Y, address);
		break;

	case 0x2F: //LDY (abs, X)
		address = get_address(absX_a);
		F_LOAD_I(REGISTER_Y, address);
		break;

	case 0x3F: //LDY (abs, Y)
		address = get_address(absY_a);
		F_LOAD_I(REGISTER_Y, address);
		break;

	case 0x4F: //LDY (ind)
		address = get_address(ind_a);
		F_LOAD_I(REGISTER_Y, address);
		break;

	case 0x5F: //LDY (ind, X)
		address = get_address(indX_a);
		F_LOAD_I(REGISTER_Y, address);
		break;

		/*
		* Opcode name: STOY
		* Description: Stores register Y into Memory
		*/
	case 0xBD: //STOY (abs)
		address = get_address(abs_a);
		F_STORE_I(REGISTER_Y, address);
		break;

	case 0xCD: //STOY (abs, X)
		address = get_address(absX_a);
		F_STORE_I(REGISTER_Y, address);
		break;

	case 0xDD: //STOY  (abs, Y)
		address = get_address(absY_a);
		F_STORE_I(REGISTER_Y, address);
		break;

	case 0xED: //STOY  (ind)
		address = get_address(ind_a);
		F_STORE_I(REGISTER_Y, address);
		break;

	case  0xFD: //STOY  (ind, X)
		address = get_address(indX_a);
		F_STORE_I(REGISTER_Y, address);
		break;

		/*
		* Opcode name: MVI
		* Description: Loads Memory into register
		*/
	case 0x1C: //MVI L (#)
		F_MVI(REGISTER_L);
		break;

	case 0x1D: //MVI H (#)
		F_MVI(REGISTER_H);
		break;

		/*
		* Opcode name: LODS
		* Description: Loads Memory into Stackpointer
		*/
	case 0x20: //LODS (#)
		data = fetch();
		StackPointer = data << 8;
		StackPointer += fetch();
		break;

	case 0x30: //LODS (abs)
		address = get_address(abs_a);
		F_LOAD_S(address);
		break;

	case 0x40: //LODS (abs, X)
		address = get_address(absX_a);
		F_LOAD_S(address);
		break;

	case 0x50: //LODS (abs, Y)
		address = get_address(absY_a);
		F_LOAD_S(address);
		break;

	case 0x60: //LODS (ind)
		address = get_address(ind_a);
		F_LOAD_S(address);
		break;

	case 0x70: //LODS (ind, X)
		address = get_address(indX_a);
		F_LOAD_S(address);
		break;

		/*
		* Opcode name: STOS
		* Description: Stores Stackpointer into Memory
		*/
	case 0x6A: //STOS (abs)
		address = get_address(abs_a);
		F_STORE_S(address);
		break;

	case 0x7A: //STOS (abs, X)
		address = get_address(absX_a);
		F_STORE_S(address);
		break;

	case 0x8A: //STOS  (abs, Y)
		address = get_address(absY_a);
		F_STORE_S(address);
		break;

	case 0x9A: //STOS  (ind)
		address = get_address(ind_a);
		F_STORE_S(address);
		break;

	case  0xAA: //STOS  (ind, X)
		address = get_address(indX_a);
		F_STORE_S(address);
		break;

		/*
		* Opcode name: LX
		* Description: Loads Memory into register pair
		*/
	case 0x0C: //LX	  (LH,#)
		Registers[REGISTER_H] = fetch();
		Registers[REGISTER_L] = fetch();
		break;

	case 0x0D: //LX	  (LH,#)
		Registers[REGISTER_H] = fetch();
		Registers[REGISTER_L] = fetch();
		break;

		/*
		* Opcode name: MYA
		* Description: Transfer index register Y to Accumulator
		*/
	case 0xF1: //MYA  (impl)
		Registers[REGISTER_A] = Index_Registers[REGISTER_Y];
		break;

		/*
		* Opcode name: CSA
		* Description: Transfer Status register to Accumulator
		*/
	case 0xF2: //CSA (impl)
		Registers[REGISTER_A] = Flags;
		break;

		/*
		* Opcode name: CAY
		* Description: Transfer Accumulator to index register Y
		* Flags: - - - I - Z N C
		* Flags: - - - - - - T -
		*/
	case 0xF0: //CAY (impl)
		temp_word = (WORD)Registers[REGISTER_A];
		Index_Registers[REGISTER_Y] = temp_word;
		set_flag_n((BYTE)temp_word);
		break;

		/*
		* Opcode name: ABA
		* Description: Adds Accumulator B into Accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xF3: //ABA (impl)
		F_ADD(REGISTER_A, REGISTER_B);
		break;

		/*
		* Opcode name: AAB
		* Description: Adds Accumulator A into Accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xF5: //AAB (impl)
		F_ADD(REGISTER_B, REGISTER_A);
		break;

		/*
		* Opcode name: SBA
		* Description: Subtracts Accumulator B from Accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xF4: //SBA (impl)
		F_SUB(REGISTER_A, REGISTER_B);
		break;

		/*
		* Opcode name: SAB
		* Description: Subtracts Accumulator A from Accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xF6: //SAB (impl)
		F_SUB(REGISTER_B, REGISTER_A);
		break;

		/*
		* Opcode name: ADC
		* Description: Register added to Accumulator with Carry (A + CF + R)
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x31: //ADC A-L
		F_ADC(REGISTER_A, REGISTER_L);
		break;

	case 0x41: //ADC A-H
		F_ADC(REGISTER_A, REGISTER_H);
		break;

	case 0x51: //ADC A-M
		F_ADC(REGISTER_A, REGISTER_M);
		break;

	case 0x61: //ADC B-L
		F_ADC(REGISTER_B, REGISTER_L);
		break;

	case 0x71: //ADC B-H
		F_ADC(REGISTER_B, REGISTER_H);
		break;

	case 0x81: //ADC B-M
		F_ADC(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: CMP
		* Description: Register compared to Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x35: //CMP A-L
		F_CMP(REGISTER_A, REGISTER_L);
		break;

	case 0x45: //CMP A-H
		F_CMP(REGISTER_A, REGISTER_H);
		break;

	case 0x55: //CMP A-M
		F_CMP(REGISTER_A, REGISTER_M);
		break;

	case 0x65: //CMP B-L
		F_CMP(REGISTER_B, REGISTER_L);
		break;

	case 0x75: //CMP B-H
		F_CMP(REGISTER_B, REGISTER_H);
		break;

	case 0x85: //CMP B-M
		F_CMP(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: SBC
		* Description: Register subtracted to Accumulator with Carry (A - CF - R)
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x32: //SBC A-L
		F_SBC(REGISTER_A, REGISTER_L);
		break;

	case 0x42: //SBC A-H
		F_SBC(REGISTER_A, REGISTER_H);
		break;

	case 0x52: //SBC A-M
		F_SBC(REGISTER_A, REGISTER_M);
		break;

	case 0x62: //SBC B-L
		F_SBC(REGISTER_B, REGISTER_L);
		break;

	case 0x72: //SBC B-H
		F_SBC(REGISTER_B, REGISTER_H);
		break;

	case 0x82: //SBC B-M
		F_SBC(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: SUB
		* Description: Register subtracted to Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x34: //SUB A-L
		F_SUB(REGISTER_A, REGISTER_L);
		break;

	case 0x44: //SUB A-H
		F_SUB(REGISTER_A, REGISTER_H);
		break;

	case 0x54: //SUB A-M
		F_SUB(REGISTER_A, REGISTER_M);
		break;

	case 0x64: //SUB B-L
		F_SUB(REGISTER_B, REGISTER_L);
		break;

	case 0x74: //SUB B-H
		F_SUB(REGISTER_B, REGISTER_H);
		break;

	case 0x84: //SUB B-M
		F_SUB(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: ADD
		* Description: Register added to Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x33: //ADD A-L
		F_ADD(REGISTER_A, REGISTER_L);
		break;

	case 0x43: //ADD A-H
		F_ADD(REGISTER_A, REGISTER_H);
		break;

	case 0x53: //ADD A-M
		F_ADD(REGISTER_A, REGISTER_M);
		break;

	case 0x63: //ADD B-L
		F_ADD(REGISTER_B, REGISTER_L);
		break;

	case 0x73: //ADD B-H
		F_ADD(REGISTER_B, REGISTER_H);
		break;

	case 0x83: //ADD B-M
		F_ADD(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: AND
		* Description: Register bitwise and with Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0x37: //AND A-L
		F_AND(REGISTER_A, REGISTER_L);
		break;

	case 0x47: //AND A-H
		F_AND(REGISTER_A, REGISTER_H);
		break;

	case 0x57: //AND A-M
		F_AND(REGISTER_A, REGISTER_M);
		break;

	case 0x67: //AND B-L
		F_AND(REGISTER_B, REGISTER_L);
		break;

	case 0x77: //AND B-H
		F_AND(REGISTER_B, REGISTER_H);
		break;

	case 0x87: //AND B-M
		F_AND(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: XOR
		* Description: Register bitwise exclusive or with Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0x38: //XOR A-L
		F_XOR(REGISTER_A, REGISTER_L);
		break;

	case 0x48: //XOR A-H
		F_XOR(REGISTER_A, REGISTER_H);
		break;

	case 0x58: //XOR A-M
		F_XOR(REGISTER_A, REGISTER_M);
		break;

	case 0x68: //XOR B-L
		F_XOR(REGISTER_B, REGISTER_L);
		break;

	case 0x78: //XOR B-H
		F_XOR(REGISTER_B, REGISTER_H);
		break;

	case 0x88: //XOR B-M
		F_XOR(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: OR
		* Description: Register bitwise inclusive or with Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0x36: //OR A-L
		F_OR(REGISTER_A, REGISTER_L);
		break;

	case 0x46: //OR A-H
		F_OR(REGISTER_A, REGISTER_H);
		break;

	case 0x56: //OR A-M
		F_OR(REGISTER_A, REGISTER_M);
		break;

	case 0x66: //OR B-L
		F_OR(REGISTER_B, REGISTER_L);
		break;

	case 0x76: //OR B-H
		F_OR(REGISTER_B, REGISTER_H);
		break;

	case 0x86: //OR B-M
		F_OR(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: CLC
		* Description: Clear Carry flag
		* Flags: - - - I - Z N C
		* Flags: - - - - - - - 0
		*/
	case 0x05: //CLC
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*
		* Opcode name: STC
		* Description: Set Carry flag
		* Flags: - - - I - Z N C
		* Flags: - - - - - - - 1
		*/
	case 0x06: //STC
		Flags = Flags | FLAG_C;
		break;

		/*
		* Opcode name: CLI
		* Description: Clear Interupt flag
		* Flags: - - - I - Z N C
		* Flags: - - - 0 - - - -
		*/
	case 0x07: //CLI
		Flags = Flags & (0xFF - FLAG_I);
		break;

		/*
		* Opcode name: STI
		* Description: Set Interupt flag
		* Flags: - - - I - Z N C
		* Flags: - - - 1 - - - -
		*/
	case 0x08: //STI
		Flags = Flags | FLAG_I;
		break;

		/*
		* Opcode name: BIT
		* Description: Register Bit tested with Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0x39: //BIT A-L
		F_BIT(REGISTER_A, REGISTER_L);
		break;

	case 0x49: //BIT A-H
		F_BIT(REGISTER_A, REGISTER_H);
		break;

	case 0x59: //BIT A-M
		F_BIT(REGISTER_A, REGISTER_M);
		break;

	case 0x69: //BIT B-L
		F_BIT(REGISTER_B, REGISTER_L);
		break;

	case 0x79: //BIT B-H
		F_BIT(REGISTER_B, REGISTER_H);
		break;

	case 0x89: //BIT B-M
		F_BIT(REGISTER_B, REGISTER_M);
		break;

		/*
		* Opcode name: PUSH
		* Description: Pushes Register onto the Stack
		*/
	case 0xBE: //PUSH A
		F_PUSH(REGISTER_A);
		break;

	case 0xCE: //PUSH B
		F_PUSH(REGISTER_B);
		break;

	case 0xDE: //PUSH FL
		F_PUSH_FL();
		break;

	case 0xEE: //PUSH L
		F_PUSH(REGISTER_L);
		break;

	case 0xFE: //PUSH H
		F_PUSH(REGISTER_H);
		break;

		/*
		* Opcode name: POP
		* Description: Pop the top of the stack into the Register
		*/
	case 0xBF: //POP A
		F_POP(REGISTER_A);
		break;

	case 0xCF: //POP B
		F_POP(REGISTER_B);
		break;

	case 0xDF: //POP FL
		F_POP_FL();
		break;

	case 0xEF: //POP L
		F_POP(REGISTER_L);
		break;

	case 0xFF: //POP H
		F_POP(REGISTER_H);
		break;

		/*
		* Opcode name: JMP
		* Description: Loads Memory into ProgramCounter
		*/
	case 0x10: // JMP (abs)
		ProgramCounter = get_address(abs_a);
		break;

		/*
		* Opcode name: JSR
		* Description: Jump to subroutine
		*/
	case 0x21: // JSR (abs)
		address = get_address(abs_a);
		//Checks if stack pointer is vaild
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
		}
		//Sets the program counter to the next address
		ProgramCounter = address;
		break;

		/*
		* Opcode name: RET
		* Description: Return from subroutine
		*/
	case 0x4C: // RET (impl)
		//Checks if stack pointer is vaild
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 2))
		{
			//Pops data off the stack (LB and HB)
			StackPointer++;
			LB = Memory[StackPointer];
			StackPointer++;
			HB = Memory[StackPointer];
		}
		//Puts LB and HB into the program counter
		ProgramCounter = ((WORD)HB << 8) + (WORD)LB;
		break;

		/*
		* Opcode name: JCC
		* Description: Jump on Carry clear
		*/
	case 0x11: // JCC (abs)
		 address = get_address(abs_a);
		 if ((Flags & FLAG_C) == 0)
		 {
				ProgramCounter = address;
		 }
		 break;

		/*
		* Opcode name: JCS
		* Description: Jump on Carry Set
		*/
	case 0x12: // JCS (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_C) != 0)
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: JNE
		* Description: Jump on result not zero
		*/
	case 0x13: // JNE (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_Z) == 0)
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: JEQ
		* Description: Jump on result equal to Zero
		*/
	case 0x14: // JEQ (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_Z) != 0)
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: JMI
		* Description: Jump on negative result
		*/
	case 0x15: // JMI (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_N) != 0)
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: JPL
		* Description: Jump on positive result
		*/
	case 0x16: // JPL (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_N) == 0)
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: JHI
		* Description: Jump on result same or lower
		*/
	case 0x17: // JHI (abs)
		address = get_address(abs_a);
		if ((((Flags & FLAG_C) | ((Flags & FLAG_Z))) != 0))
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: JLE
		* Description: Jump on result higher
		*/
	case 0x18: // JLE (abs)
		address = get_address(abs_a);
		if ((((Flags & FLAG_C)  | ((Flags & FLAG_Z))) == 0))
		{
			ProgramCounter = address;
		}
		break;

		/*
		* Opcode name: CCC
		* Description: Call on Carry clear
		*/
	case 0x22: // CCC (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_C) == 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CCS
		* Description: Call on Carry set
		*/
	case 0x23: // CCS (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_C) != 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CNE
		* Description: Call on result not zero
		*/
	case 0x24: // CNE (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_Z) == 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CEQ
		* Description: Call on result equal to zero
		*/
	case 0x25: // CEQ (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_Z) != 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CMI
		* Description: Call on negative result
		*/
	case 0x26: // CMI (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_N) != 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CPL
		* Description: Call on positive result
		*/
	case 0x27: // CPL (abs)
		address = get_address(abs_a);
		if ((Flags & FLAG_N) == 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CHI
		* Description: Call on result same or lower
		*/
	case 0x28: // CHI (abs)
		address = get_address(abs_a);
		if (((Flags & FLAG_C) | (Flags & FLAG_Z)) != 0)
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: CLE
		* Description: Call on result higher
		*/
	case 0x29:
		address = get_address(abs_a);
		if ((((Flags & FLAG_C) | ((Flags & FLAG_Z))) == 0))
		{
			F_CALL(address);
		}
		break;

		/*
		* Opcode name: INCA
		* Description: Increment Memory or Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xD0:
		F_INC_R(REGISTER_A);
		break;

		/*
		* Opcode name: INCB
		* Description: Increment Memory or Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xE0:
		F_INC_R(REGISTER_B);
		break;

		/*
		* Opcode name: DECA
		* Description: Decrement Memory or Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xD1:
		F_DEC_R(REGISTER_A);
		break;

		/*
		* Opcode name: DECB
		* Description: Decrement Memory or Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xE1:
		F_DEC_R(REGISTER_B);
		break;

		/*
		* Opcode name: INCY
		* Description: Increment register Y
		* Flags: - - - I - Z N C
		* Flags: - - - - - T - -
		*/
	case 0x04:
		F_INC_I(REGISTER_Y);
		break;

		/*
		* Opcode name: DEY
		* Description: Decrement register Y
		* Flags: - - - I - Z N C
		* Flags: - - - - - T - -
		*/
	case 0x03:
		F_DEC_I(REGISTER_Y);
		break;

		/*
		* Opcode name: INCX
		* Description: Increment register X
		* Flags: - - - I - Z N C
		* Flags: - - - - - T - -
		*/
	case 0x02:
		F_INC_I(REGISTER_X);
		break;

		/*
		* Opcode name: DECX
		* Description: Decrement register X
		* Flags: - - - I - Z N C
		* Flags: - - - - - T - -
		*/
	case 0x01:
		F_DEC_I(REGISTER_X);
		break;

		/*
		* Opcode name: INC
		* Description: Increment Memory or Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xA0: //INC (abs)
		address = get_address(abs_a);
		F_INC_M(address);
		break;

	case 0xB0: //INC (abs, X)
		address = get_address(absX_a);
		F_INC_M(address);
		break;

	case 0xC0: //INC (abs, Y)
		address = get_address(absY_a);
		F_INC_M(address);
		break;

		/*
		* Opcode name: DEC
		* Description: Decrement Memory or Accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xA1: //DEC (abs)
		address = get_address(abs_a);
		F_DEC_M(address);
		break;

	case 0xB1: //DEC (abs, X)
		address = get_address(absX_a);
		F_DEC_M(address);
		break;

	case 0xC1: //DEC (abs, Y)
		address = get_address(absY_a);
		F_DEC_M(address);
		break;

		/*
		* Opcode name: SBIA
		* Description: Data subtracted to accumulator with carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x93:
		F_SBI(REGISTER_A);
		break;

		/*
		* Opcode name: SBIB
		* Description: Data subtracted to accumulator with carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x94:
		F_SBI(REGISTER_B);
		break;

		/*
		* Opcode name: CPIA
		* Description: Data compared to accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x95:
		F_CPI(REGISTER_A);
		break;

		/*
		* Opcode name: CPIB
		* Description: Data compared to accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0x96:
		F_CPI(REGISTER_B);
		break;

		/*
		* Opcode name: ORIA
		* Description: Data bitwise inclusive or with accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0x97:
		F_ORI(REGISTER_A);
		break;

		/*
		* Opcode name: ORIB
		* Description: Data bitwise inclusive or with accumulator
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0x98:
		F_ORI(REGISTER_B);
		break;

		/*
		* Opcode name: HLT
		* Description: Wait for interupt
		*/
	case 0x2D:
		halt = true;
		break;

		/*
		* Opcode name: COMA
		* Description: Negate Accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xD7:
		F_COM_R(REGISTER_A);
		break;

		/*
		* Opcode name: COMB
		* Description: Negate Accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xE7:
		F_COM_R(REGISTER_B);
		break;

		/*
		* Opcode name: COM
		* Description: Negate Memory
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xA7: //COM (abs)
		address = get_address(abs_a);
		F_COM_M(address);
		break;

	case 0xB7: //COM (abs, X)
		address = get_address(absX_a);
		F_COM_M(address);
		break;

	case 0xC7: //COM (abs, Y)
		address = get_address(absY_a);
		F_COM_M(address);
		break;

		/*
		* Opcode name: SAL
		* Description: Arithmetic shift left Memory
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xA4: //SAL (abs)
		address = get_address(abs_a);
		F_SL_M(address);
		break;

	case 0xB4: //SAL (abs, X)
		address = get_address(absX_a);
		F_SL_M(address);
		break;

	case 0xC4: //SAL (abs, Y)
		address = get_address(absY_a);
		F_SL_M(address);
		break;

		/*
		* Opcode name: SALA
		* Description: Arithmetic shift left accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xD4:
		F_SL_R(REGISTER_A);
		break;

		/*
		* Opcode name: SALB
		* Description: Arithmetic shift left accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xE4:
		F_SL_R(REGISTER_B);
		break;

		/*
		* Opcode name: SAR
		* Description: Arithmetic shift right Memory
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xA5: //SAR (abs)
		address = get_address(abs_a);
		F_SR_M(address);
		break;

	case 0xB5: //SAR (abs, X)
		address = get_address(absX_a);
		F_SR_M(address);
		break;

	case 0xC5: //SAR (abs, Y)
		address = get_address(absY_a);
		F_SR_M(address);
		break;

		/*
		* Opcode name: SARA
		* Description: Arithmetic shift right accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xD5:
		F_SR_R(REGISTER_A);
		break;

		/*
		* Opcode name: SARB
		* Description: Arithmetic shift right accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xE5:
		F_SR_R(REGISTER_B);
		break;

		/*
		* Opcode name: ROL
		* Description: Rotate left memory without carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xA8: //ROL (abs)
		address = get_address(abs_a);
		F_ROL_M(address);
		break;

	case 0xB8: //ROL (abs, X)
		address = get_address(absX_a);
		F_ROL_M(address);
		break;

	case 0xC8: //ROL (abs, Y)
		address = get_address(absY_a);
		F_ROL_M(address);
		break;

		/*
		* Opcode name: ROLA
		* Description: Rotate left accumulator A without carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xD8:
		F_ROL_R(REGISTER_A);
		break;

		/*
		* Opcode name: ROLB
		* Description: Rotate left accumulator B without carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xE8:
		F_ROL_R(REGISTER_B);
		break;

		/*
		* Opcode name: RR
		* Description: Rotate right memory without carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xA9: //RR (abs)
		address = get_address(abs_a);
		F_RR_M(address);
		break;

	case 0xB9: //RR (abs, X)
		address = get_address(absX_a);
		F_RR_M(address);
		break;

	case 0xC9: //RR (abs, Y)
		address = get_address(absY_a);
		F_RR_M(address);
		break;

		/*
		* Opcode name: RRA
		* Description: Rotate right accumulator A without carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xD9:
		F_RR_R(REGISTER_A);
		break;

		/*
		* Opcode name: RRB
		* Description: Rotate right accumulator B without carry
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T -
		*/
	case 0xE9:
		F_RR_R(REGISTER_B);
		break;

		/*
		* Opcode name: RLCA
		* Description: Rotate left through carry, accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xD3:
		F_RLC_R(REGISTER_A);
		break;

		/*
		* Opcode name: RLCB
		* Description: Rotate left through carry, accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xE3:
		F_RLC_R(REGISTER_B);
		break;

		/*
		* Opcode name: RLC
		* Description: Rotate left through carry, memory
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xA3: //RLC (abs)
		address = get_address(abs_a);
		F_RLC_M(address);
		break;

	case 0xB3: //RLC (abs, X)
		address = get_address(absX_a);
		F_RLC_M(address);
		break;

	case 0xC3: //RLC (abs, Y)
		address = get_address(absX_a);
		F_RLC_M(address);
		break;

		/*
		* Opcode name: RRCA
		* Description: Rotate right through carry, accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xD2:
		F_RRC_R(REGISTER_A);
		break;

		/*
		* Opcode name: RRCB
		* Description: Rotate right through carry, accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xE2:
		F_RRC_R(REGISTER_B);
		break;

		/*
		* Opcode name: RRC
		* Description: Rotate right through carry, memory
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xA2: //RRC (abs)
		address = get_address(abs_a);
		F_RRC_M(address);
		break;

	case 0xB2: //RRC (abs, X)
		address = get_address(absX_a);
		F_RRC_M(address);
		break;

	case 0xC2: //RRC (abs, Y)
		address = get_address(absX_a);
		F_RRC_M(address);
		break;

		/*
		* Opcode name: LSR
		* Description: Shift right memory
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xA6: //LSR (abs)
		address = get_address(abs_a);
		F_LSR_M(address);
		break;

	case 0xB6: //LSR (abs, X)
		address = get_address(absX_a);
		F_LSR_M(address);
		break;

	case 0xC6: //LSR (abs)
		address = get_address(absY_a);
		F_LSR_M(address);
		break;

		/*
		* Opcode name: LSRA
		* Description: Shift right accumulator A
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xD6:
		F_LSR_R(REGISTER_A);
		break;

		/*
		* Opcode name: LSRB
		* Description: Shift right accumulator B
		* Flags: - - - I - Z N C
		* Flags: - - - - - T T T
		*/
	case 0xE6:
		F_LSR_R(REGISTER_B);
		break;

		/*
		* Opcode name: SWI
		* Description: Software interupt
		* Flags: - - - I - Z N C
		* Flags: - - - 1 - - - -
		*/
	case 0x5C:
		Flags = Flags | FLAG_I;
		F_PUSH(REGISTER_A);
		F_PUSH(REGISTER_B);
		F_PUSH_FL();
		F_PUSH(REGISTER_L);
		F_PUSH(REGISTER_H);
		break;

		/*
		* Opcode name: RTI
		* Description: Return from software interupt
		*/
	case 0x5D:
		F_POP(REGISTER_H);
		F_POP(REGISTER_L);
		F_POP_FL();
		F_POP(REGISTER_B);
		F_POP(REGISTER_A);
		break;



//TODO: F_regM
//TODO: Rest of opcodes
//TODO: comments
//TODO: compare tests
//TODO: Comment for F_CALL
//TODO: Function comments
//TODO: Function documentation description - cnz


	}
}

/*
* Function: Group_2_Move
* Description: Function to decode group 2 opcodes to carry out move instructions
* Parameters: opcode (BYTE)
* Returns: None (VOID)
* Warnings: None
*/
void Group_2_Move(BYTE opcode)
{

	BYTE *registers[] =
	{
		&Registers[REGISTER_A],
		&Registers[REGISTER_B],
		&Registers[REGISTER_L],
		&Registers[REGISTER_H],
		&Registers[REGISTER_M],
	};

	int source = (opcode >> 4) - 0x6;
	int dest = (opcode & 0x0F) - 0xB;

	*registers[dest] = *registers[source];

}

/*
* Function: execute
* Description: Function to carry out the execute part of the fetch-execute instruction
* Parameters: opcode (BYTE)
* Returns: None (VOID)
* Warnings: none
*/
void execute(BYTE opcode)
{

	if (((opcode >= 0x6B) && (opcode <= 0x6F))
		|| ((opcode >= 0x7B) && (opcode <= 0x7F))
		|| ((opcode >= 0x8B) && (opcode <= 0x8F))
		|| ((opcode >= 0x9B) && (opcode <= 0x9F))
		|| ((opcode >= 0xAB) && (opcode <= 0xAF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

/*
* Function: emulate
* Description: Function that emulates the processor
* Parameters: None (VOID)
* Returns: None (VOID)
* Warnings: none
*/
void emulate()
{
	BYTE opcode;
	int sanity;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;
	sanity = 0;

	printf("                    A  B  L  H  X  Y  SP\n");

	while ((!halt) && (memory_in_range) && (sanity < 200))
	{
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_L]);
		printf("%02X ", Registers[REGISTER_H]);
		printf("%02X ", Index_Registers[REGISTER_X]);
		printf("%02X ", Index_Registers[REGISTER_Y]);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
		sanity++;
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames()
{
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++)
	{
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename)
 {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		if (chr == '.')
		{
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename)
 {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename)
{
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL )
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename)
{
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL )
	{
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args,_TCHAR** argv)
{
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if(args == 2)
	{
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else
	{
		ln = 0;
		chr = '\0';
		while (chr != '\n')
		{
			chr = getchar();

			switch(chr)
			{
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)
				{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	} else
	{
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	} else
	{
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file))
	{
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++)
		{
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL )
		{
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer))
			{
				if (sscanf(InputBuffer, "L=%x", &address) == 1)
				{
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1)
				{
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
					{
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else
				{
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else
	{
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv)
{
	char buffer[1024];
	load_and_run(args,argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X",
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4],
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6],
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8],
		Memory[TEST_ADDRESS_9],
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark()
{
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1)
			{
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1)
			{
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0)
			{
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++)
				{
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"branch.txt", "a");

				while (!end_of_program)
				{
					chr = buffer[i];
					switch (chr)
					{
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1)
						{
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1)
						{
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
							{
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else
						{
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1)
				{
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4],
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6],
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8],
						Memory[TEST_ADDRESS_9],
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {
		// Creation failed!
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if(argc == 2){ building(argc,argv); exit(0);}
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc,argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}
