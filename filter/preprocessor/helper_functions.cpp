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
    if (bh_operands(a->opcode) == 1)
        return false;
    if (bh_operands(a->opcode) == 2) {
        if (!bh_is_constant(&a->operand[1]) && !bh_is_constant(&b->operand[1])) {
            return bh_view_same_base(&a->operand[1], &b->operand[1]);
        }
        return false;
    }
    else {
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
