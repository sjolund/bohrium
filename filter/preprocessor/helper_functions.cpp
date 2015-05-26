#include <bh.h>
#include <bh_pprint.h>


void helper_pprint_instr_list(bh_ir *bhir)
{
	bh_pprint_instr_list(&bhir->instr_list[0], bhir->instr_list.size(), "\n\n\n\nNumber of instructions in batch");
}

void helper_pprint_view(const bh_view *view) {
    char source[1024]="";
    bh_sprint_view(view, source);
    std::cout << source << std::endl;
}

void helper_pprint_base(const bh_base *base) {
    char source[1024]="";
    bh_sprint_base(base, source);
    std::cout << source << std::endl;
}



bool const_equals(const bh_instruction *instr, int compareTo) {

    switch( instr->constant.type) {
        case BH_INT8:
            return instr->constant.value.int8 == compareTo;
            break;
        case BH_INT16:
            return instr->constant.value.int16 == compareTo;
            break;
        case BH_INT32:
            return instr->constant.value.int32 == compareTo;
            break;
        case BH_INT64:
            return instr->constant.value.int64 == compareTo;
            break;
        case BH_UINT8:
            return (int)instr->constant.value.uint8 == compareTo;
            break;
        case BH_UINT16:
            return (int)instr->constant.value.uint16 == compareTo;
            break;
        case BH_UINT32:
            return (int)instr->constant.value.uint32 == compareTo;
            break;
        case BH_UINT64:
            return (int)instr->constant.value.uint64 == compareTo;
            break;
        case BH_FLOAT32:
            return (int)instr->constant.value.float32 == compareTo;
            break;
        case BH_FLOAT64:
            return (int)instr->constant.value.float64 == compareTo;
            break;
        default:
            return false;
    }

}

bool const_is_int(const bh_instruction *instr) {
    if (instr->constant.type == BH_FLOAT32) {
        return (bh_float32)(int)instr->constant.value.float32 == instr->constant.value.float32;
    }
    if (instr->constant.type == BH_FLOAT64) {
        return (bh_float64)(int)instr->constant.value.float64 == instr->constant.value.float64;
    }
    return true;
}

int const_get_int(const bh_instruction *instr) {
    switch(instr->constant.type) {
        case BH_INT8:
            return (int)instr->constant.value.int8;
            break;
        case BH_INT16:
            return (int)instr->constant.value.int16;
            break;
        case BH_INT32:
            return (int)instr->constant.value.int32;
            break;
        case BH_INT64:
            return (int)instr->constant.value.int64;
            break;
        case BH_UINT8:
            return (int)instr->constant.value.uint8;
            break;
        case BH_UINT16:
            return (int)instr->constant.value.uint16;
            break;
        case BH_UINT32:
            return (int)instr->constant.value.uint32;
            break;
        case BH_UINT64:
            return (int)instr->constant.value.uint64;
            break;
        case BH_FLOAT32:
            return (int)instr->constant.value.float32;
            break;
        case BH_FLOAT64:
            return (int)instr->constant.value.float64;
            break;
        default:
            return 0;
    }
}

bool constants_identical(const bh_constant *a, const bh_constant *b) {
    switch(a->type) {
        case BH_INT8:
            return b->type == BH_INT8 && a->value.int8 == b->value.int8;
            break;
        case BH_INT16:
            return b->type == BH_INT16 && a->value.int16 == b->value.int16;
            break;
        case BH_INT32:
            return b->type == BH_INT32 && a->value.int32 == b->value.int32;
            break;
        case BH_INT64:
            return b->type == BH_INT64 && a->value.int64 == b->value.int64;
            break;
        case BH_UINT8:
            return b->type == BH_UINT8 && (int)a->value.uint8 == (int)b->value.uint8;
            break;
        case BH_UINT16:
            return b->type == BH_UINT16 && (int)a->value.uint16 == (int)b->value.uint16;
            break;
        case BH_UINT32:
            return b->type == BH_UINT32 && (int)a->value.uint32 == (int)b->value.uint32;
            break;
        case BH_UINT64:
            return b->type == BH_UINT64 && (int)a->value.uint64 == (int)b->value.uint64;
            break;
        case BH_FLOAT32:
            return b->type == BH_FLOAT32 && (int)a->value.float32 == (int)b->value.float32;
            break;
        case BH_FLOAT64:
            return b->type == BH_FLOAT64 && (int)a->value.float64 == (int)b->value.float64;
            break;
        default:
            return false;
    }
}



bool bh_view_same_base(const bh_view *a, const bh_view *b)
{
    if(bh_is_constant(a) || bh_is_constant(b))
        return false;
    if(a->base != b->base)
        return false;
    return true;
}

bool bh_view_equally_shifted(const bh_view *a, const bh_view *b)
{
    int i;
    if(bh_is_constant(a) || bh_is_constant(b))
        return false;
    if(a->ndim != b->ndim)
        return false;
    if(a->start != b->start)
        return false;
    for(i=0; i<a->ndim; ++i)
    {
        if(a->shape[i] != b->shape[i])
            return false;
        if(a->stride[i] != b->stride[i])
            return false;
    }
    return true;
}

bool operation_commutative(bh_opcode opcode) {

    if (opcode == (bh_opcode)BH_SUBTRACT
        || opcode == (bh_opcode)BH_DIVIDE
        || opcode == (bh_opcode)BH_POWER
        || opcode == (bh_opcode)BH_GREATER
        || opcode == (bh_opcode)BH_GREATER_EQUAL
        || opcode == (bh_opcode)BH_LESS
        || opcode == (bh_opcode)BH_LESS_EQUAL
        || opcode == (bh_opcode)BH_LEFT_SHIFT
        || opcode == (bh_opcode)BH_RIGHT_SHIFT)
        return false;
    /*     // Add arguments element-wise.
        BH_SUBTRACT = 2,        // Subtract arguments, element-wise.
        BH_MULTIPLY = 3,        // Multiply arguments element-wise.
        BH_DIVIDE = 4,      // Divide arguments element-wise.
        BH_POWER = 5,       // First array elements raised to powers from second array, element-wise.
        BH_ABSOLUTE = 6,        // Calculate the absolute value element-wise.
        BH_GREATER = 7,     // Return the truth value of (x1 > x2) element-wise.
        BH_GREATER_EQUAL = 8,       // Return the truth value of (x1 >= x2) element-wise.
        BH_LESS = 9,        // Return the truth value of (x1 < x2) element-wise.
        BH_LESS_EQUAL = 10,     // Return the truth value of (x1 =< x2) element-wise.
        BH_EQUAL = 11,      // Return (x1 == x2) element-wise.
        BH_NOT_EQUAL = 12,      // Return (x1 != x2) element-wise.
        BH_LOGICAL_AND = 13,        // Compute the truth value of x1 AND x2 elementwise.
        BH_LOGICAL_OR = 14,     // Compute the truth value of x1 OR x2 elementwise.
        BH_LOGICAL_XOR = 15,        // Compute the truth value of x1 XOR x2, element-wise.
        BH_LOGICAL_NOT = 16,        // Compute the truth value of NOT x elementwise.
        BH_MAXIMUM = 17,        // Element-wise maximum of array elements.
        BH_MINIMUM = 18,        // Element-wise minimum of array elements.
        BH_BITWISE_AND = 19,        // Compute the bit-wise AND of two arrays element-wise.
        BH_BITWISE_OR = 20,     // Compute the bit-wise OR of two arrays element-wise.
        BH_BITWISE_XOR = 21,        // Compute the bit-wise XOR of two arrays element-wise.
        BH_INVERT = 22,     // Compute bit-wise inversion, or bit-wise NOT, element-wise.
        BH_LEFT_SHIFT = 23,     // Shift the bits of an integer to the left.
        BH_RIGHT_SHIFT = 24,        // Shift the bits of an integer to the right.
        BH_COS = 25,        // Cosine elementwise.
        BH_SIN = 26,        // Trigonometric sine, element-wise.
        BH_TAN = 27,        // Compute tangent element-wise.
        BH_COSH = 28,       // Hyperbolic cosine, element-wise.
        BH_SINH = 29,       // Hyperbolic sine, element-wise.
        BH_TANH = 30,       // Compute hyperbolic tangent element-wise.
        BH_ARCSIN = 31,     // Inverse sine, element-wise.
        BH_ARCCOS = 32,     // Trigonometric inverse cosine, element-wise.
        BH_ARCTAN = 33,     // Trigonometric inverse tangent, element-wise.
        BH_ARCSINH = 34,        // Inverse hyperbolic sine elementwise.
        BH_ARCCOSH = 35,        // Inverse hyperbolic cosine, elementwise.
        BH_ARCTANH = 36,        // Inverse hyperbolic tangent elementwise.
        BH_ARCTAN2 = 37,        // Element-wise arc tangent of ``x1/x2`` choosing the quadrant correctly.
        BH_EXP = 38,        // Calculate the exponential of all elements in the input array.
        BH_EXP2 = 39,       // Calculate `2**p` for all `p` in the input array.
        BH_EXPM1 = 40,      // Calculate ``exp(x) - 1`` for all elements in the array.
        BH_LOG = 41,        // Natural logarithm, element-wise.
        BH_LOG2 = 42,       // Base-2 logarithm of `x`.
        BH_LOG10 = 43,      // Return the base 10 logarithm of the input array, element-wise.
        BH_LOG1P = 44,      // Return the natural logarithm of one plus the input array, element-wise.
        BH_SQRT = 45,       // Return the positive square-root of an array, element-wise.
        BH_CEIL = 46,       // Return the ceiling of the input, element-wise.
        BH_TRUNC = 47,      // Return the truncated value of the input, element-wise.
        BH_FLOOR = 48,      // Return the floor of the input, element-wise.
        BH_RINT = 49,       // Round elements of the array to the nearest integer.
        BH_MOD = 50,        // Return the element-wise remainder of division.
        BH_ISNAN = 51,      // Test for NaN values.
        BH_ISINF = 52,      // Test for infinity values.
        BH_IDENTITY = 53,       // The identity function that returns the input value converted to the output data type.
        BH_DISCARD = 54,        // System instruction that informs the child component to forget the array and release any metadata allocated.
        BH_FREE = 55,       // System instruction that informs the child component to deallocate the data storage associated with the array.
        BH_SYNC = 56,       // System instruction that informs the child component to make data synchronized and available.
        BH_NONE = 57,       // A opcode that should be ignored.
        BH_ADD_REDUCE = 59,     // Sums all elements in the specified dimension.
        BH_MULTIPLY_REDUCE = 60,        // Multiplies all elements in the specified dimension.
        BH_MINIMUM_REDUCE = 61,     // Finds the smallest elements in the specified dimension.
        BH_MAXIMUM_REDUCE = 62,     // Finds the largest elements in the specified dimension.
        BH_LOGICAL_AND_REDUCE = 63,     // Logical AND of all elements in the specified dimension.
        BH_BITWISE_AND_REDUCE = 64,     // Bitwise AND of all elements in the specified dimension.
        BH_LOGICAL_OR_REDUCE = 65,      // Logical OR of all elements in the specified dimension.
        BH_BITWISE_OR_REDUCE = 66,      // Bitwise OR of all elements in the specified dimension.
        BH_LOGICAL_XOR_REDUCE = 67,     // Logical XOR of all elements in the specified dimension.
        BH_BITWISE_XOR_REDUCE = 68,     // Bitwise XOR of all elements in the specified dimension.
        BH_RANDOM = 69,     // Random123: The returned result is a deterministic function of the key and counter, i.e. a unique (seed, indexes) tuple will always produce the same result. The result is highly sensitive to small changes in the inputs, so that the sequence of values produced by simply incrementing the counter (or key) is effectively indistinguishable from a sequence of samples of a uniformly distributed random variable.
        BH_RANGE = 70,      // Returns a 1-dim base-array filled with integer range starting a zero
        BH_REAL = 71,       // Return the real part of the elements of the array.
        BH_IMAG = 72,       // Return the imaginary part of the elements of the array.
        BH_ADD_ACCUMULATE = 73,     // Computes the prefix sum.
        BH_MULTIPLY_ACCUMULATE = 74,        // Computes the prefix product.
    */

    return true;

}

bool operation_datamanagement(bh_opcode opcode) {
    if (opcode == (bh_opcode)BH_DISCARD
        || opcode == (bh_opcode)BH_FREE
        || opcode == (bh_opcode)BH_SYNC)
        return true;
    return false;
}

bool identical_instr(const bh_instruction *a, const bh_instruction *b) {

    if (a->opcode != b->opcode)
        return false;
    if (bh_operands(a->opcode) < 3)
        // must have 3 operands?????????????????????????????????
        return false;
    if (bh_is_constant(&a->operand[1]))
    {
        if (bh_is_constant(&b->operand[1])) {
            if (constants_identical(&a->constant, &b->constant)
                && bh_view_same_base(&a->operand[2], &b->operand[2]))
                return true;
            else
                return false;
        }
        if (operation_commutative(a->opcode) && bh_is_constant(&b->operand[2])) {
            if (constants_identical(&a->constant, &b->constant)
                && bh_view_same_base(&a->operand[2], &b->operand[1]))
                return true;
            else
                return false;
        }
    }
    else if (bh_is_constant(&a->operand[2]))
    {
        if (operation_commutative(a->opcode) && bh_is_constant(&b->operand[1])) {
            if (constants_identical(&a->constant, &b->constant)
                && bh_view_same_base(&a->operand[1], &b->operand[2]))
                return true;
            else
                return false;
        }
        if (bh_is_constant(&b->operand[2])) {
            if (constants_identical(&a->constant, &b->constant)
                && bh_view_same_base(&a->operand[1], &b->operand[1]))
                return true;
            else
                return false;
        }
    }
    else {
        if (bh_view_same_base(&a->operand[1], &b->operand[1]) 
            && bh_view_same_base(&a->operand[2], &b->operand[2])
            && bh_view_equally_shifted(&a->operand[1], &a->operand[2])
            && bh_view_equally_shifted(&b->operand[1], &b->operand[2]))
            return true;
        if (operation_commutative(a->opcode)
            && bh_view_same_base(&a->operand[1], &b->operand[2]) 
            && bh_view_same_base(&a->operand[2], &b->operand[1])
            && bh_view_equally_shifted(&a->operand[1], &a->operand[2])
            && bh_view_equally_shifted(&b->operand[1], &b->operand[2]))
            return true;
    }

    return false;

}

bh_view* instruction_get_view_of_interest(const bh_instruction *instr) {

    if (bh_operands(instr->opcode) < 2)
        // must have at least 2 operands
        return NULL;

    if (bh_operands(instr->opcode) == 3 ) {
        if (bh_is_constant(&instr->operand[1]))
        {
            return (bh_view*)&instr->operand[2];
        }
        else if (bh_is_constant(&instr->operand[2])) {
            return (bh_view*)&instr->operand[1];
        }

        // Same dimension and shape?
        int i;
        if(instr->operand[1].ndim != instr->operand[2].ndim)
            return NULL;
        for(i=0; i<instr->operand[2].ndim; ++i)
        {
            if(instr->operand[1].shape[i] != instr->operand[2].shape[i])
                return NULL;
        }
        
        if (instr->operand[2].base->nelem > instr->operand[1].base->nelem) {
            return (bh_view*)&instr->operand[2];
        }
    }
    return (bh_view*)&instr->operand[1];
}


bool has_equally_shaped_references(const bh_view *view, std::vector<bh_instruction>::iterator firstElmPtr, std::vector<int> *offsets) {


    if (view->base == firstElmPtr->operand[0].base) {
        // Already found expansions fulfill the invariant that they are valid equally shifted, hence no need for matching views.
        return true;
    }

    for (std::vector<int>::iterator iter = offsets->begin(); iter != offsets->end(); iter++) {
        if (view->base == (firstElmPtr+(*iter))->operand[0].base) {
            return true;
        }
    }
    return false;

}

bool has_constant(const bh_instruction *instr) {
    if (bh_operands(instr->opcode) == 1)
        return false;

    return (bh_is_constant(&instr->operand[1]) 
            && (bh_operands(instr->opcode) < 3 
                || bh_is_constant(&instr->operand[2])));
}

/*bh_view* create_result_view(bh_type type, bh_index nelem) {
    bh_base* resultBase;
    bh_create_base(type, nelem, &resultBase);

    bh_view resultView = { resultBase, 1, 0, {nelem}, {1}};

    return &resultView;
}*/

bh_view build_flat_view(const bh_base *base, bh_index nelem) {
    bh_view resultView = { (bh_base*)base, 1, 0, {nelem}, {1}};
    return resultView;
}

bh_view build_similar_view(const bh_view *referenceView, const bh_base *base) {
    bh_view *view = new bh_view();
    memcpy(view, (bh_view*)referenceView, sizeof((bh_view)*referenceView));
    view->base = (bh_base*)base;
    return *view;
}

void build_similar_view2(const bh_view *referenceView, const bh_base *base, bh_view *output) {
    memcpy(output, (bh_view*)referenceView, sizeof((bh_view)*referenceView));
    output->base = (bh_base*)base;
}

void insertView(bh_ir& bhir, std::vector<bh_instruction>::iterator iter, bh_opcode opcode, const bh_view *out) {
    bh_instruction instr;
    instr.opcode = opcode;
    instr.operand[0] = *out;
    bhir.instr_list.insert(iter, instr);
}

void insertView(bh_ir& bhir, std::vector<bh_instruction>::iterator iter, bh_opcode opcode, const bh_view *out, const bh_view *in1) {
    bh_instruction instr;
    instr.opcode = opcode;
    instr.operand[0] = *out;
    instr.operand[1] = *in1;
    bhir.instr_list.insert(iter, instr);
}

void insertView(bh_ir& bhir, std::vector<bh_instruction>::iterator iter, bh_opcode opcode, const bh_view *out, const bh_view *in1, const bh_view *in2) {
    bh_instruction instr;
    instr.opcode = opcode;
    instr.operand[0] = *out;
    instr.operand[1] = *in1;
    instr.operand[2] = *in2;
    bhir.instr_list.insert(iter, instr);
}
