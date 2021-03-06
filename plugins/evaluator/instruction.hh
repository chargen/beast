// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef BSE_EVALUATOR_INSTRUCTION_H
#define BSE_EVALUATOR_INSTRUCTION_H

#include "symbols.hh"
#include <math.h>

namespace Bse {
namespace EvaluatorUtils {

class Instruction {
private:
    union {
	int reg;
    } p1;
    union {
	int reg;
	double val;
    } p2;

public:
    enum Type {
	SET,   /* dest, value */
	MOVE,  /* dest, src */
	ADD,   /* dest, src */
	MUL,   /* dest, src */
	SIN    /* dest */
    } ins;

    inline void exec(double *regs) const
    {
	switch(ins)
	{
	    case SET:	regs[p1.reg] = p2.val;
			break;

	    case MOVE:	regs[p1.reg] = regs[p2.reg];
			break;

	    case ADD:	regs[p1.reg] += regs[p2.reg];
			break;

	    case MUL:	regs[p1.reg] *= regs[p2.reg];
			break;

	    case SIN:	regs[p1.reg] = sin(regs[p1.reg]);
			break;
	}
    }

    void print(const Symbols& symbols) const;
    void rw_registers(int& read1, int& read2, int& write1, int& write2) const;

    /* "constuctors" */
    static Instruction rr(Type ins, int reg1, int reg2);  // register register instruction
    static Instruction rv(Type ins, int reg, double val); // register value instruction
};

}
}

#endif // BSE_EVALUATOR_INSTRUCTION_H
