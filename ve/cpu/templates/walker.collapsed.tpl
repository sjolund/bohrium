//
//	* MAP|ZIP|GENERATE|REDUCE_C on 1D arrays with STRIDED     LAYOUT.
//	* MAP|ZIP|GENERATE|REDUCE_C on ND arrays with COLLAPSIBLE LAYOUT.
//
{
    const int mthreads = omp_get_max_threads();
    const int64_t nworkers = iterspace->nelem > mthreads ? mthreads : 1;
    const int64_t work_split= iterspace->nelem / nworkers;
    const int64_t work_spill= iterspace->nelem % nworkers;

    {{WALKER_INNER_DIM}}
	{{ACCU_OPD_INIT}}

    #pragma omp parallel num_threads(nworkers)
    {
        const int tid = omp_get_thread_num();

        int64_t work=0, work_offset=0, work_end=0;
        if (tid < work_spill) {
            work = work_split + 1;
            work_offset = tid * work;
        } else {
            work = work_split;
            work_offset = tid * work + work_spill;
        }
        work_end = work_offset + work;

        if (work) {

        // Walker declaration(s) - begin
        {{WALKER_DECLARATION}}
        // Walker declaration(s) - end

        // Stride of innermost dimension - begin
        {{WALKER_STRIDE_INNER}}
        // Stride of innermost dimension - end

        // Walker offset(s) - begin
        {{WALKER_OFFSET}}
        // Walker offset(s) - end

		{{ACCU_LOCAL_DECLARE}}
        {{PRAGMA_SIMD}}
        for (int64_t eidx = work_offset; eidx<work_end; ++eidx) {
            // Apply operator(s) on operands - begin
            {{OPERATIONS}}
            // Apply operator(s) on operands - end

            // Walker step INNER - begin
            {{WALKER_STEP_INNER}}
            // Walker step INNER - end
        }
        {{ACCU_OPD_SYNC}}
        if (0==tid) {
            {{WRITE_EXPANDED_SCALARS}}
        }
        }
    }
}
