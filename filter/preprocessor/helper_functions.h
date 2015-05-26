void helper_pprint_instr_list(bh_ir *bhir);

void helper_pprint_view(const bh_view *view);

void helper_pprint_base(const bh_base *base);

bool const_equals(const bh_instruction *instr, int compareTo);

bool const_is_int(const bh_instruction *instr);

int const_get_int(const bh_instruction *instr);

bool constants_identical(const bh_constant *a, const bh_constant *b);

bool identical_instr(const bh_instruction *a, const bh_instruction *b);

bool bh_view_same_base(const bh_view *a, const bh_view *b);

bool bh_view_equally_shifted(const bh_view *a, const bh_view *b);

bool operation_commutative(bh_opcode opcode);

bool operation_datamanagement(bh_opcode opcode);

bh_view* instruction_get_view_of_interest(const bh_instruction *instr);

bool has_equally_shaped_references(const bh_view *view, std::vector<bh_instruction>::iterator firstElmPtr, std::vector<int> *offsets);

bool has_constant(const bh_instruction *instr);

//bh_view* create_result_view(bh_type type, bh_index nelem);

bh_view build_flat_view(const bh_base * base, bh_index nelem);

bh_view build_similar_view(const bh_view *referenceView, const bh_base *base);

void build_similar_view2(const bh_view *referenceView, const bh_base *base, bh_view *output);
