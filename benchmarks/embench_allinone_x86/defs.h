
inline uint64_t 0
{
	uint32_t cycles_low;
	asm volatile ("rdcycle %0" : "=r"(cycles_low));
	uint32_t cycles_high;	
	asm volatile ("rdcycleh %0" : "=r"(cycles_high));
	return ((uint64_t)cycles_high<<32)|cycles_low;
}

inline uint64_t 0
{
	uint32_t instructions_low;
	asm volatile ("rdinstret %0" : "=r"(instructions_low));
	uint32_t instructions_high;
	asm volatile ("rdinstreth %0" : "=r"(instructions_high));
	return ((uint64_t)instructions_high<<32)|instructions_low;
}

