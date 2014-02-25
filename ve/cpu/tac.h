#ifndef __BH_VE_CPU_TAC
#define __BH_VE_CPU_TAC

#include "stdint.h"

typedef enum OPERATION {
    MAP         = 1,    // aka elementwise operation unary operator
    ZIP         = 2,    // aka elementwise operation with binary operator
    REDUCE      = 4,
    SCAN        = 8,
    GENERATE    = 16,   // examples: iota, random numbers etc.
    SYSTEM      = 32,
    EXTENSION   = 64,
} OPERATION;

typedef enum OPERATOR {
    // Used by elementwise operations
    ABSOLUTE,
    ARCCOS,
    ARCCOSH,
    ARCSIN,
    ARCSINH,
    ARCTAN,
    ARCTANH,
    CEIL,
    COS,
    COSH,
    EXP,
    EXP2,
    EXPM1,
    FLOOR,
    IDENTITY,
    IMAG,
    INVERT,
    ISINF,
    ISNAN,
    LOG,
    LOG10,
    LOG1P,
    LOG2,
    LOGICAL_NOT,
    REAL,
    RINT,
    SIN,
    SINH,
    SQRT,
    TAN,
    TANH,
    TRUNC,

    // Used by elementwise, reduce, and scan operations
    ADD,
    ARCTAN2,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    DIVIDE,
    EQUAL,
    GREATER,
    GREATER_EQUAL,
    LEFT_SHIFT,
    LESS,
    LESS_EQUAL,
    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_XOR,
    MAXIMUM,
    MINIMUM,
    MOD,
    MULTIPLY,
    NOT_EQUAL,
    POWER,
    RIGHT_SHIFT,
    SUBTRACT,

    // Used by system operations
    DISCARD,
    FREE,
    NONE,
    SYNC,

    // Used by generator operations
    FLOOD,
    RANDOM,
    RANGE,

    NBUILTIN,   // Not an operator but a count of built-in operators
    EXT_OFFSET  // Wildcard for extension opcodes

} OPERATOR;

typedef enum ETYPE {
    BOOL        = 1,   
        
    INT8        = 2,   
    INT16       = 4,   
    INT32       = 8,   
    INT64       = 16,  
        
    UINT8       = 32,  
    UINT16      = 64,  
    UINT32      = 128, 
    UINT64      = 256, 

    FLOAT32     = 512, 
    FLOAT64     = 1024,

    COMPLEX64   = 2048,
    COMPLEX128  = 4096,
} ETYPE;

typedef enum LAYOUT {
    CONSTANT    = 1,
    CONTIGUOUS  = 2,
    STRIDED     = 4,
    SPARSE      = 8
} LAYOUT;   // Uses a single byte

typedef struct tac {
    OPERATION op;       // Operation
    OPERATOR  oper;     // Operator
    uint32_t  out;      // Output operand
    uint32_t  in1;      // First input operand
    uint32_t  in2;      // Second input operand
} tac_t;

typedef struct operand {
    LAYOUT  layout;     // The layout of the data
    void*   data;       // Pointer to memory allocated for the array
    ETYPE   type;       // Type of the elements stored
    int64_t start;      // Offset from memory allocation to start of array
    int64_t nelem;      // Number of elements available in the allocation

    int64_t ndim;       // Number of dimensions of the array
    int64_t* shape;     // Shape of the array
    int64_t* stride;    // Stride in each dimension of the array
} operand_t;            // Meta-data for a block argument

#define BUILTIN_ARRAY_OPS (MAP | ZIP | REDUCE | SCAN | GENERATE)

#endif